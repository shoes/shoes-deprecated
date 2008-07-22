//
// shoes/http/curl.c
// the downloader routines using libcurl.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/http.h"
#include "shoes/version.h"
#include "shoes/internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct {
  char *mem;
  unsigned long memlen;
  FILE *fp;
  size_t size;
  size_t total;
  unsigned long status;
  shoes_download_handler handler;
  SHOES_TIME last;
  CURL *curl;
  void *data;
} shoes_curl_data;

const char *content_len_str = "Content-Length: ";

size_t
shoes_curl_header_funk(char *ptr, size_t size, size_t nmemb, shoes_curl_data *data)
{
  char *colon, *val, *end;
  size_t realsize = size * nmemb;
  if (data->status == 0)
  {
    shoes_download_event event;
    event.stage = SHOES_HTTP_STATUS;
    curl_easy_getinfo(data->curl, CURLINFO_RESPONSE_CODE, &event.status);
    data->status = event.status;
    if (data->handler != NULL) data->handler(&event, data->data);
  }

  for (colon = ptr; colon < ptr + realsize; colon++)
    if (colon[0] == ':')
      break;
  for (val = colon + 1; val < ptr + realsize; val++)
    if (val[0] != ' ')
      break;
  for (end = (ptr + realsize) - 1; end > ptr; end--)
    if (end[0] != '\r' && end[0] != '\n' && end[0] != ' ')
      break;

  if (colon < ptr + realsize)
  {
    shoes_download_event event;
    event.stage = SHOES_HTTP_HEADER;
    event.hkey = ptr;
    event.hkeylen = colon - ptr;
    event.hval = val;
    event.hvallen = (end - val) + 1;
    if (data->handler != NULL) data->handler(&event, data->data);

    if (strncmp(ptr, content_len_str, strlen(content_len_str)) == 0)
    {
      data->total = strtoull(ptr + strlen(content_len_str), NULL, 10);
      if (data->mem != NULL && data->total > data->memlen)
      {
        data->memlen = data->total;
        SHOE_REALLOC_N(data->mem, char, data->memlen);
      }
    }
  }
  return realsize;
}

size_t
shoes_curl_write_funk(void *ptr, size_t size, size_t nmemb, shoes_curl_data *data)
{
  size_t realsize = size * nmemb;
  if (data->size == 0)
  {
    HTTP_EVENT(data->handler, SHOES_HTTP_CONNECTED, data->last, 0, 0, data->total, data->data, NULL, return -1);
  }
  if (data->mem != NULL)
  {
    if (data->size + realsize > data->memlen)
    {
      while (data->size + realsize > data->memlen)
        data->memlen += SHOES_BUFSIZE;
      SHOE_REALLOC_N(data->mem, char, data->memlen);
    }
    SHOE_MEMCPY(&(data->mem[data->size]), ptr, char, realsize);
  }
  if (data->fp != NULL)
    fwrite(ptr, nmemb, size, data->fp);
  data->size += realsize;
  return realsize;
}

int
shoes_curl_progress_funk(shoes_curl_data *data,
  double dltotal, double dlnow, double ultotal, double ulnow)
{
  HTTP_EVENT(data->handler, SHOES_HTTP_TRANSFER, data->last, dlnow * 100.0 / dltotal, dlnow,
             dltotal, data->data, NULL, return 1);
  return 0;
}

void
shoes_download(shoes_download_request *req)
{
  char url[SHOES_BUFSIZE], uagent[SHOES_BUFSIZE], slash[2] = "/";
  CURL *curl = curl_easy_init();
  CURLcode res;
  shoes_curl_data cdata;
  if (curl == NULL) return;

  if (req->path[0] == '/') slash[0] = '\0';
  sprintf(url, "http://%s:%d%s%s", req->host, req->port, slash, req->path);
  sprintf(uagent, "Shoes/0.r%d (%s) %s/%d", SHOES_REVISION, SHOES_PLATFORM,
    SHOES_RELEASE_NAME, SHOES_BUILD_DATE);

  cdata.mem = req->mem;
  cdata.memlen = req->memlen;
  cdata.fp = NULL;
  cdata.size = req->size = 0;
  cdata.total = 0;
  cdata.handler = req->handler;
  cdata.data = req->data;
  cdata.last.tv_sec = 0;
  cdata.last.tv_nsec = 0;
  cdata.status = 0;
  cdata.curl = curl;

  if (req->mem == NULL)
  {
    cdata.fp = fopen(req->filepath, "wb");
    if (cdata.fp == NULL) return;
  } 

  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, TRUE);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, shoes_curl_header_funk);
  curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &cdata);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, shoes_curl_write_funk);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cdata);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, shoes_curl_progress_funk);
  curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &cdata);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, uagent);

  res = curl_easy_perform(curl);
  if (res != CURLE_OK)
  {
    HTTP_EVENT(cdata.handler, SHOES_HTTP_ERROR, cdata.last, res, 0, 0, cdata.data, NULL, 1);
    goto done;
  }

  req->size = cdata.total;
  req->mem = cdata.mem;

  HTTP_EVENT(cdata.handler, SHOES_HTTP_COMPLETED, cdata.last, 100, req->size, req->size, cdata.data, req->mem, 1);

done:
  if (cdata.fp != NULL)
    fclose(cdata.fp);

  curl_easy_cleanup(curl);
}

void *
shoes_download2(void *data)
{
  shoes_download_request *req = (shoes_download_request *)data;
  shoes_download(req);
  if (req->mem != NULL) free(req->mem);
  if (req->filepath != NULL) free(req->filepath);
  free(req->data);
  free(req);
  return NULL;
}

void
shoes_queue_download(shoes_download_request *req)
{
  pthread_t tid;
  pthread_create(&tid, NULL, shoes_download2, req);
}

VALUE
shoes_download_error(SHOES_DOWNLOAD_ERROR code)
{
  return rb_str_new2(curl_easy_strerror(code));
}
