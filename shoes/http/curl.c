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

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

typedef struct {
  shoes_download_handler handler;
  void *data;
} shoes_curl_progress_data;

typedef struct {
  char *mem;
  FILE *fp;
  size_t size;
} shoes_curl_write_data;

size_t
shoes_curl_write_funk(void *ptr, size_t size, size_t nmemb, shoes_curl_write_data *data)
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
shoes_curl_progress_funk(shoes_curl_progress_data *data,
  double dltotal, double dlnow, double ultotal, double ulnow)
{
  shoes_download_event event;
  event.percent = dlnow * 100.0 / dltotal;
  event.transferred = dlnow;
  event.total = dltotal;

  if (data->handler != NULL && (data->handler(&event, data->data) & SHOES_DOWNLOAD_HALT))
    return 1;

  return 0;
}

void
shoes_download(char *host, int port, char *path, char *mem, char *filepath,
  unsigned long long *size, shoes_download_handler handler, void *data)
{
  char url[SHOES_BUFSIZE], uagent[SHOES_BUFSIZE];
  CURL *curl = curl_easy_init();
  CURLcode res;
  shoes_curl_progress_data progress_data;
  shoes_curl_write_data write_data;
  if (curl == NULL) return;

  sprintf(url, "http://%s:%d/%s", host, port, path);
  sprintf(uagent, "Shoes/0.r%d (%s) %s/%d", SHOES_REVISION, SHOES_PLATFORM,
    SHOES_RELEASE_NAME, SHOES_BUILD_DATE);

  progress_data.handler = handler;
  progress_data.data = data;

  write_data.mem = mem;
  write_data.fp = NULL;
  write_data.size = 0;

  if (mem == NULL)
  {
    write_data.fp = fopen(filepath, "wb");
    if (write_data.fp == NULL) return;
  } 

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, shoes_curl_write_funk);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_data);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, shoes_curl_progress_funk);
  curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progress_data);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, uagent);

  res = curl_easy_perform(curl);
  *size = write_data.size;

  if (write_data.fp != NULL)
    fclose(write_data.fp);

  curl_easy_cleanup(curl);
}
