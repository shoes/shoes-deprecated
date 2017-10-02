#import <Cocoa/Cocoa.h>
@interface ShoesTextField : NSTextField
{
  VALUE object;
}
- (void)textDidEndEditing: (NSNotification *)note;

@end

@interface ShoesSecureTextField : NSSecureTextField
{
  VALUE object;
}
@end
