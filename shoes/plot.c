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


/*  ------- Plot widget -----
 *  several methods are defined in ruby.c Macros (CLASS_COMMON2, TRANS_COMMON)
 */

// forward declares in this file:
static int
shoes_plot_draw_surface(cairo_t *, shoes_plot *, shoes_place *, int, int);
static void shoes_plot_draw_title(shoes_canvas *, shoes_plot *);
static void shoes_plot_draw_caption(shoes_canvas *,shoes_plot *);
static void shoes_plot_draw_fill(shoes_canvas *, shoes_plot *);
static void shoes_plot_draw_adornments(shoes_canvas *, shoes_plot *);
static void shoes_plot_draw_datapts(shoes_canvas *, shoes_plot *);
static void shoes_plot_draw_ticks_and_labels(shoes_canvas *, shoes_plot *);
static void shoes_plot_draw_legend_marker(shoes_canvas *, shoes_plot *);
static void shoes_plot_draw_tick(shoes_canvas *, shoes_plot *, int, int, int);
static void shoes_plot_draw_label(shoes_canvas *, shoes_plot *, int, int , char*, int);
// ugly defines only used in this file?  Could use fancy new C enum? Or Not.
#define VERTICALLY 0
#define HORIZONTALLY 1
#define LEFT 0
#define BELOW 1
#define RIGHT 2

// alloc some memory for a shoes_plot; We'll protect it's Ruby VALUES from gc
// out of caution. fingers crossed.
void
shoes_plot_mark(shoes_plot *plot)
{
  rb_gc_mark_maybe(plot->parent);
  rb_gc_mark_maybe(plot->attr);
  rb_gc_mark_maybe(plot->values);
  rb_gc_mark_maybe(plot->minvs);
  rb_gc_mark_maybe(plot->maxvs);
  rb_gc_mark_maybe(plot->names);
  rb_gc_mark_maybe(plot->sizes);
  rb_gc_mark_maybe(plot->long_names);
  rb_gc_mark_maybe(plot->title);
  rb_gc_mark_maybe(plot->caption);
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
  plot->parent = Qnil;
  plot->st = NULL;
  return obj;
}

#if 0
void
plot_aspect_ratio(int imw, int imh, shoes_plot *self_t, shoes_plothandle *plothan)
{
  double outw = imw * 1.0; // width given to plot() 
  double outh = imh * 1.0; // height given to plot() 

  self_t->scalew = outw / plothan->plothdim.width;   // don't keep aspect ratio, Fill provided or deduced dimensions
  self_t->scaleh = outh / plothan->plothdim.height;  // 

  if (plothan->aspect == 0.0) {                    // keep aspect ratio
    self_t->scalew = self_t->scaleh = MIN(outw / plothan->plothdim.width, outh / plothan->plothdim.height);

  } else if (plothan->aspect > 0.0) {              // don't keep aspect ratio, User aspect ratio

    double new_plotdim_height = plothan->plothdim.width / plothan->aspect;
    double new_plotdim_width = plothan->plothdim.height * plothan->aspect;

    if (outw / new_plotdim_width < outh / new_plotdim_height)
      self_t->scaleh = self_t->scalew * new_plotdim_height / plothan->plothdim.height;
    else
      self_t->scalew = self_t->scaleh * new_plotdim_width / plothan->plothdim.width;
  }
}
#endif

