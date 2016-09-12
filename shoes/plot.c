/*
 * plot - draws graphs from annotated arrays
*/
#include "shoes/app.h"
#include "shoes/canvas.h"
#include "shoes/ruby.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/version.h"
#include "shoes/http.h"
#include "shoes/effects.h"
#include <math.h>

/* -------- plot_series object - not a widget -----
*/

/*  ------- Plot widget -----
 *  several methods are defined in ruby.c Macros (CLASS_COMMON2, TRANS_COMMON)
 */

// forward declares in this file:
//static int shoes_plot_draw_surface(cairo_t *, shoes_plot *, shoes_place *, int, int); // for svg save?
static void shoes_plot_draw_title(cairo_t *, shoes_plot *);
static void shoes_plot_draw_caption(cairo_t *,shoes_plot *);
static void shoes_plot_draw_fill(cairo_t *, shoes_plot *);
static void shoes_plot_draw_adornments(cairo_t *, shoes_plot *);
static void shoes_plot_draw_datapts(cairo_t *, shoes_plot *);
static void shoes_plot_draw_ticks_and_labels(cairo_t *, shoes_plot *);
static void shoes_plot_draw_legend(cairo_t *, shoes_plot *);
static void shoes_plot_draw_tick(cairo_t *, shoes_plot *, int, int, int);
static void shoes_plot_draw_label(cairo_t *, shoes_plot *, int, int , char*, int);
static void shoes_plot_draw_everything(cairo_t *, shoes_place *, shoes_plot *);
static void shoes_plot_draw_nub(cairo_t *, int, int);

static float plot_colors[6][3] = {
  { 0.0, 0.0, 0.9 }, // 0 is blue
  { 0.9, 0.0, 0.0 }, // 1 is red
  { 0.0, 0.9, 0.0 }, // 2 is green
  { 0.9, 0.9, 0.9 }, // 3 is yellow
  { 0.9, 0.5, 0.0 }, // 4 is orange-ish
  { 0.5, 0.0, 0.9 }  // 5 is purple
} ;

// ugly defines used only in this file?  Could use fancy new C enum? Or Not.
#define VERTICALLY 0
#define HORIZONTALLY 1
#define LEFT 0
#define BELOW 1
#define RIGHT 2
#define MISSING_SKIP 0
#define MISSING_MIN 1
#define MISSING_MAX 2


// alloc some memory for a shoes_plot; We'll protect it's Ruby VALUES from gc
// out of caution. fingers crossed.
void
shoes_plot_mark(shoes_plot *plot)
{
  rb_gc_mark_maybe(plot->parent);
  rb_gc_mark_maybe(plot->attr);
  rb_gc_mark_maybe(plot->values);
  rb_gc_mark_maybe(plot->xobs);
  rb_gc_mark_maybe(plot->minvs);
  rb_gc_mark_maybe(plot->maxvs);
  rb_gc_mark_maybe(plot->names);
  rb_gc_mark_maybe(plot->sizes);
  rb_gc_mark_maybe(plot->long_names);
  rb_gc_mark_maybe(plot->strokes);
  rb_gc_mark_maybe(plot->nubs);
  rb_gc_mark_maybe(plot->title);
  rb_gc_mark_maybe(plot->caption);
  rb_gc_mark_maybe(plot->legend);
}

static void
shoes_plot_free(shoes_plot *plot)
{
  shoes_transform_release(plot->st);
  RUBY_CRITICAL(SHOE_FREE(plot));
}

VALUE
shoes_plot_alloc(VALUE klass)
{
  VALUE obj;
  shoes_plot *plot = SHOE_ALLOC(shoes_plot);
  SHOE_MEMZERO(plot, shoes_plot, 1);
  obj = Data_Wrap_Struct(klass, shoes_plot_mark, shoes_plot_free, plot);
  plot->values = rb_ary_new();
  plot->xobs  = rb_ary_new();
  plot->minvs = rb_ary_new();
  plot->maxvs = rb_ary_new();
  plot->names = rb_ary_new();
  plot->sizes = rb_ary_new();
  plot->long_names = rb_ary_new();
  plot->strokes = rb_ary_new();
  plot->nubs = rb_ary_new();
  plot->parent = Qnil;
  plot->st = NULL;
  plot->auto_grid = 0;
  plot->x_ticks = 8;
  plot->y_ticks = 6;
  plot->missing = MISSING_SKIP;
  return obj;
}

