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
  char *body;
  size_t size, total, readpos;
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
  size_t realsize = size * nmemb;
  if (data->status == 0)
  {
    shoes_download_event event;
    event.stage = SHOES_HTTP_STATUS;
    curl_easy_getinfo(data->curl, CURLINFO_RESPONSE_CODE, &event.status);
    data->status = event.status;
    if (data->handler != NULL) data->handler(&event, data->data);
  }

  HTTP_HEADER(ptr, realsize, data->handler, data->data);
  if (strncmp(ptr, content_len_str, strlen(content_len_str)) == 0)
  {
    data->total = strtoull(ptr + strlen(content_len_str), NULL, 10);
    if (data->mem != NULL && data->total > data->memlen)
    {
      data->memlen = data->total;
      SHOE_REALLOC_N(data->mem, char, data->memlen);
      if (data->mem == NULL) return -1;
    }
  }
  return realsize;
}

size_t
shoes_curl_read_funk(void *ptr, size_t size, size_t nmemb, shoes_curl_data *data)
{
  size_t realsize = size * nmemb;
  SHOE_MEMCPY(ptr, &(data->body[data->readpos]), char, realsize);
  data->readpos += realsize;
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
      if (data->mem == NULL) return -1;
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
  cdata.size = cdata.readpos = req->size = 0;
  cdata.total = 0;
  cdata.handler = req->handler;
  cdata.data = req->data;
  cdata.last.tv_sec = 0;
  cdata.last.tv_nsec = 0;
  cdata.status = 0;
  cdata.curl = curl;
  cdata.body = NULL;

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
  if (req->method)
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req->method);
  if (req->headers)
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, req->headers);
  if (req->body)
  {
    cdata.body = req->body;
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, shoes_curl_read_funk);
    curl_easy_setopt(curl, CURLOPT_READDATA, &cdata);
  }

  res = curl_easy_perform(curl);
  req->size = cdata.total;
  req->mem = cdata.mem;

  if (res != CURLE_OK)
  {
    shoes_download_event event;
    event.stage = SHOES_HTTP_ERROR;
    event.error = res;
    if (req->handler != NULL) req->handler(&event, req->data);
    goto done;
  }

  if (cdata.fp != NULL)
  { 
    fclose(cdata.fp);
    cdata.fp = NULL;
  }

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
  if (req->method != NULL) free(req->method);
  if (req->body != NULL) free(req->body);
  if (req->headers != NULL) curl_slist_free_all(req->headers);
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
shoes_http_error(SHOES_DOWNLOAD_ERROR code)
{
  return rb_str_new2(curl_easy_strerror(code));
}

SHOES_DOWNLOAD_HEADERS
shoes_http_headers(VALUE hsh)
{
  long i;
  struct curl_slist *slist = NULL;
  VALUE keys = rb_funcall(hsh, s_keys, 0);
  for (i = 0; i < RARRAY(keys)->len; i++ )
  {
    VALUE key = rb_ary_entry(keys, i);
    VALUE header = rb_str_dup(key);
    rb_str_cat2(header, ": ");
    rb_str_append(header, rb_hash_aref(hsh, key));
    slist = curl_slist_append(slist, RSTRING_PTR(header));
  }
  return slist;
}
