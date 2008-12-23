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
  size_t size, total, readpos, bodylen;
  unsigned long status;
  shoes_http_handler handler;
  SHOES_TIME last;
  CURL *curl;
  void *data;
} shoes_curl_data;

const char *content_len_str = "Content-Length: ";

size_t
shoes_curl_header_funk(char *ptr, size_t size, size_t nmemb, void *user)
{
  shoes_curl_data *data = (shoes_curl_data *)user;
  size_t realsize = size * nmemb;
  if (data->status == 0 && data->handler != NULL)
  {
    curl_easy_getinfo(data->curl, CURLINFO_RESPONSE_CODE, (long *)&data->status);
    if (data->handler != NULL)
    {
      shoes_http_event *event = SHOE_ALLOC(shoes_http_event);
      SHOE_MEMZERO(event, shoes_http_event, 1);
      event->stage = SHOES_HTTP_STATUS;
      event->status = data->status;
      data->handler(event, data->data);
      SHOE_FREE(event);
    }
  }

  HTTP_HEADER(ptr, realsize, data->handler, data->data);
  if ((data->mem != NULL || data->fp != NULL) &&
      strncmp(ptr, content_len_str, strlen(content_len_str)) == 0)
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
shoes_curl_read_funk(void *ptr, size_t size, size_t nmemb, void *user)
{
  shoes_curl_data *data = (shoes_curl_data *)user;
  size_t realsize = size * nmemb;

  if (realsize > data->bodylen - data->readpos)
    realsize = data->bodylen - data->readpos;
fprintf(stderr, "realsize: %i, readpos: %i, bodylen: %i\n", realsize, data->readpos, data->bodylen);
  SHOE_MEMCPY(ptr, &(data->body[data->readpos]), char, realsize);
  data->readpos += realsize;
  return realsize;
}

size_t
shoes_curl_write_funk(void *ptr, size_t size, size_t nmemb, void *user)
{
  shoes_curl_data *data = (shoes_curl_data *)user;
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
    realsize = fwrite(ptr, size, nmemb, data->fp) * size;
  if (realsize > 0)
    data->size += realsize;
  return realsize;
}

int
shoes_curl_progress_funk(void *user, double dltotal, double dlnow, double ultotal, double ulnow)
{
  shoes_curl_data *data = (shoes_curl_data *)user;
  if (dltotal > 0.)
  {
    HTTP_EVENT(data->handler, SHOES_HTTP_TRANSFER, data->last, dlnow * 100.0 / dltotal, dlnow,
               dltotal, data->data, NULL, return 1);
  }
  return 0;
}

static
void dump(const char *text,
          FILE *stream, unsigned char *ptr, size_t size)
{
  size_t i;
  fprintf(stream, "%s, %zd bytes (0x%zx)\n", text, size, size);

  for(i=0; i<size; i+= 1) {
    /* check for 0D0A; if found, skip past and start a new line of output */
    if((ptr[i]>=0x20) && (ptr[i]<0x80))
      fprintf(stream, "%c", ptr[i]);
    else if (ptr[i]==0x0A)
      fputc('\n', stream); /* newline */
    else if (ptr[i]==0x0D && ptr[i+1]==0x0A) {
      fputc('\n', stream); /* newline */
      i+= 1;
    }
    else
      fputc('.', stream);
  }
  fputc('\n', stream);
  fflush(stream);
}


static
int my_trace(CURL *handle, curl_infotype type,
             unsigned char *data, size_t size,
             void *userp)
{
  const char *text;

  switch (type) {
  case CURLINFO_TEXT:
    INFO("HTTP: %s\n", data);
  default: /* in case a new one is introduced to shock us */
    return 0;

  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    break;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    break;
  case CURLINFO_SSL_DATA_OUT:
    text = "<= Send SSL data";
    break;
  }

  INFO("HTTP: %s\n", text);
  dump(text, stderr, data, size);
  return 0;
}

void
shoes_download(shoes_http_request *req)
{
  char uagent[SHOES_BUFSIZE];
  CURL *curl = curl_easy_init();
  CURLcode res;
  shoes_curl_data *cdata = SHOE_ALLOC(shoes_curl_data);
  if (curl == NULL) return;

  sprintf(uagent, "Shoes/0.r%d (%s) %s/%d", SHOES_REVISION, SHOES_PLATFORM,
    SHOES_RELEASE_NAME, SHOES_BUILD_DATE);

  cdata->mem = req->mem;
  cdata->memlen = req->memlen;
  cdata->fp = NULL;
  cdata->size = cdata->readpos = req->size = 0;
  cdata->total = 0;
  cdata->handler = req->handler;
  cdata->data = req->data;
  cdata->last.tv_sec = 0;
  cdata->last.tv_nsec = 0;
  cdata->status = 0;
  cdata->curl = curl;
  cdata->body = NULL;

  if (req->mem == NULL && req->filepath != NULL)
  {
    cdata->fp = fopen(req->filepath, "wb");
    if (cdata->fp == NULL) goto done;
  } 

  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, TRUE);
  curl_easy_setopt(curl, CURLOPT_URL, req->url);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, shoes_curl_header_funk);
  curl_easy_setopt(curl, CURLOPT_WRITEHEADER, cdata);
  if (cdata->mem != NULL || cdata->fp != NULL)
  {
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, shoes_curl_write_funk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, cdata);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, shoes_curl_progress_funk);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, cdata);
  }
  else
  {
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
  }
  curl_easy_setopt(curl, CURLOPT_USERAGENT, uagent);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
  curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
  if (req->flags & SHOES_DL_REDIRECTS)
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  if (req->method)
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req->method);
  if (req->headers)
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, req->headers);
  if (req->body)
  {
    cdata->body = req->body;
    cdata->bodylen = req->bodylen;
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
    curl_easy_setopt(curl, CURLOPT_INFILE, cdata);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, cdata->bodylen);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, shoes_curl_read_funk);
  }

  res = curl_easy_perform(curl);
  req->size = cdata->size;
  req->mem = cdata->mem;

  if (res != CURLE_OK)
  {
    if (req->handler != NULL)
    {
      shoes_http_event *event = SHOE_ALLOC(shoes_http_event);
      SHOE_MEMZERO(event, shoes_http_event, 1);
      event->stage = SHOES_HTTP_ERROR;
      event->error = res;
      req->handler(event, req->data);
      SHOE_FREE(event);
    }
    goto done;
  }

  if (cdata->fp != NULL)
  { 
    fclose(cdata->fp);
    cdata->fp = NULL;
  }

  HTTP_EVENT(cdata->handler, SHOES_HTTP_COMPLETED, cdata->last, 100, req->size, req->size, cdata->data, req->mem, goto done);

done:
  if (cdata->fp != NULL)
    fclose(cdata->fp);
  if (cdata != NULL)
    SHOE_FREE(cdata);

  curl_easy_cleanup(curl);
}

void *
shoes_download2(void *data)
{
  shoes_http_request *req = (shoes_http_request *)data;
  shoes_download(req);
  shoes_http_request_free(req);
  free(req);
  return NULL;
}

void
shoes_queue_download(shoes_http_request *req)
{
  pthread_t tid;
  pthread_create(&tid, NULL, shoes_download2, req);
}

VALUE
shoes_http_err(SHOES_DOWNLOAD_ERROR code)
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

void
shoes_http_headers_free(struct curl_slist *slist)
{
  curl_slist_free_all(slist);
}
