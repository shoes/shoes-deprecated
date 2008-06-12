//
// shoes/native/cocoa.h
// Custom Cocoa interfaces for Shoes
//
#import <Cocoa/Cocoa.h>

@interface ShoesEvents : NSObject
{
}
@end

@interface ShoesWindowEvents : NSObject
{
  VALUE app;
}
@end

@interface ShoesView : NSView
{
  VALUE canvas;
}
@end

@interface ShoesButton : NSButton
{
  VALUE object;
}
@end

@interface ShoesTextField : NSTextField
{
  VALUE object;
}
@end

@interface ShoesTextView : NSScrollView
{
  VALUE object;
  NSTextView *textView;
}
@end

@interface ShoesPopUpButton : NSPopUpButton
{
  VALUE object;
}
@end

@interface ShoesTimer : NSObject
{
  VALUE object;
  NSTimer *timer;
}
@end