VALUE
shoes_plot_new(int argc, VALUE *argv, VALUE parent)
{
  VALUE attr = Qnil, widthObj, heightObj, optsArg;
  VALUE title, caption;
  shoes_canvas *canvas;
  Data_Get_Struct(parent, shoes_canvas, canvas);
  
  rb_arg_list args;
  // TODO parse args
  switch (rb_parse_args(argc, argv, "iih", &args))
  {
    case 1: 
     widthObj  = args.a[0];
     heightObj = args.a[1];
     optsArg = args.a[2];
    break;
  }
  if (!NIL_P(optsArg)) {
    // TODO pick out :title and :caption if given and other style args
    title = shoes_hash_get(optsArg, rb_intern("title"));
    caption = shoes_hash_get(optsArg, rb_intern("caption"));
#if 0
    // C debugging
    if (!NIL_P(title)) printf("have title: %s\n", RSTRING_PTR(title));
    if (!NIL_P(caption)) printf("have caption: %s\n", RSTRING_PTR(caption));
#endif
  } 

  VALUE obj = shoes_plot_alloc(cPlot);
  shoes_plot *self_t;
  Data_Get_Struct(obj, shoes_plot, self_t);
  
  
  self_t->place.w = NUM2INT(widthObj);
  self_t->place.h = NUM2INT(heightObj);
  /* TODO should call something(self_t) to recompute x,y,w,h on resize
   * TODO fontmetrics for title and caption (and arg parsing)
   * and many more - width of Y axis label space. Challenging.
   * at this time, we have't been placed on screen so computing x and y
   * is kind of tricky as these are relative to where ever that happens
   * to be. Conside these are offsets.
  */
  if (!NIL_P(title)) {
    self_t->title = title;
  }
  self_t->title_h = 20;
  self_t->title_x = 0;
  self_t->title_y = 20; 
  self_t->title_w = 0;
  
  if (!NIL_P(caption)) {
    self_t->caption = caption;
  }
  self_t->caption_h = 20;
  self_t->caption_w = 0;
  self_t->caption_x = 0;
  self_t->caption_y = 1;
  // width of y axis on left and right of plot, in pixels
  // really should be computed based on the data being presented.
  // TODO Of course.
  self_t->yaxis_offset = 50; 
  
  self_t->parent = parent;
  self_t->attr = Qnil;
  
  // initialize cairo matrice used in transform methods (rotate, scale, skew, translate)
  self_t->st = shoes_transform_touch(canvas->st);
  
  return obj;
}



