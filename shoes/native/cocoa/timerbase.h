#import <Cocoa/Cocoa.h>
@interface ShoesTimer : NSObject
{
  VALUE object;
  NSTimer *timer;
}
@end

// Who needs this? shoes_canvas_send_start (canvas.c)
@interface CanvasOneShot : NSObject
{
  VALUE object;
  NSTimer *timer;
}
@end
