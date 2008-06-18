#import <Cocoa/Cocoa.h>

#define INIT    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE [pool release]

@interface StubEvents : NSObject
{
  NSMutableData *data;
  NSString *setupURL;
  NSURLConnection *metaConn;
  NSURLDownload *pkgDown;
  NSURLResponse *pkgResp;
  NSString *pkgPath;
  NSFileHandle *tempDir, *pkgFile;
  NSTextField *textField;
  NSProgressIndicator *progress;
  long long bytes;
}
@end

@implementation StubEvents
- (id)init
{
  if ((self = [super init]))
  {
    data = [[NSMutableData data] retain];
  }
  return self;
}
- (NSString *)getTempPath
{
  NSString *temp = NSTemporaryDirectory(), *p;
  if (temp == NULL) temp = NSHomeDirectory();
  return temp;
}
- (void)setMetaConn: (NSURLConnection *)conn andText: (NSTextField *)text andProgress: (NSProgressIndicator *)pop
{
  metaConn = conn;
  progress = pop;
  textField = text;
}
- (void)connection: (NSURLConnection *)conn didReceiveResponse: (NSURLResponse *)resp
{
  [data setLength: 0];
}
- (void)connection: (NSURLConnection *)conn didReceiveData: (NSData *)chunk
{
  [data appendData: chunk];
}
- (void)connectionDidFinishLoading: (NSURLConnection *)conn
{
  if (metaConn == conn)
  {
    NSString *pkgPath;
    NSURLRequest *req;
    setupURL = [[NSString alloc] initWithData: data
      encoding: NSUTF8StringEncoding];
    setupURL = [@"http://hacketyhack.net" stringByAppendingString:
      [setupURL stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceAndNewlineCharacterSet]]];

    req = [NSURLRequest requestWithURL: [NSURL URLWithString: setupURL]
      cachePolicy: NSURLRequestUseProtocolCachePolicy timeoutInterval: 60.0];
    pkgDown = [[NSURLDownload alloc] initWithRequest: req delegate: self];
    [conn release];
  }
}
- (void)download: (NSURLDownload *)download decideDestinationWithSuggestedFilename: (NSString *)filename
{
  pkgPath = [[self getTempPath] stringByAppendingPathComponent: @"shoes-setup.dmg"];
  [download setDestination: pkgPath allowOverwrite: YES];
}
- (void)setDownloadResponse: (NSURLResponse *)aDownloadResponse
{
  [aDownloadResponse retain];
  [pkgResp release];
  pkgResp = aDownloadResponse;
}
 
- (void)download: (NSURLDownload *)download didReceiveResponse: (NSURLResponse *)response
{
  bytes = 0;
  [self setDownloadResponse: response];
}
- (void)download: (NSURLDownload *)download didReceiveDataOfLength: (unsigned)length
{
  long long expectedLength = [pkgResp expectedContentLength];
  bytes += length;
  if (expectedLength != NSURLResponseUnknownLength) {
    float perc = (bytes / (float)expectedLength) * 100.0;
    [textField setStringValue: [NSString stringWithFormat: @"Downloading Shoes. (%0.1f%% done)", perc]];
    [progress setDoubleValue: perc];
  }
}
- (void)downloadDidFinish: (NSURLDownload *)download
{
  NSFileManager *fm;
  NSString *shoesMount, *shoesPath;
  NSTask *mount = [[NSTask alloc] init];
  [mount setLaunchPath: @"/usr/bin/hdiutil"];
  [mount setArguments: [NSArray arrayWithObjects: 
    @"mount", @"-noidme", @"-quiet", @"-mountroot", [self getTempPath], pkgPath, nil]];
  [mount launch];
  printf("Mounting %s\n", [pkgPath UTF8String]);
  [mount waitUntilExit];
  [mount release];

  shoesMount = [[self getTempPath] stringByAppendingPathComponent: @"Shoes"];
  shoesPath = [shoesMount stringByAppendingPathComponent: @"Shoes.app"]; 
  fm = [NSFileManager defaultManager];
  [fm copyPath: shoesPath toPath: @"/Applications/Shoes.app" handler: nil];

  mount = [[NSTask alloc] init];
  [mount setLaunchPath: @"/usr/bin/hdiutil"];
  [mount setArguments: [NSArray arrayWithObjects: @"unmount", @"-quiet", shoesMount, nil]];
  printf("Unounting %s\n", [shoesMount UTF8String]);
  [mount launch];
  [mount waitUntilExit];
  [mount release];

  [download release];
}
@end

int
main(int argc, char *argv[])
{
  NSApplication *app = [NSApplication sharedApplication];
  INIT;
  StubEvents *events = [[StubEvents alloc] init];
  NSWindow *win = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, 340, 140)
    styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask)
    backing: NSBackingStoreBuffered defer: NO];
  NSProgressIndicator *pop = [[[NSProgressIndicator alloc] initWithFrame: 
    NSMakeRect(40, 68, 260, 14)] autorelease];
  NSButton *button = [[[NSButton alloc] initWithFrame: 
    NSMakeRect(130, 20, 100, 30)] autorelease];
  NSTextField *text = [[[NSTextField alloc] initWithFrame:
    NSMakeRect(40, 90, 260, 18)] autorelease];
  NSURLRequest *req = [NSURLRequest requestWithURL: 
    [NSURL URLWithString: @"http://hacketyhack.net/pkg/osx/shoes"]
    cachePolicy: NSURLRequestUseProtocolCachePolicy
    timeoutInterval: 60.0];
  NSURLConnection *conn = [[NSURLConnection alloc] initWithRequest: req
    delegate: events];
  [events setMetaConn: conn andText: text andProgress: pop];

  [[win contentView] addSubview: text];
  [text setStringValue: @"Downloading Shoes."];
  [text setBezeled: NO];
  [text setBackgroundColor: [NSColor windowBackgroundColor]];
  [text setEditable: NO];
  [text setSelectable: NO];

  [[win contentView] addSubview: pop];
  [pop setIndeterminate: FALSE];
  [pop setDoubleValue: 0.];
  [pop setBezeled: YES];

  [[win contentView] addSubview: button];
  [button setTitle: @"Cancel"];
  [button setBezelStyle: 1];

  // [win center];
  [win orderFrontRegardless];
  RELEASE;
  printf("RUN?\n");
  [app run];
  return 0;
}
