//
// shoes/http/rbload
// the downloader routines using ruby's open_uri  
// Might still have some nsurl.m stuff for comments
// Cecil Coupe screwed this together. 
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/internal.h"
#include "shoes/config.h"
#include "shoes/http.h"
#include "shoes/version.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/canvas.h"

void
shoes_download(shoes_http_request *req)
{
  /* monkey patched out in download.rb - need function for linking */
  printf("monkey patch failed\n");
}

void
shoes_queue_download(shoes_http_request *req)
{
  //printf("shoes_queue_download called: %s -> %s\n",req->url,req->filepath);
  VALUE path, url, opts, svsym, dnobj, stvar;
  // convert req->url, req->filepath to ruby strings
  path = rb_str_new2(req->filepath);
  url = rb_str_new2(req->url);
  // make a hash
  opts = rb_hash_new();
  // make a :save symbol
  svsym = ID2SYM(rb_intern("save"));
  // add :save and filepath to hash
  rb_hash_aset(opts, svsym, path);
  // Call Shoes::image_download_sync - defined in image.rb
  dnobj = rb_funcall(cShoes, 
      rb_intern("image_download_sync"), 2, url, opts);
  // convert the status var of dnobj to a C int and save it in req->idat
  stvar = rb_funcall(dnobj, rb_intern("status"), 0);
  int st = NUM2INT(stvar);
  // dig the etag field out of headers.
  VALUE hdrs = rb_funcall(dnobj, rb_intern("headers"), 0);
  //printf("returned from image_download_sync %i\n", st);
  // shoes_image_download_event *idat
  // shoes_image_download_event in req->data 
  shoes_image_download_event *side = req->data;
  side->status = st;
  VALUE etag = NULL;
  VALUE keys = rb_funcall(hdrs, s_keys, 0);
  int i;
  for (i = 0; i < RARRAY_LEN(keys); i++ )
  {
    VALUE key = rb_ary_entry(keys, i);
    if (strcmp("etag", RSTRING_PTR(key)) == 0) {
		etag = key;
		break;
    }
    //printf("key=%s\n",RSTRING_PTR(key));
  }
  side->etag = RSTRING_PTR(rb_hash_aref(hdrs, etag));
  //VALUE rbetag = rb_hash_aref(hdrs, rb_intern("etag"));
  //printf("%s\n",RSTRING_PTR(rbetag));
  shoes_catch_message(SHOES_IMAGE_DOWNLOAD, NULL, side);
  shoes_http_request_free(req);
  free(req);
  // assume Ruby will garbage collect all the VALUE var's. 
  // after this stack frame pops there's nothing holding them.
}

VALUE
shoes_http_err(SHOES_DOWNLOAD_ERROR code)
{
  /* a little unclear what this does or what it returns
  I think it converts the platform 'code' to a Shoes string
  */
  const char *errorString;
  sprintf(errorString, "%i", code);
  printf("shoes_http_err called%s\n",errorString);
  return rb_str_new2(errorString);
}

SHOES_DOWNLOAD_HEADERS
shoes_http_headers(VALUE hsh)
{
 
  //printf("shoes_http_headers called\n");
  return (SHOES_DOWNLOAD_HEADERS)hsh;
}

void
shoes_http_headers_free(SHOES_DOWNLOAD_HEADERS headers)
{
 // printf("shoes_headers_free called\n");
}
