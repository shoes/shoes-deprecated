#import <Cocoa/Cocoa.h>

#define INIT    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE [pool release]
NSString *shoesSite;
NSString *shoesPath;

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

/*
 * NSURLConnection code to download the selector value
 * a single line of text with the servers file path to really download
 * from. Creates a NSURL with that and starts the file download
*/
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
/*  
 * here when the single line (a server path) is downloaded. It creates
 * a NSURLRequest with that and the server string and
 * 
*/
- (void)connectionDidFinishLoading: (NSURLConnection *)conn
{
  if (metaConn == conn)
  {
    NSString *pkgPath;
    NSURLRequest *req;
    setupURL = [[NSString alloc] initWithData: data
      encoding: NSUTF8StringEncoding];
    // setupURL = [@"http://hacketyhack.net" stringByAppendingString:
    //setupURL = [@"http://shoes.mvmanila.com" stringByAppendingString:
    setupURL = [shoesSite stringByAppendingString:
      [setupURL stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceAndNewlineCharacterSet]]];

    req = [NSURLRequest requestWithURL: [NSURL URLWithString: setupURL]
      cachePolicy: NSURLRequestUseProtocolCachePolicy timeoutInterval: 60.0];
    pkgDown = [[NSURLDownload alloc] initWithRequest: req delegate: self];
    [conn release];
  }
}

/*
 *  File downloading delegates below
*/
- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
   NSString *destinationFilename;
   NSString *homeDirectory = NSHomeDirectory();
   destinationFilename = [[homeDirectory stringByAppendingPathComponent:@".shoes/install/"]
     stringByAppendingPathComponent:filename];
   [download setDestination:destinationFilename allowOverwrite:YES];
}

- (void)setDownloadResponse: (NSURLResponse *)aDownloadResponse
{
  [aDownloadResponse retain];
  [pkgResp release];
  pkgResp = aDownloadResponse;
}

// Don't unzip the download
- (BOOL)download:(NSURLDownload *)download shouldDecodeSourceDataOfMIMEType:(NSString *)encodingType
{
  return NO;
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
    // Inform the user.
    NSLog(@"Download failed! Error - %@ %@",
          [error localizedDescription],
          [[error userInfo] objectForKey:NSURLErrorFailingURLStringErrorKey]);
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
  [download release];
  [[NSApplication sharedApplication] terminate: self];
}
@end

int
main(int argc, char *argv[])
{
  NSApplication *app = [NSApplication sharedApplication];
  INIT;
	// copy the args to globals
	shoesSite = [NSString stringWithUTF8String: argv[1]];
	shoesPath = [NSString stringWithUTF8String: argv[2]];
	NSString *shoesSelect = [NSString stringWithString: shoesSite];
	shoesSelect = [shoesSelect stringByAppendingString: shoesPath];
		
  NSWindow *win = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, 340, 140)
    styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSTexturedBackgroundWindowMask)
    backing: NSBackingStoreBuffered defer: NO];
  NSProgressIndicator *pop = [[[NSProgressIndicator alloc] initWithFrame: 
    NSMakeRect(40, 68, 260, 14)] autorelease];
  NSButton *button = [[[NSButton alloc] initWithFrame: 
    NSMakeRect(130, 20, 100, 30)] autorelease];
  NSTextField *text = [[[NSTextField alloc] initWithFrame:
    NSMakeRect(40, 90, 260, 18)] autorelease];
  StubEvents *events = [[StubEvents alloc] initWithWindow: win andText: text andProgress: pop];
	
  //[events checkForLatestShoesAt: @"http://hacketyhack.net/pkg/osx/shoes"];
  //[events checkForLatestShoesAt: @"http://shoes.mvmanila.com/public/select/osx.rb"];
  [events checkForLatestShoesAt: shoesSelect];
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
  [win setLevel: NSFloatingWindowLevel];
  [win makeKeyAndOrderFront: win];
  [app run];
  RELEASE;
  return 0;
}
