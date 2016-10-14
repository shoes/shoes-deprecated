/*
 * plot - draws charts/graphs 
*/
#include "shoes/plot/plot.h"

/* -------- plot_series object - not a widget -----
*/

/*  ------- Plot widget -----
 *  several methods are defined in ruby.c Macros (CLASS_COMMON2, TRANS_COMMON)
 */
 
// some forward declares for functions in this file
void shoes_plot_draw_everything(cairo_t *, shoes_place *, shoes_plot *);

// alloc some memory for a shoes_plot; We'll protect it's Ruby VALUES from gc
// out of caution. fingers crossed.
void
shoes_plot_mark(shoes_plot *self_t)
{
  rb_gc_mark_maybe(self_t->parent);
  rb_gc_mark_maybe(self_t->attr);
  rb_gc_mark_maybe(self_t->series);
  rb_gc_mark_maybe(self_t->values);
  rb_gc_mark_maybe(self_t->xobs);
  rb_gc_mark_maybe(self_t->minvs);
  rb_gc_mark_maybe(self_t->maxvs);
  rb_gc_mark_maybe(self_t->names);
  rb_gc_mark_maybe(self_t->long_names);
  rb_gc_mark_maybe(self_t->strokes);
  rb_gc_mark_maybe(self_t->nubs);
  rb_gc_mark_maybe(self_t->title);
  rb_gc_mark_maybe(self_t->caption);
  rb_gc_mark_maybe(self_t->legend);
  rb_gc_mark_maybe(self_t->background);
  rb_gc_mark_maybe(self_t->color);
}

void
shoes_plot_free(shoes_plot *self_t)
{
  pango_font_description_free (self_t->title_pfd);
  pango_font_description_free (self_t->caption_pfd);
  pango_font_description_free (self_t->legend_pfd);
  pango_font_description_free (self_t->label_pfd);
  shoes_transform_release(self_t->st);
  if (self_t->c_things) {
    switch (self_t-> chart_type) {
      case PIE_CHART:
        shoes_plot_pie_dealloc(self_t);
        break;
      case RADAR_CHART:
        shoes_plot_radar_dealloc(self_t);
        break;
    }
  }
  RUBY_CRITICAL(SHOE_FREE(self_t));
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
  plot->long_names = rb_ary_new();
  plot->strokes = rb_ary_new();
  plot->nubs = rb_ary_new();
  plot->color = rb_ary_new();
  plot->parent = Qnil;
  plot->series = rb_ary_new();
  plot->st = NULL;
  plot->auto_grid = 0;
  plot->x_ticks = 8;
  plot->y_ticks = 6;
  plot->missing = MISSING_SKIP;
  plot->chart_type = LINE_CHART;
  plot->background = Qnil;
  plot->c_things = NULL;
  return obj;
}

