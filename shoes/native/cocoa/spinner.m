// Shoes RadioButton subclasses NSButton.

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/spinner.h"
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

/*
 * ---- spinner ----
 * subclass NSProgressIndicator  for better control. Sadly, this is not a cocoa control
 * so it can't be first class Shoes control
*/

@implementation ShoesSpinner
- (id)initWithAnObject: (VALUE)o
{
  if ((self = [super init]))
  {
    object = o;
    self.indeterminate = true;
    [self setStyle: NSProgressIndicatorSpinningStyle];
    [self setBezeled: true];
  }
  return self;
}
@end

// ---- Ruby calls C/obj->C here ----

void shoes_native_spinner_start(SHOES_CONTROL_REF ref)
{
  ShoesSpinner *spin = (ShoesSpinner *)ref;
  spin->state = true;
  [spin startAnimation: (id)ref]; 
}
void shoes_native_spinner_stop(SHOES_CONTROL_REF ref)
{
  ShoesSpinner *spin = (ShoesSpinner *)ref;
  spin->state = false;
  [spin stopAnimation: (id)ref]; 
}

int shoes_native_spinner_started(SHOES_CONTROL_REF ref)
{
  ShoesSpinner *spin = (ShoesSpinner *)ref;
  return spin->state;
}


SHOES_CONTROL_REF shoes_native_spinner(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  ShoesSpinner *spin  = [[ShoesSpinner alloc] initWithAnObject: self];
  spin->state = false;
  if (!NIL_P(shoes_hash_get(attr, rb_intern("start")))) {
    spin->state = (Qtrue == shoes_hash_get(attr, rb_intern("start"))) ?  true: false; 
  }
  if (spin->state == true)
    shoes_native_spinner_start((SHOES_CONTROL_REF)spin);
  else
    shoes_native_spinner_stop((SHOES_CONTROL_REF)spin);
  return (SHOES_CONTROL_REF) spin;
}