VALUE
shoes_plot_new(int argc, VALUE *argv, VALUE parent)
{
  VALUE attr = Qnil, widthObj = Qnil, heightObj = Qnil, optsArg = Qnil;
  VALUE title = Qnil, caption = Qnil, fontreq = Qnil, auto_grid = Qnil;
  VALUE x_ticks = Qnil, y_ticks = Qnil;
  VALUE missing = Qnil;
  shoes_canvas *canvas;
  Data_Get_Struct(parent, shoes_canvas, canvas);
  
  rb_arg_list args;
  switch (rb_parse_args(argc, argv, "iih", &args))
  {
    case 1: 
     widthObj  = args.a[0];
     heightObj = args.a[1];
     attr = args.a[2];
    break;
  }
  if (!NIL_P(attr)) {
    title = shoes_hash_get(attr, rb_intern("title"));
    caption = shoes_hash_get(attr, rb_intern("caption"));
    fontreq = shoes_hash_get(attr, rb_intern("font"));
    auto_grid = shoes_hash_get(attr, rb_intern("auto_grid"));
    x_ticks = shoes_hash_get(attr, rb_intern("x_ticks"));
    y_ticks = shoes_hash_get(attr, rb_intern("y_ticks"));
    missing = shoes_hash_get(attr, rb_intern("missing"));
    // there are many other things in that hash
  } else {
    rb_raise(rb_eArgError, "Plot: missing mandatory {options}");
  }

  VALUE obj = shoes_plot_alloc(cPlot);
  shoes_plot *self_t;
  Data_Get_Struct(obj, shoes_plot, self_t);
  
  self_t->place.w = NUM2INT(widthObj);
  self_t->place.h = NUM2INT(heightObj);
  /* 
   * TODO: pangocairo fontmetrics for title and caption 
   * and many more - width of Y axis label space. Challenging.
   * at this time, we have't been placed on screen so computing x and y
   * is kind of tricky as these are relative to where ever that happens
   * to be. 
  */
  if (! NIL_P(fontreq)) {
    self_t->fontname = RSTRING_PTR(fontreq);
  } else {
    self_t->fontname = "Helvitica";
  }
  
  if (!NIL_P(title)) {
    self_t->title = title;
  } else {
    self_t->title = rb_str_new2("Missing a title:");
  }
  // setup pangocairo for the title
  self_t->title_pfd = pango_font_description_new ();
  pango_font_description_set_family (self_t->title_pfd, self_t->fontname);
  pango_font_description_set_weight (self_t->title_pfd, PANGO_WEIGHT_BOLD);
  pango_font_description_set_absolute_size (self_t->title_pfd, 16 * PANGO_SCALE);

     
  self_t->auto_grid = 0;
  if (! NIL_P(auto_grid)) {
    if (RTEST(auto_grid))
      self_t->auto_grid = 1;
  } 
  // todo :sym would be more ruby like than strings.
  if ((!NIL_P(missing)) && (TYPE(missing) == T_STRING)) {
    char *mstr = RSTRING_PTR(missing);
    if (strcmp(mstr, "min") == 0)
      self_t->missing = MISSING_MIN;
    else if (strcmp(mstr, "max") == 0)
      self_t->missing = MISSING_MAX;
    else 
      self_t->missing = MISSING_SKIP;
  } 

  self_t->title_h = 50;

  if (!NIL_P(caption)) {
    self_t->caption = caption;
  } else {
    self_t->caption = rb_str_new2("Missing a caption:");
  }
  self_t->legend_h = 25;
  self_t->caption_h = 25;

  // width of y axis on left and right of plot, in pixels
  // really should be computed based on the data being presented.
  // TODO Of course.
  self_t->yaxis_offset = 50; 
  
  if (!NIL_P(x_ticks))
    self_t->x_ticks = NUM2INT(x_ticks);
  if (!NIL_P(y_ticks))
    self_t->y_ticks = NUM2INT(y_ticks);
    
  self_t->parent = parent;
  self_t->attr = attr;
  
  // initialize cairo matrice used in transform methods (rotate, scale, skew, translate)
  self_t->st = shoes_transform_touch(canvas->st);
  
  return obj;
}

// This gets called very often by Shoes. May be slow for large plots?
VALUE shoes_plot_draw(VALUE self, VALUE c, VALUE actual)
{
  shoes_plot *self_t; 
  shoes_place place; 
  shoes_canvas *canvas; 
  Data_Get_Struct(self, shoes_plot, self_t); 
  Data_Get_Struct(c, shoes_canvas, canvas); 
  if (ATTR(self_t->attr, hidden) == Qtrue) return self; 
  int rel =(REL_CANVAS | REL_SCALE);
  shoes_place_decide(&place, c, self_t->attr, self_t->place.w, self_t->place.h, rel, REL_COORDS(rel) == REL_CANVAS);
  
  if (RTEST(actual)) {
    shoes_plot_draw_everything(CCR(canvas), &place, self_t);
    //self_t->place = place;
  } 
  
  if (!ABSY(place)) { 
    canvas->cx += place.w; 
    canvas->cy = place.y; 
    canvas->endx = canvas->cx; 
    canvas->endy = max(canvas->endy, place.y + place.h); 
  } 
  if(rb_obj_class(c) == cStack) { 
    canvas->cx = CPX(canvas); 
    canvas->cy = canvas->endy; 
  }
  return self;
}