// This gets called very often by Shoes. May be slow for large plots?
VALUE shoes_plot_draw(VALUE self, VALUE c, VALUE actual)
{
  shoes_plot *self_t, *plot; 
  shoes_place place; 
  shoes_canvas *canvas; 
  Data_Get_Struct(self, shoes_plot, self_t); 
  plot = self_t;  // I don't always remember to type self_t
  Data_Get_Struct(c, shoes_canvas, canvas); 
  if (ATTR(self_t->attr, hidden) == Qtrue) return self; 
  int rel =(REL_CANVAS | REL_SCALE);
  shoes_place_decide(&place, c, self_t->attr, self_t->place.w, self_t->place.h, rel, REL_COORDS(rel) == REL_CANVAS);
  
  if (RTEST(actual)) {
    cairo_t *cr = CCR(canvas);
    shoes_apply_transformation(cr, self_t->st, &place, 0);  // cairo_save(cr) inside
    cairo_translate(cr, place.ix + place.dx, place.iy + place.dy);
    
    // draw widget box and fill with color (mostly white). 
    shoes_plot_draw_fill(canvas, self_t);
    // draw title TODO - should use pango/fontmetrics
    cairo_select_font_face(cr, "Helvitica", CAIRO_FONT_SLANT_NORMAL,
      CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    shoes_plot_draw_title(canvas, self_t);
  
    // draw caption TODO: should use pango/fontmetrics
    cairo_select_font_face(cr, "Helvitica", CAIRO_FONT_SLANT_NORMAL,
      CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);  
    shoes_plot_draw_caption(canvas, self_t);
    
    plot->graph_h = plot->place.h - (plot->title_h + plot->caption_h);
    plot->graph_y = plot->title_h + 3;
    plot->yaxis_offset = 50; // TODO TODO run TOTO, run!
    plot->graph_w = plot->place.w - plot->yaxis_offset;
    plot->graph_x = plot->yaxis_offset;

    // draw  box, ticks and x,y labels.
    shoes_plot_draw_adornments(canvas, plot);
    
    // draw data
    shoes_plot_draw_datapts(canvas, plot);
    
    // drawing finished
    shoes_undo_transformation(cr, self_t->st, &place, 0); // doing cairo_restore(cr)
    self_t->place = place;
    // printf("leaving shoes_plot_draw\n"); 
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

static void shoes_plot_draw_fill(shoes_canvas *canvas, shoes_plot *plot)
{
  cairo_set_source_rgb(canvas->cr, 0.99, 0.99, 0.99);
  cairo_set_line_width(canvas->cr, 1);
  cairo_rectangle(canvas->cr, 0, 0, plot->place.w, plot->place.h);
  cairo_stroke_preserve(canvas->cr);
  cairo_fill(canvas->cr);
  cairo_set_source_rgb(canvas->cr, 0.1, 0.1, 0.1);
}

static void shoes_plot_draw_adornments(shoes_canvas *canvas, shoes_plot *plot)
{
  // draw box around data area (plot->graph_?)
  cairo_set_line_width(canvas->cr, 1);
  int t,l,b,r;
  l = plot->graph_x; t = plot->graph_y;
  r = plot->graph_w; b = plot->graph_h;
  //cairo_move_to(canvas->cr, l, t);
  //cairo_show_text(canvas->cr,"foobar");
  cairo_move_to(canvas->cr, l, t);
  cairo_line_to(canvas->cr, r, t);  // across top
  cairo_line_to(canvas->cr, r, b);  // down right side
  cairo_line_to(canvas->cr, l, b);  // across bottom
  cairo_line_to(canvas->cr, l, t);  // up left
  cairo_stroke(canvas->cr);
  shoes_plot_draw_ticks_and_labels(canvas, plot);
  shoes_plot_draw_legend_marker(canvas, plot);
}

static void shoes_plot_draw_ticks_and_labels(shoes_canvas *canvas, shoes_plot *plot)
{
  int top, left, bottom, right; // these are cairo abs for plot->graph
  int width, height;   // full plot space so it includes everything
  int range;
  int h_padding = 45; // default width of tick TODO: an option in plot-> 
  int v_padding = 15; // default height of tick TODO: an option in plot->
  left = plot->graph_x; top = plot->graph_y;
  right = plot->graph_w; bottom = plot->graph_h; 
  range = plot->end_idx - plot->beg_idx;
  width = right - left; 
  height = bottom - top;
  /* java 
		double hScale = width / (double) (end - 1);
		int hInterval = (int) Math.ceil(hPadding / hScale);
  */
  double h_scale; 
  int h_interval; 
  h_scale = width / (double) (range -1);
  h_interval = (int) ceil(h_padding / h_scale);
 
  // draw x axis - labels and tick mark uses plot->xobs - assumes it's string
  // in the array -- TODO: allow a proc to be called to create the string. at 'i'
  int i;
  VALUE xobs = rb_ary_entry(plot->xobs, 0); // first series is x axis descripter
  if (NIL_P(xobs)) printf("FAIL: getting first xobs\n");
  if (TYPE(xobs) != T_ARRAY) printf("FAIL: xobs in not array\n");
  /* java
		for (int i = 1; i < end - 1; i++) {
      int x = (int) Math.round(i * hScale);
      int y = height;
      if (i % hInterval == 0) {
        Date dt = DataSeries.idxToDate(i+begIdx);
        StringBuffer hLabel = new StringBuffer(dTF.format(dt));
			  drawTick(g, x, y, width, height, VERTICALLY);
			  drawLabel(g, x, y, hLabel.toString(), BELOW);
      }
    }
  */
  for (i = 0 ; i < range; i++ ) {
    int x = (int) roundl(i * h_scale);
    x += left;
    long y = bottom;
    if ((i % h_interval) == 0) {
      VALUE rbstr = rb_ary_entry(xobs, i + plot->beg_idx);
      if (NIL_P(rbstr)) {
        rb_raise(rb_eArgError, "FAIL: can't get xobs[%i]\n", i+plot->beg_idx);
      }
      char *rawstr = RSTRING_PTR(rbstr);
      //printf("x label i: %i, x: %i, y: %i, \"%s\" %i %f \n", i, (int) x, (int) y, rawstr, h_interval, h_scale);
      shoes_plot_draw_tick(canvas, plot, x, y, VERTICALLY);
      shoes_plot_draw_label(canvas, plot, x, y, rawstr, BELOW);
    }
  }
    /* java 
		// Draw the vertical ticks for the first two series
		for (int j = 0; j < Math.min(2,numSeries); j++) {
			double maximum = ((Double)maximums.at(j)).doubleValue();
			double minimum = ((Double)minimums.at(j)).doubleValue();
			double vScale = height / (maximum - minimum);
			int vInterval = (int) Math.ceil(vPadding / vScale);

			for (long i = (long) minimum + 1; i < ((long) Math.round(maximum)); i=i+vInterval)
			{
				int x = 0;
				int y = (int) (height - Math.round((i - minimum) * vScale));
				if (j==0) {
	            	String vLabel = new String(new Double(i).toString());
	            	drawTick(g, x, y, width, height, HORIZONTALLY);
	            	drawLabel(g, x, y, vLabel, LEFT);
		  		}
		  		else { // Right side Ticks and Labels
	            	String vLabel = new String(new Double(i).toString());
	            	drawTick(g, width+1, y, width+30, height, HORIZONTALLY);
	            	drawLabel(g, width+2, y, vLabel, RIGHT);
		  		}
			} // for i
		 // for j}
     */
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
        sprintf(tstr, "%i",  (int)i); // not correct for floats? 
        if (j == 0) { // left side y presentation 
            x = left;
            //printf("hoz left %i, %i, %s\n", (int)x, (int)y,tstr);
            shoes_plot_draw_tick(canvas, plot, x, y, HORIZONTALLY);
            shoes_plot_draw_label(canvas, plot, x, y, tstr, LEFT);
        } else {        // right side y presentation
          x = right;
          printf("hoz right %i, %i, %s\n", (int)x, (int)y,tstr);
          shoes_plot_draw_tick(canvas, plot, x, y, HORIZONTALLY);
          shoes_plot_draw_label(canvas, plot, x, y, tstr, RIGHT);        }
      }
    }
}
static void shoes_plot_draw_legend_marker(shoes_canvas *canvas, shoes_plot *plot)
{
  // replace caption with name[s] unless there is one
  //printf("TODO: shoes_plot_draw_legend_marker  called\n");
}

static void shoes_plot_draw_tick(shoes_canvas *canvas, shoes_plot *plot,
    int x, int y, int orientation) 
{
  /* java
		int tickSize = 3;
		Color save = g.getColor();

		g.setColor(Color.black);      

		// Draw the tick mark, gridline, and shading.      
		if (orientation == VERTICALLY)
		{
			g.drawLine(x , y - tickSize, x, y);
			g.setColor(Color.gray);
			g.drawLine(x , y - (tickSize + 1), x, 0);
			g.setColor(Color.white);
			g.drawLine(x + 1 , y , x + 1, 0);
		}
		else // HORIZONTALLY
		{
			g.drawLine(x, y , x + tickSize, y);
			g.setColor(Color.gray);
			g.drawLine(x + (tickSize + 1), y , width, y);
			g.setColor(Color.white);
			g.drawLine(x, y + 1, width, y + 1);
		}
		g.setColor(save);
  */
  int tick_size = 3;
  if (orientation == VERTICALLY) {
    cairo_move_to(canvas->cr, x, y);
    cairo_line_to(canvas->cr, x, plot->graph_y);
  } else if (orientation == HORIZONTALLY) {
    cairo_move_to(canvas->cr, x, y);
    cairo_line_to(canvas->cr, plot->graph_w, y);
  } else {
    printf("FAIL: shoes_plot_draw_tick  orientation\n");
  }
  cairo_stroke(canvas->cr);
}

static void shoes_plot_draw_label(shoes_canvas *canvas, shoes_plot *plot,
    int x, int y, char *str, int where)
{
  /* java
		Color save = g.getColor();
		char characters[] = label.toCharArray();
		int theLength = label.length();
		FontMetrics metrics = getFontMetrics(font);
		int stringWidth = metrics.charsWidth(characters, 0, theLength);

		g.setFont(font);
		g.setColor(Color.black);
  */
  // TODO: Font was previously set to Helvetica 12 and color was setup
  // keep them for now
  cairo_text_extents_t ct;
  cairo_text_extents(canvas->cr, str, &ct);
  int str_w = (int) ct.width;
  int str_h = (int) ct.height;
  /* java 
		if (where == LEFT) {
			int newx = x - (stringWidth + 3);
			int newy = y;
			g.drawChars(characters, 0, theLength, newx, newy);
		} else if (where == RIGHT) {
			g.drawChars(characters, 0, theLength, x, y);
		} else { // BELOW 
			int newx = x - (stringWidth / 2);
			int newy = y + metrics.getHeight() + 3;
			g.drawChars(characters, 0, theLength, newx, newy);
		}
		g.setColor(save);
  */
  int newx;
  int newy;
  if (where == LEFT) { // left side y-axis
    newx = x - (str_w + 3);
    newy = y + (str_h -(str_h / 2));
  } else if (where == RIGHT) { // right side y-axis
    newx = x;
    newx = y;
  } else if (where == BELOW) { // bottom side x axis
    newx = x - (str_w / 2);
    newy = y + str_h + 3;
  } else { 
    printf("FAIL: shoes_plot_draw_label 'where ?'\n");
  }
  cairo_move_to(canvas->cr, newx, newy);
  cairo_show_text(canvas->cr, str);
  // printf("TODO: shoes_plot_draw_label called\n");
}

static void shoes_plot_draw_datapts(shoes_canvas *canvas, shoes_plot *plot)
{
  int i, num_series;
  int top,left,bottom,right;
  left = plot->graph_x; top = plot->graph_y;
  right = plot->graph_w; bottom = plot->graph_h;    
  for (i = 0; i < plot->seriescnt; i++) {
    int oldx = 0;
    int oldy = plot->graph_h; // Needed?
    /* java
      float points[] = (float []) dataSets.at(i);
      double maximum = ((Double)maximums.at(i)).doubleValue();
      double minimum = ((Double)minimums.at(i)).doubleValue();
    */
    VALUE rbvalues = rb_ary_entry(plot->values, i);
    VALUE rbmaxv = rb_ary_entry(plot->maxvs, i);
    VALUE rbminv = rb_ary_entry(plot->minvs, i);
    VALUE rbsize = rb_ary_entry(plot->sizes, i);
    double maximum = NUM2DBL(rbmaxv);
    double minimum = NUM2DBL(rbminv);
    /* java 
			double vScale = height / (maximum - minimum);
			int range = endIdx - begIdx;
			double hScale = width / (double) (range - 1);
			boolean nubs = (width / range > 10) ? true : false;
    */
    // Shoes: Remember - we use ints for x, y, w, h and for drawing lines and points
    int height = bottom - top;
    int width = right - left; 
    int range = plot->end_idx - plot->beg_idx; // zooming adj
    float vScale = height / (maximum - minimum);
    float hScale = width / (double) (range - 1);
    /* java 
			g.setColor(colors[i % colors.length]);
    */
    // TODO: color should be part of the series description
    switch (i) {
      case 0: // blue-ish
        cairo_set_source_rgb(canvas->cr, 0.0, 0.0, 0.9);
        break;
      case 1: // red-ish
        cairo_set_source_rgb(canvas->cr, 0.9, 0.0, 0.0);
        break;
      case 2: // green-ish
        cairo_set_source_rgb(canvas->cr, 0.0, 0.9, 0.0);
        break;
      default: // TODO should have more colors
        cairo_set_source_rgb(canvas->cr, 0.9, 0.9, 0.9);
    }
    /* java
			for (int j = 0; j < range; j++) 
			{
				float v = points[j+begIdx];
				int x = (int) Math.round(j * hScale);
				int y = (int) (height - Math.round((v - minimum) * vScale));

				if (j == 0)
					g.drawLine(x, y, x, y);
				else
					g.drawLine(oldx, oldy, x, y);
            
				if (nubs) drawNub(g, x, y);

				oldx = x;
				oldy = y;
			}
      */
    int j;
    for (j = 0; j < range; j++) {
      VALUE rbdp = rb_ary_entry(rbvalues, j + plot->beg_idx);
      double v = NUM2DBL(rbdp);
      long x = roundl(j * hScale);
      long y = height - roundl((v - minimum) *vScale);
      x += left;
      y += top;
      //printf("draw i: %i, x: %i, y: %i %f \n", j, (int) x, (int) y, hScale);
      if (j == 0)
        cairo_move_to(canvas->cr, x, y);
      else
        cairo_line_to(canvas->cr, x, y);
    }
  } // end of drawing one series
  // tell cairo to draw all lines (and points)
  cairo_stroke(canvas->cr); 
  // set color back to dark gray
  cairo_set_source_rgb(canvas->cr, 0.9, 0.9, 0.9);
}

static void shoes_plot_draw_title(shoes_canvas *canvas, shoes_plot *plot) 
{
  char *t = RSTRING_PTR(plot->title);
  int x, y;
  // TODO simplistic centering assumes helveitca 16 bold
  int offset = (plot->place.w / 2) - (strlen(t) * 8);
  x = plot->place.ix + offset;
  y = plot->title_h;
  cairo_move_to(canvas->cr, x, y);
  cairo_show_text(canvas->cr, t);
}

static void shoes_plot_draw_caption(shoes_canvas *canvas, shoes_plot *plot)
{
  char *t = RSTRING_PTR(plot->caption);
  int x, y;
  // TODO simplistic centering assumes helveitca 12 normal
  int offset = (plot->place.w / 2) - (strlen(t) * 6);
  x = plot->place.ix + offset;
  //y = plot->place.h - plot->caption_h; 
  y = plot->place.h; 
  y -= 2;
  cairo_move_to(canvas->cr, x, y);
  cairo_show_text(canvas->cr, t);}


VALUE shoes_plot_add(VALUE self, VALUE newseries) 
{
  shoes_plot *self_t;
  VALUE rbsz, rbvals, rbobs, rbmin, rbmax, rbshname, rblgname;
  Data_Get_Struct(self, shoes_plot, self_t); 
  int i = self_t->seriescnt; // track number of series to plot.
  if (TYPE(newseries) == T_HASH) {

    rbsz = shoes_hash_get(newseries, rb_intern("num_obs"));
    rbvals = shoes_hash_get(newseries, rb_intern("values"));
    rbobs = shoes_hash_get(newseries, rb_intern("xobs"));
    rbmin = shoes_hash_get(newseries, rb_intern("minv"));
    rbmax = shoes_hash_get(newseries, rb_intern("maxv"));
    rbshname = shoes_hash_get(newseries, rb_intern("name"));
    rblgname = shoes_hash_get(newseries, rb_intern("long_name"));
    if ( NIL_P(rbvals) || TYPE(rbvals) != T_ARRAY ) {
      rb_raise(rb_eArgError, "Missing an Array of values");
    }
    if (NIL_P(rbmin) || NIL_P(rbmax)) {
      rb_raise(rb_eArgError, "Missing minv: or maxv: option");
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
      rb_raise(rb_eArgError, "xobs is not an array");
    }
    if (NIL_P(rblgname)) {
      rblgname = rbshname;
    }
#if 1
    //  For C debugging 
    int l = NUM2INT(rbsz);
    double  min = NUM2DBL(rbmin);
    double  max = NUM2DBL(rbmax);
    char *shname = RSTRING_PTR(rbshname);
    char *lgname = RSTRING_PTR(rblgname);
    printf("shoes_plot_add using hash: num_obs: %i range %f, %f, |%s|, |%s| \n",
       l, min, max, shname, lgname); 
#endif
  } else {
    // throw exception here
    printf("TODO:shoes_plot_add: misssing something\n");
  }
  rb_ary_store(self_t->sizes, i, rbsz);
  rb_ary_store(self_t->values, i, rbvals);
  rb_ary_store(self_t->xobs, i, rbobs);
  rb_ary_store(self_t->maxvs, i, rbmax);
  rb_ary_store(self_t->minvs, i, rbmin);
  rb_ary_store(self_t->names, i, rbshname);
  rb_ary_store(self_t->long_names, i, rblgname);
  self_t->beg_idx = 0;
  self_t->end_idx = NUM2INT(rbsz);
  self_t->seriescnt++;
  return self;
}

VALUE shoes_plot_delete(VALUE self, VALUE series) 
{
  printf("TODO: shoes_plot_add called\n");
  return self;
}

VALUE shoes_plot_redraw(VALUE self) 
{
  printf("TODO: shoes_plot_redraw called\n");
  return self;
}

#if 0
typedef cairo_public cairo_surface_t * (cairo_surface_function_t) (const char *filename, double width, double height);

static cairo_surface_function_t *
get_vector_surface(char *format)
{
  if (strstr(format, "pdf") != NULL) return & cairo_pdf_surface_create;
  if (strstr(format, "ps") != NULL)  return & cairo_ps_surface_create;
  if (strstr(format, "svg") != NULL) return & cairo_svg_surface_create;
  return NULL;
}

static cairo_surface_t* 
buid_surface(VALUE self, VALUE docanvas, double scale, int *result, char *filename, char *format) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t);
  shoes_canvas *canvas;
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  shoes_place place = self_t->place;
  cairo_surface_t *surf;
  cairo_t *cr;
  
  if (docanvas == Qtrue) {
    if (format != NULL)
      surf = get_vector_surface(format)(filename, canvas->width*scale, canvas->height*scale);
    else
      surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)(canvas->width*scale), (int)(canvas->height*scale));
    cr = cairo_create(surf);
    
    if (scale != 1.0) cairo_scale(cr, scale, scale);
