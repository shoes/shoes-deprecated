// Shoes Button ssubclasses NSButton.

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/radio.h"
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

// TODO: Radios are pretty broken and this one is too.
@implementation ShoesRadioButton
- (id)initWithType: (NSButtonType)t andObject: (VALUE)o
{
  if ((self = [super init]))
  {
    object = o;
    [self setButtonType: NSPushOnPushOffButton];  // checkbox for now
	// [self setShoesViewPosition: NSImageOnly]; 
    [self setBezelStyle: NSCircularBezelStyle];
    [self setTarget: self];
    [self setAction: @selector(handleClick:)];
  }
  return self;
}
-(IBAction)handleClick: (id)sender
{
  //NSLog(@"radio button handler called on %lx", object);
  shoes_button_send_click(object);
}
@end

// ---- Ruby calls C/obj->C here ----
SHOES_CONTROL_REF
shoes_native_radio(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, VALUE group)
{
  INIT;
#ifdef OLD_RADIO
	ShoesButton *button = [[ShoesButton alloc] initWithType: NSRadioButton
		   andObject: self];
	 RELEASE;
	 return (NSControl *)button;
#else
 	/*
 	NSLog(@"shoes_native_radio self: %i, %lx", TYPE(self), self);
 	if (NIL_P(group))
 		NSLog(@"group: NIL");
   else
 	  NSLog(@"group: %lx", group);
 	*/
 	ShoesRadioButton *button = [[ShoesRadioButton alloc] initWithType: NSSwitchButton
 		   andObject: self];
 	RELEASE;
 	return (NSControl *)button;
#endif
}
