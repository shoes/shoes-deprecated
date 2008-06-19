#import <Cocoa/Cocoa.h>

#define INIT    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE [pool release]

@interface StubEvents : NSObject
{
  NSWindow *win;
  NSFileManager *fm;
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
- (id)initWithWindow: (NSWindow *)w andText: (NSTextField *)text andProgress: (NSProgressIndicator *)pop
{
  if ((self = [super init]))
  {
    data = [[NSMutableData data] retain];
    fm = [NSFileManager defaultManager];
    win = w;
    progress = pop;
    textField = text;
  }
  return self;
}
- (NSString *)getTempPath
{
  NSString *temp = NSTemporaryDirectory(), *p;
  if (temp == NULL) temp = NSHomeDirectory();
  return temp;
}
-(IBAction)cancelClick: (id)sender
{
  [[NSApplication sharedApplication] stop: nil];
}
- (void)checkForLatestShoesAt: (NSString *)url
{
  NSURLRequest *req = [NSURLRequest requestWithURL: 
    [NSURL URLWithString: url]
    cachePolicy: NSURLRequestUseProtocolCachePolicy
    timeoutInterval: 60.0];
  metaConn = [[NSURLConnection alloc] initWithRequest: req
    delegate: self];
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
  NSString *shoesMount, *shoesPath;
  NSTask *mount = [[NSTask alloc] init];
  [mount setLaunchPath: @"/usr/bin/hdiutil"];
  [mount setArguments: [NSArray arrayWithObjects: 
    @"mount", @"-noidme", @"-quiet", @"-mountroot", [self getTempPath], pkgPath, nil]];
  [mount launch];
  [mount waitUntilExit];
  [mount release];

  shoesMount = [[self getTempPath] stringByAppendingPathComponent: @"Shoes"];
  shoesPath = [shoesMount stringByAppendingPathComponent: @"Shoes.app"]; 
  [fm copyPath: shoesPath toPath: @"/Applications/Shoes.app" handler: nil];

  mount = [[NSTask alloc] init];
  [mount setLaunchPath: @"/usr/bin/hdiutil"];
  [mount setArguments: [NSArray arrayWithObjects: @"unmount", @"-quiet", shoesMount, nil]];
  [mount launch];
  [mount waitUntilExit];
  [mount release];
  [download release];
  [[NSApplication sharedApplication] stop: nil];
}
@end

int
main(int argc, char *argv[])
{
  NSApplication *app = [NSApplication sharedApplication];
  INIT;
  NSWindow *win = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, 340, 140)
    styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask)
    backing: NSBackingStoreBuffered defer: NO];
  NSProgressIndicator *pop = [[[NSProgressIndicator alloc] initWithFrame: 
    NSMakeRect(40, 68, 260, 14)] autorelease];
  NSButton *button = [[[NSButton alloc] initWithFrame: 
    NSMakeRect(130, 20, 100, 30)] autorelease];
  NSTextField *text = [[[NSTextField alloc] initWithFrame:
    NSMakeRect(40, 90, 260, 18)] autorelease];
  StubEvents *events = [[StubEvents alloc] initWithWindow: win andText: text andProgress: pop];
  [events checkForLatestShoesAt: @"http://hacketyhack.net/pkg/osx/shoes"];

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
  [button setTarget: events];
  [button setAction: @selector(cancelClick:)];

  [win center];
  [win orderFrontRegardless];
  RELEASE;
  [app run];
  return 0;
}
