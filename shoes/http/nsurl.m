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
- (void)download: (NSString *)url string: (char *)mem filepath: (char *)filepath size: (unsigned long long *)size handler: (shoes_download_handler)handler data: (void *)data
{
  NSString *uagent = [NSString stringWithFormat: @"Shoes/0.r%d (%s) %s/%d", 
    SHOES_REVISION, SHOES_PLATFORM, SHOES_RELEASE_NAME, SHOES_BUILD_DATE];
  NSURLRequest *req = [NSURLRequest requestWithURL: 
    [NSURL URLWithString: url]
    cachePolicy: NSURLRequestUseProtocolCachePolicy
    timeoutInterval: 60.0];
  metaConn = [[NSURLConnection alloc] initWithRequest: req
    delegate: self];
}
@end

void
shoes_download(char *host, int port, char *path, char *mem, char *filepath,
  unsigned long long *size, shoes_download_handler handler, void *data)
{
  ShoesHttp *http = [[ShoesHttp alloc] init];
  [http download: [NSString stringWithFormat: @"http://%s:%d/%s", host, port, path]
    string: mem filepath: filepath size: size handler: handler data: data];
}
