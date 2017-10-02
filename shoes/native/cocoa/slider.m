#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/slider.h"
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

@implementation ShoesSlider
- (id)initWithObject: (VALUE)o
{
  if ((self = [super init]))
  {
    object = o;
    [self setTarget: self];
    [self setAction: @selector(handleChange:)];
  }
  return self;
}
-(IBAction)handleChange: (id)sender
{
  shoes_control_send(object, s_change);
}
@end

// ---- Ruby calls C/obj->C here ----

SHOES_CONTROL_REF
shoes_native_slider(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  INIT;
  ShoesSlider *pop = [[ShoesSlider alloc] initWithObject: self];
  [pop setMinValue: 0.];
  [pop setMaxValue: 100.];
  RELEASE;
  return (NSControl *)pop;
}

double
shoes_native_slider_get_fraction(SHOES_CONTROL_REF ref)
{
  return [(ShoesSlider *)ref doubleValue] * 0.01;
}

void
shoes_native_slider_set_fraction(SHOES_CONTROL_REF ref, double perc)
{
  COCOA_DO([(ShoesSlider *)ref setDoubleValue: perc * 100.]);
}
