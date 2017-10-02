// Shoes Button ssubclasses NSButton.

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/check.h"
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
@implementation ShoesCheckButton
- (id)initWithType: (NSButtonType)t andObject: (VALUE)o
{
  if ((self = [super init]))
  {
    object = o;
    [self setButtonType: t];
    [self setBezelStyle: NSRoundedBezelStyle];
    [self setTarget: self];
    [self setAction: @selector(handleClick:)];
 }
  return self;
}

-(IBAction)handleClick: (id)sender
{
  shoes_button_send_click(object);
}
@end

// ---- Ruby calls C/obj->C here ----
SHOES_CONTROL_REF
shoes_native_check(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  INIT;
  ShoesCheckButton *button = [[ShoesCheckButton alloc] initWithType: NSSwitchButton andObject: self];
  RELEASE;
  return (NSControl *)button;
}

VALUE
shoes_native_check_get(SHOES_CONTROL_REF ref)
{
  return [(ShoesCheckButton *)ref state] == NSOnState ? Qtrue : Qfalse;
}

void
shoes_native_check_set(SHOES_CONTROL_REF ref, int on)
{
  COCOA_DO([(ShoesCheckButton *)ref setState: on ? NSOnState : NSOffState]);
}
