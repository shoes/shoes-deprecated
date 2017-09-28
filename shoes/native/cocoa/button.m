// Shoes Button ssubclasses NSButton.

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/button.h"
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
@implementation ShoesButton
- (id)initWithType: (NSButtonType)t andObject: (VALUE)o
{
  if ((self = [super init]))
  {
    object = o;
    [self setButtonType: t];
    [self setBezelStyle: NSRoundedBezelStyle];
    [self setTarget: self];
    [self setAction: @selector(handleClick:)];
    NSLog(@"button w/o frame called");
 }
  return self;
}
- (id)initWithTypeF: (NSButtonType)t withFrame:(NSRect)rect andObject: (VALUE)o
{
  if ((self = [super initWithFrame: rect]))
  {
    object = o;
    [self setButtonType: t];
    [self setBezelStyle: NSRoundedBezelStyle];
    [self setTarget: self];
    [self setAction: @selector(handleClick:)];
    NSLog(@"button with frame w: %i h: %i\n", (int) NSWidth(rect), (int)NSHeight(rect));
  }
  return self;
}
-(IBAction)handleClick: (id)sender
{
  shoes_button_send_click(object);
}
@end

// ---- Ruby calls C/obj->C here ----
SHOES_CONTROL_REF shoes_native_button(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  INIT;
  char *fntstr = 0;
  VALUE fgclr = Qnil; // Could be hex color string or Shoes color object
  VALUE lblposv = Qnil;
  NSInteger fsize = 0;
  NSArray *fontsettings;
  NSCellImagePosition imgpos = NSImageLeft;
  ShoesButton *button;
  
  NSMutableString *fontname = [[NSMutableString alloc] initWithCapacity: 40];
  NSRect reqsize = NSMakeRect(place->ix + place->dx, place->iy + place->dy,
      place->ix + place->dx + place->iw, place->iy + place->dy + place->ih);
      
      
  // get the Shoes attributes 
  if (!NIL_P(shoes_hash_get(attr, rb_intern("font")))) {
    fntstr = RSTRING_PTR(shoes_hash_get(attr, rb_intern("font")));
    // TODO: need a helper to parse into a FontDescripter and deal with missing parts
    NSString *fstr = [NSString stringWithUTF8String: fntstr];
    fontsettings = [fstr componentsSeparatedByString:@" "]; 
    // in OSX there is font name - may include Bold etc, and size
    int cnt = fontsettings.count;
    fsize = [fontsettings[cnt-1] integerValue];
    if (fsize > 0 && fsize < 24)  {
      //we probably have a size spec - everything before that is fontname
      int i;
      for (i = 0; i < cnt-1; i++) {
       [fontname appendString: fontsettings[i]];
       if (i < cnt-2) {
         [fontname appendString:@" "];
       }
      }
    } else {
      // have to assume they didn't give a point size so 
      [fontname  appendString: fstr];
      fsize = 10;
    }
  }
  if (!NIL_P(shoes_hash_get(attr, rb_intern("stroke")))) {
    fgclr = shoes_hash_get(attr, rb_intern("stroke"));
  }
  
  lblposv = shoes_hash_get(attr, rb_intern("titlepos"));
  // Ponder. Title vs Image 'left' means title on left, the image
  if (lblposv != Qnil) {
    char *lblp = RSTRING_PTR(lblposv);
    if ( !msg || (strcmp(lblp, "none") == 0)) 
      imgpos = NSImageOnly;
    else if (strcmp(lblp, "left") == 0)
      imgpos = NSImageRight;
    else if (strcmp(lblp, "right") == 0)
      imgpos =  NSImageLeft;
    else if (strcmp(lblp, "top") == 0)
      imgpos = NSImageBelow;
    else if (strcmp(lblp, "bottom") == 0)
      imgpos = NSImageAbove;
    else 
       // rb_raise? 
      imgpos = NSImageLeft;
  }
  
  if (fntstr || !NIL_P(fgclr)) {
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] initWithCapacity: 5];
    NSString *title = [NSString stringWithUTF8String: msg];
    if (fntstr) {
      NSFont *font = [NSFont fontWithName: fontname size: fsize];
      if (font == nil) 
        rb_raise(rb_eArgError, "Font \"%s\" not found", fntstr);
      [dict setObject: font forKey: NSFontAttributeName];
      // Center the text of Attributed String in NSButton is more work
      NSMutableParagraphStyle *centredStyle = [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
      [centredStyle setAlignment:NSCenterTextAlignment];
      [dict setObject: centredStyle forKey: NSParagraphStyleAttributeName];
    }
    if (! NIL_P(fgclr)) {
      // convert Shoes color to NSColor
      if (TYPE(fgclr) == T_STRING) 
        fgclr = shoes_color_parse(cColor, fgclr);  // convert string to cColor
      if (rb_obj_is_kind_of(fgclr, cColor)) 
      { 
        shoes_color *color; 
        Data_Get_Struct(fgclr, shoes_color, color); 
        CGFloat rg = (CGFloat)color->r / 255;
        CGFloat gb = (CGFloat)color->g / 255;
        CGFloat bb = (CGFloat)color->b / 255;
        NSColor *clr = [NSColor colorWithCalibratedRed: rg green: gb blue: bb alpha: 1.0];
        // add to dict with setValue x forKey NSForegroundColorAttribute
        [dict setObject: clr forKey: NSForegroundColorAttributeName];
      }
    }
   
    //Connect dict to title
    NSAttributedString *attrString = [[NSAttributedString alloc] initWithString:title
        attributes: dict];  
    button = [[ShoesButton alloc] initWithTypeF: NSMomentaryPushInButton
        withFrame: reqsize andObject: self];
    [button setAttributedTitle : attrString];
  } else {
    // this is the normal Shoes button
    button = [[ShoesButton alloc] initWithType: NSMomentaryPushInButton
        andObject: self];
    [button setTitle: [NSString stringWithUTF8String: msg]];
  }
  // Do we have an icon? 
  if (!NIL_P(shoes_hash_get(attr, rb_intern("icon")))) {
    char *cpath = RSTRING_PTR(shoes_hash_get(attr, rb_intern("icon")));
    NSString *ipath = [NSString stringWithUTF8String: cpath];
    NSImage *icon =  [[NSImage alloc] initWithContentsOfFile: ipath];
    [button setImage: icon];
    // do we have an explictt setting of title vs icon setting?
    if (lblposv != Qnil) {
      [button setImagePosition: imgpos];
    } else {
     [button setImagePosition: NSImageLeft];
    }
  }
  // Tooltip
  VALUE vtip = shoes_hash_get(attr, rb_intern("tooltip"));
  if (! NIL_P(vtip)) {
    char *cstr = RSTRING_PTR(vtip);
    NSString *tip = [NSString stringWithUTF8String: cstr];
    [button setToolTip:tip];
  } 
  RELEASE;
  return (NSControl *)button;
}
