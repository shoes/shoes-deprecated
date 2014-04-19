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
  VALUE path, url, opts, svsym, dnobj;
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
  // convert the fields of dnobj to C ptrs
 
  printf("returned from image_download_sync\n");
  //shoes_http_request_free(req);
 // free(req);

}

VALUE
shoes_http_err(SHOES_DOWNLOAD_ERROR code)
{
  /* a little unclear what this does or what it returns
  I think it converts the platform 'code' to a Shoes string
  */
  const char *errorString = "Success"; /* FIXME: */
  printf("shoes_http_err called\n");
  return rb_str_new2(errorString);
}

SHOES_DOWNLOAD_HEADERS
shoes_http_headers(VALUE hsh)
{
  // appears to copy the Ruby headers into a objc dict
  // presumably these are the header sent TO the server
  // unsure who is calling this or when
  /* long i;
  NSDictionary *d = NULL;
  VALUE keys = rb_funcall(hsh, s_keys, 0);
  if (RARRAY_LEN(keys) > 0)
  {
    d = [[NSMutableDictionary dictionaryWithCapacity: RARRAY_LEN(keys)] retain];
    for (i = 0; i < RARRAY_LEN(keys); i++)
    {
      VALUE key = rb_ary_entry(keys, i);
      VALUE val = rb_hash_aref(hsh, key);
      if(!NIL_P(val))
        [d setValue: [NSString stringWithUTF8String: RSTRING_PTR(val)]
           forKey:   [NSString stringWithUTF8String: RSTRING_PTR(key)]];
    }
  }
  return d;
  */
  printf("shoes_http_headers called\n");
  return (SHOES_DOWNLOAD_HEADERS)hsh;
}

void
shoes_http_headers_free(SHOES_DOWNLOAD_HEADERS headers)
{
  /*
    [headers release];
  */
  printf("shoes_headers_free called\n");
}