// this is called by both shoes_plot_draw (general Shoes refresh events)
// and by shoes_plot_save_as
static void shoes_plot_draw_everything(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
    
    shoes_apply_transformation(cr, self_t->st, place, 0);  // cairo_save(cr) is inside
    cairo_translate(cr, place->ix + place->dx, place->iy + place->dy);
    
    // draw widget box and fill with color (nearly white). 
    shoes_plot_draw_fill(cr, self_t);
    // draw title TODO - should use pangocairo/fontmetrics
    cairo_select_font_face(cr, self_t->fontname, CAIRO_FONT_SLANT_NORMAL,
      CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    shoes_plot_draw_title(cr, self_t);
  
    // draw caption TODO: should use pangocairo/fontmetrics
    cairo_select_font_face(cr, self_t->fontname, CAIRO_FONT_SLANT_NORMAL,
      CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);  
    shoes_plot_draw_caption(cr, self_t);
    
    self_t->graph_h = self_t->place.h - (self_t->title_h + self_t->caption_h);
    self_t->graph_y = self_t->title_h + 3;
    self_t->yaxis_offset = 50; // TODO:  run TOTO, run!
    self_t->graph_w = self_t->place.w - self_t->yaxis_offset;
    self_t->graph_x = self_t->yaxis_offset;
    if (self_t->seriescnt) {
      // draw  box, ticks and x,y labels.
      shoes_plot_draw_adornments(cr, self_t);
    
      // draw data
      shoes_plot_draw_datapts(cr, self_t);
    }
    // drawing finished
    shoes_undo_transformation(cr, self_t->st, place, 0); // doing cairo_restore(cr)
    self_t->place = *place;
}

static void shoes_plot_draw_fill(cairo_t *cr, shoes_plot *plot)
{
  cairo_set_source_rgb(cr, 0.99, 0.99, 0.99);
  cairo_set_line_width(cr, 1);
  cairo_rectangle(cr, 0, 0, plot->place.w, plot->place.h);
  cairo_stroke_preserve(cr);
  cairo_fill(cr);
  cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
}

static void shoes_plot_draw_adornments(cairo_t *cr, shoes_plot *plot)
{
  // draw box around data area (plot->graph_?)
  cairo_set_line_width(cr, 1);
  int t,l,b,r;
  l = plot->graph_x; t = plot->graph_y;
  r = plot->graph_w; b = plot->graph_h;
  cairo_move_to(cr, l, t);
  cairo_line_to(cr, r, t);  // across top
  cairo_line_to(cr, r, b);  // down right side
  cairo_line_to(cr, l, b);  // across bottom
  cairo_line_to(cr, l, t);  // up left
  cairo_stroke(cr);
  shoes_plot_draw_ticks_and_labels(cr, plot);
  shoes_plot_draw_legend(cr, plot);
}

static void shoes_plot_draw_ticks_and_labels(cairo_t *cr, shoes_plot *plot)
{
  int top, left, bottom, right; // these are cairo abs for plot->graph
  int width, height;   // full plot space so it includes everything
  int range;
  int h_padding = 65; // default width of horizontal tick cell TODO: an option in plot-> 
  int v_padding = 25; // default height of tick TODO: an option in plot->
  left = plot->graph_x; top = plot->graph_y;
  right = plot->graph_w; bottom = plot->graph_h; 
  range = plot->end_idx - plot->beg_idx;
  width = right - left; 
  height = bottom - top;
  h_padding = width / plot->x_ticks;
  v_padding = height / plot->y_ticks;
 
  double h_scale; 
  int h_interval; 
  h_scale = width / (double) (range -1);
  h_interval = (int) ceil(h_padding / h_scale);
 
  // draw x axis - labels and tick mark uses plot->xobs - assumes it's string
  // in the array -- TODO: allow a proc to be called to create the string. at 'i'
  int i;
  VALUE xobs = rb_ary_entry(plot->xobs, 0); // first series is x axis descripter
  if (NIL_P(xobs) || TYPE(xobs) != T_ARRAY) rb_raise (rb_eArgError, "xobs must be an array");
 
  for (i = 0 ; i < range; i++ ) {
    int x = (int) roundl(i * h_scale);
    x += left;
    long y = bottom;
    if ((i % h_interval) == 0) {
      char *rawstr;
      VALUE rbstr = rb_ary_entry(xobs, i + plot->beg_idx);
      if (NIL_P(rbstr)) {
        rawstr = " ";
      } else {
        rawstr = RSTRING_PTR(rbstr);
      }
      //printf("x label i: %i, x: %i, y: %i, \"%s\" %i %f \n", i, (int) x, (int) y, rawstr, h_interval, h_scale);
      shoes_plot_draw_tick(cr, plot, x, y, VERTICALLY);
      shoes_plot_draw_label(cr, plot, x, y, rawstr, BELOW);
    }
  }
  int j;
  for (j = 0; j < min(2, plot->seriescnt); j++) {
    VALUE rbmax = rb_ary_entry(plot->maxvs, j);
    double maximum = NUM2DBL(rbmax);
    VALUE rbmin = rb_ary_entry(plot->minvs, j);
    double minimum = NUM2DBL(rbmin);
    //double v_scale = plot->graph_h / (maximum - minimum);
    double v_scale = height / (maximum - minimum);
    int v_interval = (int) ceil(v_padding / v_scale);
    VALUE rbser = rb_ary_entry(plot->values, j);
    char tstr[16];
    long i;
    for (i = ((long) minimum) + 1 ; i < ((long) roundl(maximum)); i = i + roundl(v_interval)) {
      int y = (int) (bottom - roundl((i - minimum) * v_scale));
      int x = 0;
      sprintf(tstr, "%i",  (int)i); // TODO user specificed format? 
      if (j == 0) { // left side y presentation 
        x = left;
        //printf("hoz left %i, %i, %s\n", (int)x, (int)y,tstr);
        shoes_plot_draw_tick(cr, plot, x, y, HORIZONTALLY);
        shoes_plot_draw_label(cr, plot, x, y, tstr, LEFT);
      } else {        // right side y presentation
        x = right;
        shoes_plot_draw_tick(cr, plot, x, y, HORIZONTALLY);
        shoes_plot_draw_label(cr, plot, x, y, tstr, RIGHT); 
      }
    }
  }
}
static void shoes_plot_draw_legend(cairo_t *cr, shoes_plot *plot)
{
  // kind of tricksy using the cairo toy api's.
  // compute width of all name plus some space between them
  // compute left point, move_to there. for each name
  // pick and set the color, draw string and a space or two
  // repeat for next series
  
  int top, left, bottom, right; 
  int width, height;   
  left = plot->place.x; top = plot->graph_h + 5;
  right = plot->place.w; bottom = top + plot->legend_h; 
  width = right - left; 
  height = bottom - top;
  
  int i, bigstrlen = 0;
  VALUE rbstr; 
  char *strary[6];
  for (i = 0; i <  6; i++) strary[i] = 0;
  for (i = 0; i < plot->seriescnt; i++) {
    rbstr = rb_ary_entry(plot->long_names, i);
    strary[i] = RSTRING_PTR(rbstr);
    if (i > 0) 
      bigstrlen += 2; // TODO number of space
    bigstrlen += strlen(strary[i]);
  }
  char *space_str = "  ";
  char *bigstr = malloc(bigstrlen);
  bigstr[0] = '\0';
  for (i = 0; i < plot->seriescnt; i++) {
    if (i > 0) 
      strcat(bigstr, space_str);
    strcat(bigstr, strary[i]);
  }
  cairo_set_font_size(cr, 14);
  cairo_text_extents_t bigstr_ct;
  cairo_text_extents(cr, bigstr, &bigstr_ct);

  free(bigstr);
  // where to position the drawing pt?
  int pos_x;
  int pct;
  pct = (width / 2) - (bigstr_ct.width / 2);
  pos_x = plot->place.ix + pct;
  //printf("middle? w: %i, l: %i  pos_x: %i, strw: %i\n", width, left, pos_x, (int)bigstr_ct.width);
  cairo_move_to(cr, pos_x, bottom+5); //TODO: compute baseline
  for (i = 0; i < plot->seriescnt; i++) {
     cairo_set_source_rgb(cr, plot_colors[i][0], plot_colors[i][1],
         plot_colors[i][2]);
     cairo_show_text(cr, strary[i]);
     cairo_show_text(cr, space_str);
  }
  
}

static void shoes_plot_draw_tick(cairo_t *cr, shoes_plot *plot,
    int x, int y, int orientation) 
{
  if (plot->auto_grid == 0) return;
  int tick_size = 3;
  if (orientation == VERTICALLY) {
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x, plot->graph_y);
  } else if (orientation == HORIZONTALLY) {
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, plot->graph_w, y);
  } else {
    printf("FAIL: shoes_plot_draw_tick  orientation\n");
  }
  cairo_stroke(cr);
}