// TODO:   *result = shoes_plot_draw_surface(cr, self_t, &place, (int)(place.w*scale), (int)(place.h*scale));
    place.w = (int)(place.w*scale); place.h = (int)(place.h*scale);
    cairo_t *waz_cr = canvas->cr;
    canvas->cr = cr;
    shoes_canvas_draw(self_t->parent, self_t->parent, Qtrue);
    canvas->cr = waz_cr;
    *result = 1; //TODO
  } else {
    int w = (int)(NUM2INT(shoes_plot_get_actual_width(self))*scale);
    int h = (int)(NUM2INT(shoes_plot_get_actual_height(self))*scale);
    if (format != NULL)
      surf = get_vector_surface(format)(filename, w, h);
    else
      surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    cr = cairo_create(surf);
    
    if (scale != 1.0) cairo_scale(cr, scale, scale);
    cairo_translate(cr, -(place.ix + place.dx), -(place.iy + place.dy));
    // TODO *result = shoes_plot_draw_surface(cr, self_t, &place, w, h);
  }
  if (format != NULL) cairo_show_page(cr);
  cairo_destroy(cr);
  
  return surf;
}

VALUE shoes_plot_export(VALUE self, VALUE attr) 
{
  VALUE _filename, _dpi, _docanvas;
  _filename = shoes_hash_get(attr, rb_intern("filename"));
  _dpi = shoes_hash_get(attr, rb_intern("dpi"));
  _docanvas = shoes_hash_get(attr, rb_intern("canvas"));
  double scale = 1.0;
  int result;
  
  if (NIL_P(_filename)) {
    rb_raise(rb_eArgError, "wrong arguments for plot export ({:filename=>'...', "
                            "[:dpi=>90, :canvas=>true|false] })\n:filename is mandatory\n");
  }
  
  if (!NIL_P(_dpi)) scale = NUM2INT(_dpi)/90.0;
  
  cairo_surface_t *surf = buid_surface(self, _docanvas, scale, &result, NULL, NULL);
  
  cairo_status_t r = cairo_surface_write_to_png(surf, RSTRING_PTR(_filename));
  cairo_surface_destroy(surf);
  
  return r == CAIRO_STATUS_SUCCESS ? Qtrue : Qfalse;
}
  
