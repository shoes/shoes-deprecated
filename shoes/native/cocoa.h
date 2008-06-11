//
// shoes/native/cocoa.h
// Custom Cocoa interfaces for Shoes
//
#import <Cocoa/Cocoa.h>

@interface ShoesEvents : NSObject
{
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

@interface ShoesTextField : NSSecureTextField
{
  VALUE object;
}
@end

@interface ShoesPopUpButton : NSPopUpButton
{
  VALUE object;
}
@end