static void shoes_plot_draw_label(cairo_t *cr, shoes_plot *plot,
    int x, int y, char *str, int where)
{
  // TODO: Font was previously set to Helvetica 12 and color was setup
  // keep them for now
  cairo_text_extents_t ct;
  cairo_text_extents(cr, str, &ct);
  int str_w = (int) ct.width;
  // measure the max height of the font not the string.
  cairo_font_extents_t ft;
  cairo_font_extents(cr, &ft);
  int str_h = (int) ceil(ft.height);
  int newx;
  int newy;
  if (where == LEFT) { // left side y-axis
    newx = x - (str_w + 3) - 1 ;
    newy = y + (str_h -(str_h / 2));
  } else if (where == RIGHT) { // right side y-axis
    newx = x;
    newy = y + (str_h -(str_h / 2));
    //printf("lbl rightx: %i, y: %i, %s\n", (int)newx, (int)newy, str);
  } else if (where == BELOW) { // bottom side x axis
    newx = x - (str_w / 2);
    newy = y + str_h + 3;
  } else { 
    printf("FAIL: shoes_plot_draw_label 'where ?'\n");
  }
  cairo_move_to(cr, newx, newy);
  cairo_show_text(cr, str);
  // printf("TODO: shoes_plot_draw_label called\n");
}

static void shoes_plot_draw_datapts(cairo_t *cr, shoes_plot *plot)
{
  int i, num_series;
  int top,left,bottom,right;
  left = plot->graph_x; top = plot->graph_y;
  right = plot->graph_w; bottom = plot->graph_h;    
  for (i = 0; i < plot->seriescnt; i++) {
    int oldx = 0;
    int oldy = plot->graph_h; // Needed?
    VALUE rbvalues = rb_ary_entry(plot->values, i);
    VALUE rbmaxv = rb_ary_entry(plot->maxvs, i);
    VALUE rbminv = rb_ary_entry(plot->minvs, i);
    VALUE rbsize = rb_ary_entry(plot->sizes, i);
    VALUE rbstroke = rb_ary_entry(plot->strokes, i);
    VALUE rbnubs = rb_ary_entry(plot->nubs, i);
    double maximum = NUM2DBL(rbmaxv);
    double minimum = NUM2DBL(rbminv);
    int strokew = NUM2INT(rbstroke);
    if (strokew < 1) strokew = 1;
    cairo_set_line_width(cr, strokew);
    // Shoes: Remember - we use ints for x, y, w, h and for drawing lines and points
    int height = bottom - top;
    int width = right - left; 
    int range = plot->end_idx - plot->beg_idx; // zooming adj
    float vScale = height / (maximum - minimum);
    float hScale = width / (double) (range - 1);
    int nubs = (width / range > 10) ? RTEST(rbnubs) : 0;  // could be done if asked
  
    // Note colors can be part of the series description and modify
    // the plot_colors tables; That is not done here.
    cairo_set_source_rgb(cr, plot_colors[i][0], plot_colors[i][1],
        plot_colors[i][2]);

    int j;
    int brk = 0; // for missing value control
    for (j = 0; j < range; j++) {
      VALUE rbdp = rb_ary_entry(rbvalues, j + plot->beg_idx);
      if (NIL_P(rbdp)) {
        if (plot->missing == MISSING_MIN) {
          rbdp = rbminv;
        } else if (plot->missing == MISSING_MAX) {
          rbdp = rbmaxv;
        } else {
          brk = 1;
          continue;
        }
      }
      double v = NUM2DBL(rbdp);
      long x = roundl(j * hScale);
      long y = height - roundl((v - minimum) *vScale);
      x += left;
      y += top;
      //printf("draw i: %i, x: %i, y: %i %f \n", j, (int) x, (int) y, hScale);
      if (j == 0 || brk == 1) {
        cairo_move_to(cr, x, y);
        brk = 0;
      } else {
        cairo_line_to(cr, x, y);
      }
      if (nubs) 
        shoes_plot_draw_nub(cr, x, y);
    }
    cairo_stroke(cr);
    cairo_set_line_width(cr, 1.0); // reset between series
  } // end of drawing one series
  // tell cairo to draw all lines (and points)
  cairo_stroke(cr); 
  // set color back to dark gray and stroke to 1
  cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
  cairo_set_line_width(cr, 1.0);
}

