#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/textview.h"
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

@implementation ShoesTextView
- (id)initWithFrame: (NSRect)frame andObject: (VALUE)o
{
  if ((self = [super initWithFrame: frame]))
  {
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

// cjc - bug230 2014-07-26 - just had to learn that cocoa setEnabled is
// called for Shoes widget.state = or widget :state = "disabled"

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
-(NSTextView *) textView
{
  return textView;
}
-(void)textDidChange: (NSNotification *)n
{
  shoes_control_send(object, s_change);
}
@end

// ---- Ruby calls C/obj->C here ----

void
shoes_native_edit_box_set_text(SHOES_CONTROL_REF ref, char *msg)
{
  COCOA_DO([[[(ShoesTextView *)ref textStorage] mutableString] setString: [NSString stringWithUTF8String: msg]]);
}

SHOES_CONTROL_REF
shoes_native_edit_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  INIT;
  ShoesTextView *tv = [[ShoesTextView alloc] initWithFrame:
    NSMakeRect(place->ix + place->dx, place->iy + place->dy,
    place->ix + place->dx + place->iw, place->iy + place->dy + place->ih)
    andObject: self];
  //shoes_native_edit_box_set_text((NSControl *)tv, msg);
  shoes_native_edit_box_set_text((SHOES_CONTROL_REF )tv, msg);
  RELEASE;
  return (NSControl *)tv;
}

VALUE
shoes_native_edit_box_get_text(SHOES_CONTROL_REF ref)
{
  VALUE text = Qnil;
  INIT;
  text = rb_str_new2([[[(ShoesTextView *)ref textStorage] string] UTF8String]);
  RELEASE;
  return text;
}


void
shoes_native_edit_box_append(SHOES_CONTROL_REF ref, char *msg)
{
  COCOA_DO([[[(ShoesTextView *)ref textStorage] mutableString] appendString: [NSString stringWithUTF8String: msg]]);
}

void
shoes_native_edit_box_scroll_to_end(SHOES_CONTROL_REF ref)
{

  NSTextView *tv = [(ShoesTextView *) ref textView];
  [tv scrollRangeToVisible: NSMakeRange(tv.string.length, 0)];
}
