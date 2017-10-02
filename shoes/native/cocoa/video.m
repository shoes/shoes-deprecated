// Shoes Video is weird. 

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/video.h"
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

@implementation ShoesVideoView
- (id)initWithFrame: (NSRect)frame andVideo: (VALUE)vid
{
  if ((self = [super initWithFrame: frame]))
  {
    video = vid;
    // Paint It Black
    [self setWantsLayer:YES];
    [self.layer setBackgroundColor:[[NSColor blackColor] CGColor]];
  }
  return self;
}

- (void)viewDidMoveToWindow
{
  if (self.window != nil)    // shows how to access a property. who knew?
  {
    // NSLog(@"vid moved to Window");
    shoes_video *vid;
    Data_Get_Struct(video, shoes_video, vid);
    vid->realized = 1;
  }
}
@end


// ---- Ruby calls C/obj->C here ----

// This changed in 3.3.1 - only video uses it


SHOES_CONTROL_REF
shoes_native_surface_new(VALUE attr, VALUE video)
{
  // Create an NSView
  int w = NUM2INT(ATTR(attr, width));
  int h = NUM2INT(ATTR(attr, height));
  NSRect rect = NSMakeRect(0, 0, w, h);
  ShoesVideoView *nativeView = [[ShoesVideoView alloc] initWithFrame: rect andVideo: video];
  // Paint It Black  Hot Stuff !
  //[nativeView setWantsLayer:YES];
  //[nativeView.layer setBackgroundColor:[[NSColor blackColor] CGColor]];
  //shoes_video *vid;
  //Data_Get_Struct(video, shoes_video, vid);
  //vid->realized = 1;
  return (SHOES_CONTROL_REF)nativeView;
}


void
shoes_native_surface_position(SHOES_SURFACE_REF ref, shoes_place *p1,
  VALUE self, shoes_canvas *canvas, shoes_place *p2)
{
  PLACE_COORDS();
}

void
shoes_native_surface_hide(SHOES_SURFACE_REF ref)
{
  ShoesVideoView *vidView = (ShoesVideoView *)ref;
  vidView.hidden = true;
  // HIViewSetVisible(vidView, false); // probably wrong
}

void
shoes_native_surface_show(SHOES_SURFACE_REF ref)
{
  ShoesVideoView *vidView = (ShoesVideoView *)ref;
  vidView.hidden = false;
  // HIViewSetVisible(vidView, true); // probably wrong
}

void
shoes_native_surface_remove(SHOES_SURFACE_REF ref)
{
  COCOA_DO([(ShoesVideoView *)ref removeFromSuperview]);
}