VALUE
shoes_plot_new(int argc, VALUE *argv, VALUE parent)
{
  VALUE attr = Qnil, widthObj = Qnil, heightObj = Qnil, optsArg = Qnil;
  VALUE title = Qnil, caption = Qnil, fontreq = Qnil, auto_grid = Qnil;
  VALUE x_ticks = Qnil, y_ticks = Qnil, boundbox = Qnil;
  VALUE missing = Qnil, chart_type = Qnil, background = Qnil;
  VALUE pie_pct = Qnil;
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
    missing = shoes_hash_get(attr, rb_intern("default"));
    chart_type = shoes_hash_get(attr, rb_intern("chart"));
    background = shoes_hash_get(attr, rb_intern("background"));
    boundbox = shoes_hash_get(attr, rb_intern("boundary_box"));
    pie_pct = shoes_hash_get(attr, rb_intern("pie_percent"));
    // there may be many other things in that hash :-)
  } else {
    rb_raise(rb_eArgError, "Plot: missing mandatory {options}");
  }

  VALUE obj = shoes_plot_alloc(cPlot);
  shoes_plot *self_t;
  Data_Get_Struct(obj, shoes_plot, self_t);
  
  self_t->place.w = NUM2INT(widthObj);
  self_t->place.h = NUM2INT(heightObj);
 
  if (NIL_P(chart_type)) {
    self_t->chart_type = LINE_CHART; // default
  } else {
    char *str;
    int err = 0;
    if (TYPE(chart_type) == T_STRING) {
      str = RSTRING_PTR(chart_type);
      if (! strcmp(str, "line"))
        self_t->chart_type = LINE_CHART;
      else if (! strcmp(str, "timeseries"))
        self_t->chart_type = TIMESERIES_CHART;
      else if (! strcmp(str, "column"))
        self_t->chart_type = COLUMN_CHART;
      else if (! strcmp(str, "scatter"))
        self_t->chart_type = SCATTER_CHART;
      else if (! strcmp(str, "pie"))
        self_t->chart_type = PIE_CHART;
      else if (! strcmp(str, "radar"))
        self_t->chart_type = RADAR_CHART;
        //self_t->chart_type = PIE_CHART;
      else 
        err = 1;
    } else err = 1;
    if (err)    
      rb_raise(rb_eArgError, "Plot: bad chart type");
  } 
  
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

     
  self_t->auto_grid = 0;  // default
  if (! NIL_P(auto_grid)) {
    if (RTEST(auto_grid))
      self_t->auto_grid = 1;
  } 
  
  self_t->boundbox = 1;  // default 
  if (! NIL_P(boundbox)) {
    if (! RTEST(boundbox))
      self_t->boundbox = 0;
  }
  
  // TODO :sym would be more ruby like than strings but appears to
  // pollute symbol space in ruby.c and something of PITA.
  if ((!NIL_P(missing)) && (TYPE(missing) == T_STRING)) {
    char *mstr = RSTRING_PTR(missing);
    if (strcmp(mstr, "min") == 0)
      self_t->missing = MISSING_MIN;
    else if (strcmp(mstr, "max") == 0)
      self_t->missing = MISSING_MAX;
    else 
      self_t->missing = MISSING_SKIP;
  } 
  if (self_t->chart_type == PIE_CHART) {
    // using the missing field to keep track of percentage option
    self_t->missing = 0;  // default
    if (! NIL_P(pie_pct))
      self_t->missing = RTEST(pie_pct);
  }
  if (!NIL_P(caption)) {
    self_t->caption = caption;
  } else {
    self_t->caption = rb_str_new2("Missing a caption:");
  }
  
  // setup pangocairo for the caption
  self_t->caption_pfd = pango_font_description_new ();
  pango_font_description_set_family (self_t->caption_pfd, self_t->fontname);
  pango_font_description_set_weight (self_t->caption_pfd, PANGO_WEIGHT_NORMAL);
  pango_font_description_set_absolute_size (self_t->caption_pfd, 12 * PANGO_SCALE);
  
  // setup pangocairo for the legend
  self_t->legend_pfd = pango_font_description_new ();
  pango_font_description_set_family (self_t->legend_pfd, self_t->fontname);
  pango_font_description_set_weight (self_t->legend_pfd, PANGO_WEIGHT_NORMAL);
  pango_font_description_set_absolute_size (self_t->legend_pfd, 14 * PANGO_SCALE);
  
   // setup pangocairo for the labels
  self_t->label_pfd = pango_font_description_new ();
  pango_font_description_set_family (self_t->label_pfd, self_t->fontname);
  pango_font_description_set_weight (self_t->label_pfd, PANGO_WEIGHT_NORMAL);
  pango_font_description_set_absolute_size (self_t->label_pfd, 12 * PANGO_SCALE); 
  
  // TODO: these should be computed based on heuristics (% of vertical?)
  // and font sizes
  self_t->title_h = 50;
  self_t->legend_h = 25;
  self_t->caption_h = 25;
  self_t->yaxis_offset = 50;  
  
  // width of y axis on left and right of plot, in pixels
  // really should be computed based on the data being presented.
  // TODO Of course.

  if (!NIL_P(x_ticks))
    self_t->x_ticks = NUM2INT(x_ticks);
  if (!NIL_P(y_ticks))
    self_t->y_ticks = NUM2INT(y_ticks);
    
  if (! NIL_P(background)) {
    if (TYPE(background) != T_STRING)
        rb_raise(rb_eArgError, "plot backround color must be a string");
    char *cstr = RSTRING_PTR(background);
    VALUE cval = shoes_hash_get(cColors, rb_intern(cstr)); // segfault or raise? 
    if (NIL_P(cval))
      rb_raise(rb_eArgError, "plot.add color: not a known color");
    self_t->background = cval;
  } else {
    self_t->background = shoes_hash_get(cColors, rb_intern("white"));
  }
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
// and by shoes_plot_save_as. The real code is in plot_util.c and the
// other plot_xxxx.c files
void shoes_plot_draw_everything(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
    
    shoes_apply_transformation(cr, self_t->st, place, 0);  // cairo_save(cr) is inside
    cairo_translate(cr, place->ix + place->dx, place->iy + place->dy);
    switch (self_t->chart_type) {
      case TIMESERIES_CHART:
      case LINE_CHART:
        shoes_plot_line_draw(cr, place, self_t);
        break;
      case COLUMN_CHART:
        shoes_plot_column_draw(cr, place, self_t);
        break;
      case SCATTER_CHART:
        shoes_plot_scatter_draw(cr, place, self_t);
        break;
      case PIE_CHART:
        shoes_plot_pie_draw(cr, place, self_t);
        break;
      case RADAR_CHART:
        shoes_plot_radar_draw(cr, place, self_t);
    }
    // drawing finished
    shoes_undo_transformation(cr, self_t->st, place, 0); // does cairo_restore(cr)
    self_t->place = *place;
}