static void shoes_plot_draw_nub(cairo_t *cr, int x, int y)
{
    int sz = 2; 
    /* java
    Color save = g.getColor();

		g.setColor(Color.black);
		g.drawLine(x - 1 , y - 1, x + 1, y - 1);
		g.drawLine(x - 1 , y - 1, x - 1, y + 1);

		g.drawLine(x + 1 , y + 1, x + 1, y - 1);
		g.drawLine(x + 1 , y + 1, x - 1, y + 1);
		g.setColor(save);
    */
    // Shoes a draw a very small block at x,y
    cairo_move_to(cr, x - sz, y - sz);
    cairo_line_to(cr, x + sz, y - sz);
    
    cairo_move_to(cr, x - sz, y - sz);
    cairo_line_to(cr, x - sz, y + sz);
    
    cairo_move_to(cr, x + sz, y + sz);
    cairo_line_to(cr, x + sz, y - sz);
    
    cairo_move_to(cr, x + sz, y + sz);
    cairo_line_to(cr, x - sz, y + sz);
    
    cairo_move_to(cr, x, y); // back to center point.
}

static void shoes_plot_draw_title(cairo_t *cr, shoes_plot *plot) 
{
  char *str = RSTRING_PTR(plot->title);
  int x, y;
#ifdef TOY_CAIRO
  cairo_text_extents_t te;
  cairo_text_extents(cr, str, &te);
  int xoffset = (plot->place.w / 2) - (te.width / 2);
  int yhalf = (plot->title_h / 2 ); 
  int yoffset = yhalf + (te.height / 2);
  //x = xoffset - (plot->place.ix + plot->place.dx);
  x = xoffset - (plot->place.dx);
  //y = plot->title_h;
  y = yoffset;
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, str);
#else
  PangoLayout *layout = pango_cairo_create_layout (cr);
  pango_layout_set_font_description (layout, plot->title_pfd);
  pango_layout_set_text (layout, str, -1);
  PangoRectangle ink, logical;
  pango_layout_get_pixel_extents (layout, &ink, &logical);
  int xoffset = (plot->place.w / 2) - (logical.width / 2);
  x = xoffset - (plot->place.dx);
  int yhalf = (plot->title_h / 2 ); 
  int yoffset = yhalf; 
  y = yoffset;
  cairo_move_to(cr, x, y);
  pango_cairo_show_layout (cr, layout);
#endif 
}

static void shoes_plot_draw_caption(cairo_t *cr, shoes_plot *plot)
{
  char *str = RSTRING_PTR(plot->caption);
  int x, y;
  cairo_text_extents_t te;
  cairo_text_extents(cr, str, &te);
  int xoffset = (plot->place.w / 2) - (te.width / 2);
  int yhalf = (plot->caption_h / 2 ); 
  int yoffset = yhalf + (te.height / 2);
  //int offset = (plot->place.w / 2) - (strlen(t) * 6);
  //x = plot->place.ix + xoffset;
  x = xoffset - (plot->place.dx);
  //y = plot->place.h - plot->caption_h; 
  y = plot->place.h; 
  y -= yoffset;
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, str);
}