VALUE shoes_plot_save(VALUE self, VALUE attr)
{
  VALUE _filename, _format, _docanvas;
  _filename = shoes_hash_get(attr, rb_intern("filename"));
  _format = shoes_hash_get(attr, rb_intern("format"));
  _docanvas = shoes_hash_get(attr, rb_intern("canvas"));
  int result;
  
  if (NIL_P(_filename) || NIL_P(_format)) {
    rb_raise(rb_eArgError, "wrong arguments for plot save ({:filename=>'...', "
      ":format=>'pdf'|'ps'|'plot' [, :canvas=>true|false] })\n:filename and :format are mandatory");
  }
  
  char *filename = RSTRING_PTR(_filename);
  char *format = RSTRING_PTR(_format);

  cairo_surface_t *surf = buid_surface(self, _docanvas, 1.0, &result, filename, format);
  cairo_surface_destroy(surf);
  
  return result == 0 ? Qfalse : Qtrue;
}
#endif

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
  //printf("remove\n");
  shoes_plot *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_plot, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  
  rb_ary_delete(canvas->contents, self);    // shoes_basic_remove does it this way
  shoes_canvas_repaint_all(self_t->parent); //
  
  // let ruby gc collect handle (it may be shared) just remove this ref
  // TODO self_t->plothandle = Qnil;
  self_t = NULL;
  self = Qnil;
  
  return Qtrue;
}

//called by shoes_plot_send_click and shoes_canvas_send_motion
VALUE
shoes_plot_motion(VALUE self, int x, int y, char *touch)
{
  char h = 0;
  VALUE click;
  GET_STRUCT(plot, self_t);

  click = ATTR(self_t->attr, click);

  if (IS_INSIDE(self_t, x, y)) {
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

  if (button == 1) {
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
  if (button == 1 && (self_t->hover & HOVER_CLICK)) {
    VALUE proc = ATTR(self_t->attr, release);
    self_t->hover ^= HOVER_CLICK; // we have been clicked and released
    if (!NIL_P(proc))
      shoes_safe_block(self, proc, rb_ary_new3(1, self));
  }
}

