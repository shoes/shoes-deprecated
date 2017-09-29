// Shoes RadioButton subclasses NSButton.

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/switch.h"
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
/*
 * ---- switch ----
 * subclass  NSButton and see what it looks like as ToggleButton
 * needs it own action handlers 
*/
@implementation ShoesSwitch
- (id)initWithType: (NSButtonType)t andObject: (VALUE)o
{
  if ((self = [super init]))
  {
    object = o;
    [self setButtonType: t];
    [self setBezelStyle: NSRoundedBezelStyle];
    [self setTarget: self];
    [self setAction: @selector(handleClick:)];
    [self setState: NSOffState];
    //sw_state = 0;
  }
  return self;
}
-(IBAction)handleClick: (id)sender
{
  //fprintf(stderr, "handler: %li\n", [self state]);
  //sw_state = sw_state ^ 1;
  //fprintf(stderr, "h after: %li\n", [self state]);
  shoes_control_send(object, s_active);
}
@end
// ---- Ruby calls C/obj->C here ----

SHOES_CONTROL_REF
shoes_native_switch(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
 INIT;
  ShoesSwitch *button = [[ShoesSwitch alloc] initWithType: NSToggleButton
    andObject: self];
  [button setTitle: @"Off"];
  [button setAlternateTitle: @"On"];
  if (!NIL_P(shoes_hash_get(attr, rb_intern("active")))) {
    VALUE bstv = shoes_hash_get(attr, rb_intern("active"));
    button.state = !NIL_P(bstv) ? NSOnState : NSOffState;
    //fprintf(stderr, "have a initial active %li\n",button.state);
  }
  //button->sw_state = button.state; //property -> instance_var
  RELEASE;
  return (SHOES_CONTROL_REF) button;
}

void shoes_native_switch_set_active(SHOES_CONTROL_REF ref, int activate)
{
  ShoesSwitch *button = (ShoesSwitch *)ref;  
  //fprintf(stderr, "Set_active = %i\n", activate);
  NSInteger bst = activate ? NSOnState : NSOffState;
  [button setState: bst];
}

VALUE
shoes_native_switch_get_active(SHOES_CONTROL_REF ref)
{
  ShoesSwitch *button = (ShoesSwitch *)ref;  
  //fprintf(stderr, "get_active -> %li\n", [button state]);
  return [button state]  ?  Qtrue : Qfalse;
}