VALUE shoes_plot_add(VALUE self, VALUE newseries) 
{
  shoes_plot *self_t;
  VALUE rbsz, rbvals, rbobs, rbmin, rbmax, rbshname, rblgname, rbcolor;
  VALUE rbstroke, rbnubs;
  Data_Get_Struct(self, shoes_plot, self_t); 
  int i = self_t->seriescnt; // track number of series to plot.
  if (i >= 6) {
    rb_raise(rb_eArgError, "Maximum of 6 series");
  }
  if (TYPE(newseries) == T_HASH) {

    rbsz = shoes_hash_get(newseries, rb_intern("num_obs"));
    rbvals = shoes_hash_get(newseries, rb_intern("values"));
    rbobs = shoes_hash_get(newseries, rb_intern("xobs"));
    rbmin = shoes_hash_get(newseries, rb_intern("minv"));
    rbmax = shoes_hash_get(newseries, rb_intern("maxv"));
    rbshname = shoes_hash_get(newseries, rb_intern("name"));
    rblgname = shoes_hash_get(newseries, rb_intern("long_name"));
    rbcolor  = shoes_hash_get(newseries, rb_intern("color"));
    rbstroke = shoes_hash_get(newseries, rb_intern("strokewidth"));
    rbnubs = shoes_hash_get(newseries, rb_intern("nubs"));
    
    if ( NIL_P(rbvals) || TYPE(rbvals) != T_ARRAY ) {
      rb_raise(rb_eArgError, "plot.add: Missing an Array of values");
    }
    if (NIL_P(rbmin) || NIL_P(rbmax)) {
      rb_raise(rb_eArgError, "plot.add: Missing minv: or maxv: option");
    }
    if ( NIL_P(rbobs) ) {
      // we can fake it - poorly - TODO better. Please.
      int l = NUM2INT(rbsz);
      int i;
      rbobs = rb_ary_new2(l);
      for (i = 0; i < l; i++) {
        char t[8];
        sprintf(t, "%i", i+1);
        VALUE foostr = rb_str_new2(t);
        rb_ary_store(rbobs, i, foostr);
      }
    }
    if ( TYPE(rbobs) != T_ARRAY ) {
      rb_raise(rb_eArgError, "plot.add xobs is not an array");
    }
    if (NIL_P(rbshname)) 
      rb_raise(rb_eArgError, "plot.add missing name:");
    if (NIL_P(rblgname)) {
      rblgname = rbshname;
    }
    // handle colors - replace 
    if (! NIL_P(rbcolor)) {
      if (TYPE(rbcolor) != T_STRING)
        rb_raise(rb_eArgError, "plot.add color must be a string");
      char *cstr = RSTRING_PTR(rbcolor);
      VALUE cval = shoes_hash_get(cColors, rb_intern(cstr)); // segfault or raise? 
      if (NIL_P(cval))
        rb_raise(rb_eArgError, "plot.add color: not a known color");
      shoes_color *color;
      Data_Get_Struct(cval, shoes_color, color);
      int r,g,b; // 0..255 - need to be cairo 
      r = color->r;
      g = color->g;
      b = color->b;
      plot_colors[self_t->seriescnt][0] = (float) (r / 255.);
      plot_colors[self_t->seriescnt][1] = (float) (g / 255.);
      plot_colors[self_t->seriescnt][2] = (float) (b / 255.);
    }
    
    if (!NIL_P(rbstroke)) {
      if (TYPE(rbstroke) != T_FIXNUM) 
        rb_raise(rb_eArgError, "plot.add strokewidth not an integer\n");
    } else {
      rbstroke = INT2NUM(1); // default
    }
    if (!NIL_P(rbnubs)) {
      rbnubs = Qtrue;    
    } else {
      rbnubs = Qfalse;
    }
    //  For C debugging 
    int l = NUM2INT(rbsz);
    double  min = NUM2DBL(rbmin);
    double  max = NUM2DBL(rbmax);
    char *shname = RSTRING_PTR(rbshname);
    char *lgname = RSTRING_PTR(rblgname);
    //printf("shoes_plot_add using hash: num_obs: %i range %f, %f, |%s|, |%s| \n",
    //   l, min, max, shname, lgname); 
  } else {
    rb_raise(rb_eArgError, "misssing something in plot.add \n");
  }
  rb_ary_store(self_t->sizes, i, rbsz);
  rb_ary_store(self_t->values, i, rbvals);
  rb_ary_store(self_t->xobs, i, rbobs);
  rb_ary_store(self_t->maxvs, i, rbmax);
  rb_ary_store(self_t->minvs, i, rbmin);
  rb_ary_store(self_t->names, i, rbshname);
  rb_ary_store(self_t->long_names, i, rblgname);
  rb_ary_store(self_t->strokes, i, rbstroke);
  rb_ary_store(self_t->nubs, i, rbnubs);
  self_t->beg_idx = 0;
  self_t->end_idx = NUM2INT(rbsz);
  self_t->seriescnt++;
  shoes_canvas_repaint_all(self_t->parent);
  return self;
}

VALUE shoes_plot_delete(VALUE self, VALUE series) 
{ 
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t); 
  if (TYPE(series) != T_FIXNUM) 
    rb_raise(rb_eArgError, "plot.delete arg not integer");
  int idx = NUM2INT(series);
  if (! (idx >= 0 && idx <= self_t->seriescnt))
    rb_raise(rb_eArgError, "plot.delete arg is out of range");
  /* test deletion
  {
    int len = RARRAY_LEN(self_t->strokes);
    VALUE sw = rb_ary_entry(self_t->strokes, 0);
    printf("pre delete array len %i [0]: %i\n", len, (int)NUM2INT(sw));
  }
  */
  rb_ary_delete_at(self_t->sizes, idx);
  rb_ary_delete_at(self_t->values, idx);
  rb_ary_delete_at(self_t->xobs, idx);
  rb_ary_delete_at(self_t->maxvs, idx);
  rb_ary_delete_at(self_t->minvs, idx);
  rb_ary_delete_at(self_t->names, idx);
  rb_ary_delete_at(self_t->long_names, idx);
  rb_ary_delete_at(self_t->strokes, idx); 
  rb_ary_delete_at(self_t->nubs, idx);
  self_t->seriescnt--;
  shoes_canvas_repaint_all(self_t->parent);  
    
  // printf("shoes_plot_delete (%i) called\n", idx);
  return Qtrue;
}

// odds are extremely high that this may flash or crash if called too frequently
VALUE shoes_plot_redraw_to(VALUE self, VALUE to_here) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t); 
  if (TYPE(to_here) != T_FIXNUM) 
    rb_raise(rb_eArgError, "plot.redraw_to arg is not an integer");
  int idx = NUM2INT(to_here);
  self_t->end_idx = idx;
  int i;
  
  for (i = 0; i < self_t->seriescnt; i++) {
    rb_ary_store(self_t->sizes, i, INT2NUM(idx));
  }

  shoes_canvas_repaint_all(self_t->parent);
  //printf("shoes_plot_redraw_to(%i) called\n", idx);
  return Qtrue;
}

// id method
VALUE shoes_plot_find_name(VALUE self, VALUE name) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t); 
  if (TYPE(name) != T_STRING) rb_raise(rb_eArgError, "plot.find arg is not a string");
  char *search = RSTRING_PTR(name);
  int i; 
  for (i =0; i <self_t->seriescnt; i++) {
    VALUE rbstr = rb_ary_entry(self_t->names, i);
    char *entry = RSTRING_PTR(rbstr);
    if (strcmp(search, entry) == 0) {
      return INT2NUM(i);
    }
  }
  return Qnil; // when nothing matches
}

