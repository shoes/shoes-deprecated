#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
//#include "shoes/native/cocoa/timerbase.h"
extern VALUE cTimer;

#define HEIGHT_PAD 6

#define INIT    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE [pool release]
#define COCOA_DO(statements) do {\
  INIT; \
  @try { statements; } \
  @catch (NSException *e) { ; } \
  RELEASE; \
} while (0)

static int start_wait(VALUE); // forward decl

// ---- Cocoa Object side ----

@implementation ShoesTimer
- (id)initWithTimeInterval: (NSTimeInterval)i andObject: (VALUE)o repeats: (BOOL)r
{
  if ((self = [super init]))
  {
    object = o;
    timer = [NSTimer scheduledTimerWithTimeInterval: i
      target: self selector: @selector(animate:) userInfo: self
      repeats: r];
  }
  return self;
}
- (void)animate: (NSTimer *)t
{
  shoes_timer_call(object);
}
- (void)invalidate
{
  [timer invalidate];
}
@end

@implementation CanvasOneShot
- (id)initWithTimeInterval: (NSTimeInterval)i andObject: (VALUE)o repeats: (BOOL)r
{
  if ((self = [super init]))
  {
    object = o;
    timer = [NSTimer scheduledTimerWithTimeInterval: i
      target: self selector: @selector(fired:) userInfo: self
      repeats: r];
     //NSLog(@"one_shot created"); 
  }
  return self;
}
- (void)fired: (NSTimer *)t
{
  start_wait(object);
}
- (void)invalidate
{
  [timer invalidate];
}
@end


// ---- Ruby calls C/obj->C here ----

// ---- timer ---- 

void shoes_native_timer_remove(shoes_canvas *canvas, SHOES_TIMER_REF ref)
{
  COCOA_DO([ref invalidate]);
}

SHOES_TIMER_REF
shoes_native_timer_start(VALUE self, shoes_canvas *canvas, unsigned int interval)
{
  INIT;
  ShoesTimer *timer = [[ShoesTimer alloc] initWithTimeInterval: interval * 0.001
    andObject: self repeats:(rb_obj_is_kind_of(self, cTimer) ? NO : YES)];
  RELEASE;
  return timer;
}

// ---- canvas oneshot new with 3.3.1 -----

static int
start_wait(VALUE data) {
  VALUE rbcanvas = (VALUE)data;
  shoes_canvas *canvas;
  Data_Get_Struct(rbcanvas, shoes_canvas, canvas);
  
  shoes_safe_block(rbcanvas, ATTR(canvas->attr, start), rb_ary_new3(1, rbcanvas));
  //NSLog(@"one_shot finished"); 
  return FALSE; // timeout will be stopped and destroyed
}

void
shoes_native_canvas_oneshot(int ms, VALUE canvas)
{
  INIT;
  // CanvasOneShot *timer = 
  [[CanvasOneShot alloc] initWithTimeInterval: ms * 0.001
    andObject: canvas repeats: NO];
  RELEASE;
  //return timer;
}
