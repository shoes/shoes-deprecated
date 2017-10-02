// Shoes Button ssubclasses NSButton.

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/button.h"
extern VALUE cTimer;

//#import <Carbon/Carbon.h>

#define HEIGHT_PAD 6

#define INIT    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE [pool release]
#define COCOA_DO(statements) do {\
  INIT; \
  @try { statements; } \
  @catch (NSException *e) { ; } \
  RELEASE; \
} while (0)

// ---- Cocoa Object side ----
@implementation ShoesButton
- (id)initWithType: (NSButtonType)t andObject: (VALUE)o
{
  if ((self = [super init]))
  {
    object = o;
    [self setButtonType: t];
    [self setBezelStyle: NSRoundedBezelStyle];
    [self setTarget: self];
    [self setAction: @selector(handleClick:)];
    //NSLog(@"button w/o frame called");
 }
  return self;
}
- (id)initWithTypeF: (NSButtonType)t withFrame:(NSRect)rect andObject: (VALUE)o
{
  if ((self = [super initWithFrame: rect]))
  {
    object = o;
    [self setButtonType: t];
    [self setBezelStyle: NSRoundedBezelStyle];
    [self setTarget: self];
    [self setAction: @selector(handleClick:)];
    NSLog(@"button with frame w: %i h: %i\n", (int) NSWidth(rect), (int)NSHeight(rect));
  }
  return self;
}
-(IBAction)handleClick: (id)sender
{
  shoes_button_send_click(object);
}
@end

// ---- Ruby calls C/obj->C here ----
SHOES_CONTROL_REF shoes_native_button(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  INIT;
  ShoesButton *button = [[ShoesButton alloc] initWithType: NSMomentaryPushInButton
    andObject: self];
  [button setTitle: [NSString stringWithUTF8String: msg]];
  RELEASE;
  return (NSControl *)button;
}