VALUE shoes_plot_zoom(VALUE self, VALUE beg, VALUE end)
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t); 
  if (self_t->seriescnt < 1)
    return Qnil;
  VALUE rbsz = rb_ary_entry(self_t->sizes, 0);
  int maxe = NUM2INT(rbsz);
  int b = NUM2INT(beg);
  int e = NUM2INT(end);
  int nb = max(0, b);
  int ne = min(maxe, e);
  if ((e - b) < 3) {
    //printf("no smaller that 3 points\n");
    return Qfalse;
  }
  //printf("zoom to %i -- %i\n", nb, ne);
  self_t->beg_idx = nb;
  self_t->end_idx = ne;
  shoes_canvas_repaint_all(self_t->parent);
  return Qtrue;
}

VALUE shoes_plot_get_count(VALUE self) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t); 
  return INT2NUM(self_t->seriescnt);
}

VALUE shoes_plot_get_first(VALUE self) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t); 
  return INT2NUM(self_t->beg_idx);
}

VALUE shoes_plot_set_first(VALUE self, VALUE idx) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t); 
  if (TYPE(idx) != T_FIXNUM) rb_raise(rb_eArgError, "plot.set_first arg is not an integer"); 
  self_t->beg_idx = NUM2INT(idx);
  shoes_canvas_repaint_all(self_t->parent); 
  return idx;
}

VALUE shoes_plot_get_last(VALUE self) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t); 
  return INT2NUM(self_t->end_idx);
}
VALUE shoes_plot_set_last(VALUE self, VALUE idx) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t); 
  if (TYPE(idx) != T_FIXNUM) rb_raise(rb_eArgError, "plot.set_last arg is not an integer"); 
  self_t->end_idx = NUM2INT(idx);
  shoes_canvas_repaint_all(self_t->parent); 
  return idx;
}

// ------ widget methods for style and save/export ------
VALUE shoes_plot_get_actual_width(VALUE self)
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t);
  return INT2NUM(self_t->place.w);
}

VALUE shoes_plot_get_actual_height(VALUE self)
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t); 
  return INT2NUM(self_t->place.h);
}

VALUE
shoes_plot_get_actual_left(VALUE self)
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t);
  return INT2NUM(self_t->place.ix + self_t->place.dx);
}

VALUE
shoes_plot_get_actual_top(VALUE self)
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t);
  return INT2NUM(self_t->place.iy + self_t->place.dy);
}

// --- fun with vector and png ---
typedef cairo_public cairo_surface_t * (cairo_surface_function_t) (const char *filename, double width, double height);

static cairo_surface_function_t *get_vector_surface(char *format)
{
  if (strcmp(format, "pdf") == 0) return & cairo_pdf_surface_create;
  if (strcmp(format, "ps") == 0)  return & cairo_ps_surface_create;
  if (strcmp(format, "svg") == 0) return & cairo_svg_surface_create;
  return NULL;
}

static cairo_surface_t* 
build_surface(VALUE self, double scale, int *result, char *filename, char *format) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t);
  shoes_canvas *canvas;
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  shoes_place place = self_t->place;
  cairo_surface_t *surf;
  cairo_t *cr;

  int w = (int)(NUM2INT(shoes_plot_get_actual_width(self))*scale);
  int h = (int)(NUM2INT(shoes_plot_get_actual_height(self))*scale);
  if (format != NULL)
    surf = get_vector_surface(format)(filename, w, h);
  else
    surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  cr = cairo_create(surf);
    
  if (scale != 1.0) cairo_scale(cr, scale, scale);
  cairo_translate(cr, -(place.ix + place.dx), -(place.iy + place.dy));
  
  shoes_plot_draw_everything(cr, &self_t->place, self_t);
  if (format != NULL) cairo_show_page(cr);
  cairo_destroy(cr);
  
  return surf;
}
static int shoes_plot_save_png(VALUE self, char *filename)
{
  int result;
  cairo_surface_t *surf = build_surface(self, 1.0, &result, NULL, NULL);
  cairo_status_t r = cairo_surface_write_to_png(surf, filename);
  cairo_surface_destroy(surf);
  
  return r == CAIRO_STATUS_SUCCESS ? Qtrue : Qfalse;
}

static int shoes_plot_save_vector(VALUE self, char *filename, char *format)
{
  double scale = 1.0;
  int result;
  cairo_surface_t *surf = build_surface(self, 1.0, &result, filename, format);
  cairo_surface_destroy(surf);
  
  return 1;
}

VALUE shoes_plot_save_as(int argc, VALUE *argv, VALUE self) 
{
  if (argc == 0) {
    shoes_plot_save_png(self, NULL);
    printf("save to clipboard\n");
  } else if (TYPE(argv[0]) == T_STRING) {
    char *rbstr = RSTRING_PTR(argv[0]);
    char *lastslash = strrchr(rbstr,'/');
    char *basename;
    char *filename;
    char *lastdot;
    char *ext;
    char *bare; 
    if (lastslash) {
      lastslash++;
      basename = malloc(strlen(lastslash)+1);
      strcpy(basename, lastslash);
      lastdot = strrchr(basename, '.');
      if (lastdot == 0) {
        rb_raise(rb_eArgError,"save_as does not have an extension");
      }
      // replace dot with null (EOS)
      *lastdot = '\0';
      ext = lastdot + 1;
    }
    printf("save to: %s %s (long: %s)\n", basename, ext, rbstr);
    int result = 0;
    if (strcmp(ext, "png") == 0) {
      result = shoes_plot_save_png(self, rbstr);
    } else {
      result = shoes_plot_save_vector(self, rbstr, ext);
    }
    free(basename);
    return (result ? Qtrue : Qnil);
  }
}

