//
// shoes/http/curl.c
// the downloader routines using libcurl.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/http.h"
#include "shoes/version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

typedef struct {
  char *mem;
  FILE *fp;
  size_t size;
  size_t total;
  shoes_download_handler handler;
  time_t last;
  void *data;
} shoes_curl_data;

const char *content_len_str = "Content-Length: ";

size_t
shoes_curl_header_funk(void *ptr, size_t size, size_t nmemb, shoes_curl_data *data)
{
  size_t realsize = size * nmemb;
  if (strncmp(ptr, content_len_str, strlen(content_len_str)) == 0)
  {
    data->total = strtoull(ptr + strlen(content_len_str), NULL, 10);
    HTTP_EVENT(data->handler, SHOES_HTTP_CONNECTED, data->last, 0, 0, data->total, data->data, return -1);
  }
  return realsize;
}

size_t
shoes_curl_write_funk(void *ptr, size_t size, size_t nmemb, shoes_curl_data *data)
{
  size_t realsize = size * nmemb;
  if (data->mem != NULL)
    memcpy(&(data->mem[data->size]), ptr, realsize);
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
             dltotal, data->data, return 1);
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
  cdata.fp = NULL;
  cdata.size = req->size = 0;
  cdata.total = 0;
  cdata.handler = req->handler;
  cdata.data = req->data;
  cdata.last = 0;

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
  req->size = cdata.total;

  HTTP_EVENT(cdata.handler, SHOES_HTTP_COMPLETED, cdata.last, 100, cdata.total, cdata.total, cdata.data, 1);

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