VALUE shoes_plot_add(VALUE self, VALUE newseries) 
{
  shoes_plot *self_t;
  VALUE rbsz, rbvals, rbobs, rbmin, rbmax, rbshname, rblgname, rbcolor;
  VALUE rbstroke, rbnubs, rbnubtype  = Qnil;
  VALUE color_wrapped = Qnil;
  Data_Get_Struct(self, shoes_plot, self_t); 
  int i = self_t->seriescnt; // track number of series to plot.
  if (i >= 6) {
    rb_raise(rb_eArgError, "Maximum of 6 series");
  }
  // chartseries has already parsed it's args. 
  if (rb_obj_is_kind_of(newseries, cChartSeries)) {
    shoes_chart_series *cs;
    Data_Get_Struct(newseries, shoes_chart_series, cs);
    rbsz = RARRAY_LEN(cs->values);
    self_t->beg_idx = 0;
    self_t->end_idx = NUM2INT(rbsz);
    self_t->seriescnt++;
    rb_ary_store(self_t->series, i, newseries);
    // for compatibility with the old way, we need to dup
    rb_ary_store(self_t->values, i, cs->values);
    rb_ary_store(self_t->xobs, i, cs->labels);
    rb_ary_store(self_t->maxvs, i, cs->maxv);
    rb_ary_store(self_t->minvs, i, cs->minv);
    rb_ary_store(self_t->names, i, cs->name);
    rb_ary_store(self_t->long_names, i, cs->desc);
    rb_ary_store(self_t->strokes, i, cs->strokes);
    rb_ary_store(self_t->nubs, i, cs->point_type);
    rb_ary_store(self_t->color, i, cs->color);
    // radar & pie chart types need to pre-compute some geometery and store it
    // in their own structs.  
    if (self_t->chart_type == PIE_CHART) 
      shoes_plot_pie_init(self_t);
    else if (self_t->chart_type == RADAR_CHART) 
     shoes_plot_radar_init(self_t);
    
    shoes_canvas_repaint_all(self_t->parent);
    return self;
  }
  
  if (TYPE(newseries) == T_HASH) {

    rbvals = shoes_hash_get(newseries, rb_intern("values"));
    rbobs = shoes_hash_get(newseries, rb_intern("labels"));
    rbmin = shoes_hash_get(newseries, rb_intern("min"));
    rbmax = shoes_hash_get(newseries, rb_intern("max"));
    rbshname = shoes_hash_get(newseries, rb_intern("name"));
    rblgname = shoes_hash_get(newseries, rb_intern("desc"));
    rbcolor  = shoes_hash_get(newseries, rb_intern("color"));
    rbstroke = shoes_hash_get(newseries, rb_intern("strokewidth"));
    rbnubs = shoes_hash_get(newseries, rb_intern("points"));
    
    if ( NIL_P(rbvals) || TYPE(rbvals) != T_ARRAY ) {
      rb_raise(rb_eArgError, "plot.add: Missing an Array of values");
    }
    int valsz = RARRAY_LEN(rbvals);
    // printf("values rarray_len: %d\n", valsz);
    rbsz = INT2NUM(valsz);
    if (NIL_P(rbmin) || NIL_P(rbmax)) {
      rb_raise(rb_eArgError, "plot.add: Missing minv: or maxv: option");
    }
    int need_x_strings = (self_t->chart_type == LINE_CHART ||
        self_t->chart_type == COLUMN_CHART || 
        self_t->chart_type == TIMESERIES_CHART);
    if ( NIL_P(rbobs) && need_x_strings) {
      // we can fake it - poorly - TODO: call a user given proc ?
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
    if (need_x_strings && TYPE(rbobs) != T_ARRAY ) {
      rb_raise(rb_eArgError, "plot.add xobs is not an array");
    } 
    if (NIL_P(rbshname)) 
      rb_raise(rb_eArgError, "plot.add missing name:");
      
    if (NIL_P(rblgname)) {
      rblgname = rbshname;
    } 
    
    // handle colors
    if (! NIL_P(rbcolor)) {
      if (TYPE(rbcolor) != T_STRING)
        rb_raise(rb_eArgError, "plot.add color must be a string");
      char *cstr = RSTRING_PTR(rbcolor);
      color_wrapped = shoes_hash_get(cColors, rb_intern(cstr));
      if (NIL_P(color_wrapped))
        rb_raise(rb_eArgError, "plot.add color: not a known color");
    } else {
      switch (i) {
      case 0: 
        color_wrapped = shoes_hash_get(cColors, rb_intern("blue"));
        break;
      case 1:
        color_wrapped = shoes_hash_get(cColors, rb_intern("red"));
        break;
      case 2:
        color_wrapped = shoes_hash_get(cColors, rb_intern("green"));
        break;
      case 3:
        color_wrapped = shoes_hash_get(cColors, rb_intern("coral"));
        break;
      case 4:
        color_wrapped = shoes_hash_get(cColors, rb_intern("purple"));
        break;
      case 5:
        color_wrapped = shoes_hash_get(cColors, rb_intern("orange"));
        break;
      }
    }
    
    if (!NIL_P(rbstroke)) {
      if (TYPE(rbstroke) != T_FIXNUM) 
        rb_raise(rb_eArgError, "plot.add strokewidth not an integer\n");
    } else {
      rbstroke = INT2NUM(1); // default
    }
    
    // This is weird. We handle :true, :false/nil and string.
    if (NIL_P(rbnubs)) {
      rbnubtype = INT2NUM(NUB_NONE);
    } else if (TYPE(rbnubs) == T_STRING) {
      char *req = RSTRING_PTR(rbnubs);
      if (!strcmp(req, "dot"))
        rbnubtype = INT2NUM(NUB_DOT);
      else if (!strcmp(req, "circle"))
        rbnubtype = INT2NUM(NUB_CIRCLE);
      else if (!strcmp(req, "box"))
        rbnubtype = INT2NUM(NUB_BOX);
      else if (!strcmp(req, "rect"))
        rbnubtype = INT2NUM(NUB_RECT);
      else
        rb_raise(rb_eArgError, "plot.add nubs: string does not match know nub types\n");
    } else if (TYPE(rbnubs) == T_TRUE) {
        rbnubtype = INT2NUM(NUB_DOT);
    } else if (TYPE(rbnubs) == T_FALSE) {
        rbnubtype = INT2NUM(NUB_NONE);
    }
  } else {
    rb_raise(rb_eArgError, "misssing something in plot.add \n");
  }
  // transition code
  VALUE cs;
  shoes_chart_series *ser;
  switch (self_t->chart_type) {
    // charts using the new chart_series class
    case PIE_CHART:
    case RADAR_CHART: 
    case SCATTER_CHART:
    case COLUMN_CHART:
    case LINE_CHART:
    case TIMESERIES_CHART:
      cs = shoes_chart_series_alloc(cChartSeries);
      Data_Get_Struct(cs, shoes_chart_series, ser);
      shoes_chart_series_Cinit(ser, rbvals, rbobs, rbmax, rbmin, rbshname, rblgname,
      rbstroke, rbnubtype, color_wrapped);
      break;
    default:  // these charts use the old code
      rb_ary_store(self_t->values, i, rbvals);
      rb_ary_store(self_t->xobs, i, rbobs);
      rb_ary_store(self_t->maxvs, i, rbmax);
      rb_ary_store(self_t->minvs, i, rbmin);
      rb_ary_store(self_t->names, i, rbshname);
      rb_ary_store(self_t->long_names, i, rblgname);
      rb_ary_store(self_t->strokes, i, rbstroke);
      rb_ary_store(self_t->nubs, i, rbnubtype);
      rb_ary_store(self_t->color, i, color_wrapped);
  }
  rb_ary_store(self_t->series, i, cs);
  self_t->beg_idx = 0;
  self_t->end_idx = NUM2INT(rbsz);
  self_t->seriescnt++;
  // radar & pie chart types need to pre-compute some geometery and store it
  // in their own structs.  
  if (self_t->chart_type == PIE_CHART) 
    shoes_plot_pie_init(self_t);
  else if (self_t->chart_type == RADAR_CHART) 
    shoes_plot_radar_init(self_t);
    
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
  if (self_t->chart_type == PIE_CHART)
    shoes_plot_pie_dealloc(self_t);
  //rb_ary_delete_at(self_t->sizes, idx);
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
  // restrict to timeseries chart and line chart
  if ((self_t->chart_type != TIMESERIES_CHART) &&
       (self_t->chart_type != LINE_CHART))
    return Qnil;
  if (TYPE(to_here) != T_FIXNUM) 
    rb_raise(rb_eArgError, "plot.redraw_to arg is not an integer");
  int idx = NUM2INT(to_here);
  self_t->end_idx = idx;

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
  for (i = 0; i <self_t->seriescnt; i++) {
    VALUE rbser = rb_ary_entry(self_t->series, i);
    //VALUE rbstr = rb_ary_entry(self_t->names, i);
    shoes_chart_series *cs;
    Data_Get_Struct(rbser, shoes_chart_series, cs);
    VALUE rbstr = cs->name;
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
  // restrict to timeseries chart 
  if (self_t->chart_type != TIMESERIES_CHART) 
    return Qnil;
  if (self_t->seriescnt < 1)
    return Qnil;
  //int maxe = RARRAY_LEN(rb_ary_entry(self_t->values, 0));
  VALUE rbser = rb_ary_entry(self_t->series, 0);
  shoes_chart_series *cs;
  Data_Get_Struct(rbser, shoes_chart_series, cs);
  int maxe = RARRAY_LEN(cs->values);
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
  if (self_t->chart_type != TIMESERIES_CHART) 
    return Qnil;
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
  if (self_t->chart_type != TIMESERIES_CHART) 
    return Qnil;
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
//typedef cairo_public cairo_surface_t * (cairo_surface_function_t) (const char *filename, double width, double height);

cairo_surface_function_t *get_vector_surface(char *format)
{
  if (strcmp(format, "pdf") == 0) return & cairo_pdf_surface_create;
  if (strcmp(format, "ps") == 0)  return & cairo_ps_surface_create;
  if (strcmp(format, "svg") == 0) return & cairo_svg_surface_create;
  return NULL;
}

cairo_surface_t* 
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
int shoes_plot_save_png(VALUE self, char *filename)
{
  int result;
  cairo_surface_t *surf = build_surface(self, 1.0, &result, NULL, NULL);
  cairo_status_t r = cairo_surface_write_to_png(surf, filename);
  cairo_surface_destroy(surf);
  
  return r == CAIRO_STATUS_SUCCESS ? Qtrue : Qfalse;
}

int shoes_plot_save_vector(VALUE self, char *filename, char *format)
{
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
    char *basename = NULL;
    char *lastdot;
    char *ext  = NULL;
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
    //printf("save to: %s %s (long: %s)\n", basename, ext, rbstr);
    int result = 0;
    if (strcmp(ext, "png") == 0) {
      result = shoes_plot_save_png(self, rbstr);
    } else {
      result = shoes_plot_save_vector(self, rbstr, ext);
    }
    if (basename) free(basename);
    return (result ? Qtrue : Qnil);
  }
  return Qnil;
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
  // free some pango/cairo stuff
  pango_font_description_free (self_t->title_pfd);
  pango_font_description_free (self_t->caption_pfd);
  pango_font_description_free (self_t->legend_pfd);
  pango_font_description_free (self_t->label_pfd);
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
int shoes_plot_inside(shoes_plot *self_t, int x, int y)
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
  if (self_t->chart_type != TIMESERIES_CHART) 
    return Qnil;
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