/*  Not using PLACE_COMMMON Macro in ruby.c, as we do the plot rendering a bit differently
 *  than other widgets [parent, left, top, width, height ruby methods]
 */
VALUE
shoes_plot_get_parent(VALUE self)
{
  GET_STRUCT(plot, self_t);
  return self_t->parent;
}

VALUE shoes_plot_remove(VALUE self)
{
  shoes_plot *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_plot, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  
  rb_ary_delete(canvas->contents, self);    // shoes_basic_remove does it this way
  shoes_canvas_repaint_all(self_t->parent); 
  
  self_t = NULL;
  self = Qnil;
  return Qtrue;
}

// ----  click handling ------

/* 
 * this attempts to compute the values/xobs index nearest the x pixel 
 * from a mouse click.  First get a % of x between width
 * use that to pick between beg_idx, end_idx and return that.
 * 
 */
VALUE shoes_plot_near(VALUE self, VALUE xpos) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t);
  int x = NUM2INT(xpos); 
  int left,right;
  left = self_t->graph_x; 
  right = self_t->graph_w;
  int newx = x - (self_t->place.ix + self_t->place.dx + left);
  int wid = right - left;
  double rpos = newx  / wid; 
  int rng = (self_t->end_idx - self_t->beg_idx);
  int idx = floorl(rpos * rng);
  printf("shoes_plot_near: %i newx: %i rpos: %f here: %i\n", x,newx,rpos,idx);
  return INT2NUM(idx);
}

// define our own inside function so we can offset our own margins
// this controls what cursor is shown - mostly
static int shoes_plot_inside(shoes_plot *self_t, int x, int y)
{
  int inside = 0;
  inside = (self_t->place.iw > 0 &&  self_t->place.ih > 0 && 
   //x >= self_t->place.ix + self_t->place.dx && 
   x >= self_t->place.ix + self_t->place.dx + self_t->graph_x && 
   // x <= self_t->place.ix + self_t->place.dx + self_t->place.iw && 
   x <= self_t->place.ix + self_t->place.dx + self_t->place.iw -self_t->graph_x && 
   //y >= self_t->place.iy + self_t->place.dy && 
   y >= self_t->place.iy + self_t->place.dy + self_t->graph_y && 
   //y <= self_t->place.iy + self_t->place.dy + self_t->place.ih);
   y <= self_t->place.iy + self_t->place.dy + self_t->place.ih - 
     (self_t->caption_h + self_t->legend_h + 25));  //TODO no hardcoding of offsets
   return inside;
}

//called by shoes_plot_send_click and shoes_canvas_send_motion
VALUE
shoes_plot_motion(VALUE self, int x, int y, char *touch)
{
  char h = 0;
  VALUE click;
  GET_STRUCT(plot, self_t);

  click = ATTR(self_t->attr, click);

  //if (IS_INSIDE(self_t, x, y)) {
  if (shoes_plot_inside(self_t, x, y)) {
    if (!NIL_P(click)) {
      shoes_canvas *canvas;
      Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
      shoes_app_cursor(canvas->app, s_link);
    }
    h = 1;
  }
  
  /* Checks if element is hovered, clicked, released, leaved
   * and eventually calls hover and/or leave callbacks
   *   if hovered:  self_t->hover == 1
   *   if leaved:   self_t->hover == 0
   *   if clicked and not yet released:
   *     if hovered + clicked: self_t->hover == 3 
   *     if leaved + clicked:  self_t->hover == 2
   */
  CHECK_HOVER(self_t, h, touch);

  return h ? click : Qnil;
}

// called by shoes_canvas_send_click --> shoes_canvas_send_click2
VALUE
shoes_plot_send_click(VALUE self, int button, int x, int y)
{
  VALUE v = Qnil;

  if (button >  0) {
    GET_STRUCT(plot, self_t);
    v = shoes_plot_motion(self, x, y, NULL);
    if (self_t->hover & HOVER_MOTION)             // ok, cursor is over the element, proceed
      self_t->hover = HOVER_MOTION | HOVER_CLICK; // we have been clicked, but not yet released
  }
  
  // if we found a click callback send it back to shoes_canvas_send_click method
  // where it will be processed
  return v;
}

// called by shoes_canvas_send_release
void
shoes_plot_send_release(VALUE self, int button, int x, int y)
{
  GET_STRUCT(plot, self_t);
  if (button > 0 && (self_t->hover & HOVER_CLICK)) {
    VALUE proc = ATTR(self_t->attr, release);
    self_t->hover ^= HOVER_CLICK; // we have been clicked and released
    if (!NIL_P(proc))
      //shoes_safe_block(self, proc, rb_ary_new3(1, self));
      shoes_safe_block(self, proc, rb_ary_new3(3, INT2NUM(button), INT2NUM(x), INT2NUM(y)));
  }
}

/*
void
shoes_plot_send_release(VALUE self, int button, int x, int y)
{
  GET_STRUCT(plot, self_t);
  if (button > 0 && (self_t->hover & HOVER_CLICK)) {
    VALUE proc = ATTR(self_t->attr, release);
    self_t->hover ^= HOVER_CLICK; // we have been clicked and released
    if (!NIL_P(proc))
      //shoes_safe_block(self, proc, rb_ary_new3(1, self));
      shoes_safe_block(self, proc, rb_ary_new3(3, INT2NUM(button), INT2NUM(x), INT2NUM(y)));
  }
}
*/
