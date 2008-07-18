//
// shoes/http/nsurl.m
// the downloader routines using nsurl classes.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/http.h"
#include "shoes/version.h"

#import <Cocoa/Cocoa.h>

@interface ShoesHttp : NSObject
{
  NSURLConnection *metaConn;
  long long bytes;
}
@end

@implementation ShoesHttp
- (void)download: (shoes_download_request *)req
{
  NSString *url = [NSString stringWithFormat: @"http://%s:%d/%s", req->host, req->port, req->path];
  NSString *uagent = [NSString stringWithFormat: @"Shoes/0.r%d (%s) %s/%d", 
    SHOES_REVISION, SHOES_PLATFORM, SHOES_RELEASE_NAME, SHOES_BUILD_DATE];
  NSURLRequest *nsreq = [NSURLRequest requestWithURL: 
    [NSURL URLWithString: url]
    cachePolicy: NSURLRequestUseProtocolCachePolicy
    timeoutInterval: 60.0];
  metaConn = [[NSURLConnection alloc] initWithRequest: nsreq
    delegate: self];
}
@end

void
shoes_download(shoes_download_request *req)
{
  ShoesHttp *http = [[ShoesHttp alloc] init];
  [http download: req];
}

void
shoes_queue_download(shoes_download_request *req)
{
}
