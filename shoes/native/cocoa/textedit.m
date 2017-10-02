#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/textview.h"
#include "shoes/native/cocoa/textedit.h"
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

// new for 3.2.25 - Subclass of ShoesTextView

@implementation ShoesTextEditView
- (id)initWithFrame: (NSRect)frame andObject: (VALUE)o
{
  printf( "cocoa: creating text_edit_view_frame\n");
  if ((self = [super initWithFrame: frame]))
  {
    printf("cocoa: creating text_edit_frame\n");
    object = o;
    textView = [[NSTextView alloc] initWithFrame:
      NSMakeRect(0, 0, frame.size.width, frame.size.height)];
    [textView setVerticallyResizable: YES];
    [textView setHorizontallyResizable: YES];

    [self setBorderType: NSBezelBorder];
    [self setHasVerticalScroller: YES];
    [self setHasHorizontalScroller: NO];
    [self setDocumentView: textView];
    [textView setDelegate: (id<NSTextViewDelegate>)self];
  }
  return self;
}

-(void)setEnabled: (BOOL)enableIt
{
	//printf("setState called %d\n", enableIt);
  [textView setSelectable: enableIt];
  [textView setEditable: enableIt];
  if (enableIt)
    [textView setTextColor: [NSColor controlTextColor]];
  else
    [textView setTextColor: [NSColor disabledControlTextColor]];
}

-(NSTextStorage *)textStorage
{
  return [textView textStorage];
}
-(void)textDidChange: (NSNotification *)n
{
  shoes_control_send(object, s_change);
}
@end


// ---- Ruby calls C/obj->C here ----

// text_edit_box is new in 3.2.25
void
shoes_native_text_view_set_text(SHOES_CONTROL_REF ref, char *msg)
{
  COCOA_DO([[[(ShoesTextEditView *)ref textStorage] mutableString] setString: [NSString stringWithUTF8String: msg]]);
}

SHOES_CONTROL_REF
shoes_native_text_view(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  INIT;
  ShoesTextEditView *tv = [[ShoesTextEditView alloc] initWithFrame:
    NSMakeRect(place->ix + place->dx, place->iy + place->dy,
    place->ix + place->dx + place->iw, place->iy + place->dy + place->ih)
    andObject: self];
  //shoes_native_text_view_set_text((NSControl *)tv, msg);
  shoes_native_text_view_set_text((SHOES_CONTROL_REF)tv, msg);
  RELEASE;
  return (NSControl *)tv;
}

VALUE
shoes_native_text_view_get_text(SHOES_CONTROL_REF ref)
{
  VALUE text = Qnil;
  INIT;
  text = rb_str_new2([[[(ShoesTextView *)ref textStorage] string] UTF8String]);
  RELEASE;
  return text;
}


VALUE
shoes_native_text_view_append(SHOES_CONTROL_REF ref, char *msg)
{
  COCOA_DO([[[(ShoesTextEditView *)ref textStorage] mutableString] appendString: [NSString stringWithUTF8String: msg]]);
#ifdef dontwant
  // do not like
  NSAttributedString *atext;
  NSTextStorage *buffer;
  INIT;
  NSString *utext = [[NSString alloc] initWithCString: msg encoding: NSUTF8StringEncoding];
  atext = [[NSAttributedString alloc] initWithString: utext];
  buffer = [[(ShoesTextEditView *)ref textStorage] mutableString];
  //  [[self textStorage] appendAttributedString: attrStr];
  [buffer appendAttributedString: atext];
  //[self scrollRangeToVisible:NSMakeRange([[self string] length], 0)];
  RELEASE;
#endif
  return Qnil;
}
