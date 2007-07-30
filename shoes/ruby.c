//
// shoes/ruby.c
// Just little bits of Ruby I've become accustomed to.
//
#include "shoes/ruby.h"
#include "shoes/app.h"
#include "shoes/canvas.h"
#include "shoes/dialogs.h"
#include "shoes/internal.h"

VALUE mShoes, cCanvas, cFlow, cStack, cPath, cImage, cBackground, cTextClass, cButton, cEditLine, cEditBox, cListBox, cProgress, cColor, cLink;
VALUE reRGB_SOURCE;
ID s_new, s_run, s_to_s, s_call, s_center, s_change, s_click, s_corner, s_draw, s_hidden, s_insert, s_font, s_match, s_x, s_y, s_height, s_width, s_margin, s_marginleft, s_marginright, s_margintop, s_marginbottom;

//
// Mauricio's instance_eval hack (he bested my cloaker back in 06 Jun 2006)
//
VALUE instance_eval_proc;

VALUE
mfp_instance_eval(VALUE obj, VALUE block)
{
  return rb_funcall(instance_eval_proc, s_call, 2, obj, block);
}

VALUE
shoes_attr_set(VALUE hsh, ID key, VALUE val)
{
  if (NIL_P(hsh))
    hsh = rb_hash_new();
  rb_hash_aset(hsh, ID2SYM(key), val);
  return hsh;
}

VALUE
shoes_attr_find(ID key, VALUE hsh1, VALUE hsh2)
{
  VALUE v;

  if (!NIL_P(hsh1))
  {
    v = rb_hash_aref(hsh1, ID2SYM(key));
    if (!NIL_P(v)) return v;
  }

  if (!NIL_P(hsh2))
  {
    v = rb_hash_aref(hsh2, ID2SYM(key));
    if (!NIL_P(v)) return v;
  }

  return Qnil;
}

int
shoes_attr_int(ID key, VALUE hsh1, VALUE hsh2, int dn)
{
  VALUE v = shoes_attr_find(key, hsh1, hsh2);
  if (!NIL_P(v)) return NUM2INT(v);
  return dn;
}

double
shoes_attr_dbl(ID key, VALUE hsh1, VALUE hsh2, double dn)
{
  VALUE v = shoes_attr_find(key, hsh1, hsh2);
  if (!NIL_P(v)) return NUM2DBL(v);
  return dn;
}

char *
shoes_attr_cstr(ID key, VALUE hsh1, VALUE hsh2, char *dn)
{
  VALUE v = shoes_attr_find(key, hsh1, hsh2);
  if (!NIL_P(v)) return RSTRING_PTR(v);
  return dn;
}

VALUE
rb_str_to_pas(VALUE str)
{
  VALUE str2;
  char strlen[2];
  strlen[0] = RSTRING_LEN(str);
  strlen[1] = '\0';
  str2 = rb_str_new2(strlen);
  rb_str_append(str2, str);
  return str2;
}

//
// Macros for setting up drawing
//
#define SETUP(self_type, dw, dh) \
  int absy; \
  double x, y, w, h; \
  self_type *self_t; \
  shoes_canvas *canvas; \
  VALUE ck = rb_obj_class(c); \
  Data_Get_Struct(self, self_type, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  x = ATTR2DBL(x, canvas->cx); \
  absy = (ATTR(y) == Qnil ? 0 : 1); \
  y = ATTR2DBL(y, canvas->cy); \
  w = ATTR2DBL(width, dw); \
  h = ATTR2DBL(height, dh); \
  if (ck == cStack || x + w > canvas->width) \
  { \
    x = canvas->x; \
    y = canvas->endy; \
  }

#define SETUP_CONTROL() \
  char *msg; \
  int x, y, absy, w, h; \
  shoes_control *self_t; \
  shoes_canvas *canvas; \
  VALUE ck = rb_obj_class(c); \
  Data_Get_Struct(self, shoes_control, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  x = ATTR2INT(x, canvas->cx); \
  absy = (ATTR(y) == Qnil ? 0 : 1); \
  y = ATTR2INT(y, canvas->cy); \
  w = ATTR2INT(width, (RSTRING_LEN(self_t->text) * 6) + 32); \
  h = ATTR2INT(height, 28); \
  if (ck == cStack || x + w > canvas->width) \
  { \
    canvas->cx = x = canvas->x; \
    canvas->cy = y = canvas->endy; \
  } \
  msg = RSTRING_PTR(self_t->text)

#define FINISH() \
  if (!absy) { \
    canvas->cx += w; \
    canvas->cy = y; \
    canvas->endx = canvas->cx; \
    canvas->endy = y + h; \
  } \
  if (ck == cStack) { \
    canvas->cx = 0; \
    canvas->cy = canvas->endy; \
  }

#define CHANGED_COORDS() x != self_t->x || y != self_t->y || w != self_t->w || h != self_t->h
#define PLACE_COORDS() self_t->x = x; self_t->y = y; self_t->w = w; self_t->h = h
#ifdef SHOES_GTK
#define PLACE_CONTROL() \
  gtk_widget_set_size_request(self_t->ref, w, h); \
  gtk_layout_put(GTK_LAYOUT(canvas->slot.canvas), self_t->ref, x, y); \
  gtk_widget_show(self_t->ref); \
  PLACE_COORDS();

#define REPAINT_CONTROL() \
  if (CHANGED_COORDS()) { \
    gtk_layout_move(GTK_LAYOUT(canvas->slot.canvas), self_t->ref, x, y); \
    gtk_widget_set_size_request(self_t->ref, w, h); \
    PLACE_COORDS(); \
  } \
  if (canvas->slot.expose != NULL) \
  { \
    gtk_container_propagate_expose(GTK_CONTAINER(canvas->slot.canvas), self_t->ref, canvas->slot.expose); \
  }
#endif

#ifdef SHOES_QUARTZ
#define PLACE_CONTROL() \
  HIRect hr; \
  hr.origin.x = x; hr.origin.y = y; hr.size.width = w; hr.size.height = h; \
  HIViewAddSubview(canvas->slot.view, self_t->ref); \
  SetControlCommandID(self_t->ref, SHOES_CONTROL1 + RARRAY_LEN(canvas->slot.controls)); \
  SetControlReference(self_t->ref, (SInt32)canvas->slot.scrollview); \
  HIViewSetFrame(self_t->ref, &hr); \
  HIViewSetVisible(self_t->ref, true); \
  rb_ary_push(canvas->slot.controls, self); \
  PLACE_COORDS();

#define REPAINT_CONTROL() \
  if (CHANGED_COORDS()) { \
    HIRect hr; \
    hr.origin.x = x; hr.origin.y = y; hr.size.width = w; hr.size.height = h; \
    HIViewSetFrame(self_t->ref, &hr); \
    PLACE_COORDS(); \
  }
#endif

#ifdef SHOES_WIN32
#define PLACE_CONTROL() \
  PLACE_COORDS()

#define REPAINT_CONTROL() \
  if (CHANGED_COORDS()) { \
    MoveWindow(self_t->ref, x, y, w, h, FALSE); \
    PLACE_COORDS(); \
  }

inline void shoes_win32_control_font(int id, HWND hwnd)
{
  SendDlgItemMessage(hwnd, id, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(true, 0));
}
#endif

//
// Shoes::Path
//
static void
shoes_path_mark(shoes_path *path)
{
  rb_gc_mark_maybe(path->parent);
  rb_gc_mark_maybe(path->attr);
}

static void
shoes_path_free(shoes_path *path)
{
  if (path->line != NULL)
    cairo_path_destroy(path->line);
  free(path);
}

VALUE
shoes_path_new(cairo_path_t *line, VALUE parent)
{
  shoes_path *path;
  VALUE obj = shoes_path_alloc(cPath);
  Data_Get_Struct(obj, shoes_path, path);
  path->line = line;
  path->parent = parent;
  return obj;
}

VALUE
shoes_path_alloc(VALUE klass)
{
  shoes_path *path;
  VALUE obj = Data_Make_Struct(klass, shoes_path, shoes_path_mark, shoes_path_free, path);
  path->line = NULL;
  path->attr = Qnil;
  path->parent = Qnil;
  return obj;
}

VALUE
shoes_path_move(VALUE self, VALUE x, VALUE y)
{
  shoes_path *self_t;
  Data_Get_Struct(self, shoes_path, self_t);
  ATTRSET(x, x);
  ATTRSET(y, y);
  shoes_canvas_repaint_all(self_t->parent);
  return self;
}

VALUE
shoes_path_draw(VALUE self, VALUE c, VALUE attr)
{
  double x, y;
  shoes_path *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_path, self_t);
  Data_Get_Struct(c, shoes_canvas, canvas);

  x = ATTR2DBL(x, 0);
  y = ATTR2DBL(y, 0);
  cairo_new_path(canvas->cr);
  cairo_translate(canvas->cr, x, y);
  cairo_append_path(canvas->cr, self_t->line);

  if (canvas->bg.on)
  {
    cairo_set_source_rgba(canvas->cr, canvas->bg.r, canvas->bg.g, canvas->bg.b, canvas->bg.a);
    cairo_fill_preserve(canvas->cr);
  }
  if (canvas->fg.on)
  {
    cairo_set_source_rgba(canvas->cr, canvas->fg.r, canvas->fg.g, canvas->fg.b, canvas->fg.a);
    cairo_stroke_preserve(canvas->cr);
  }

  return self;
}

//
// Shoes::Image
//
static void
shoes_image_mark(shoes_image *image)
{
  rb_gc_mark_maybe(image->path);
  rb_gc_mark_maybe(image->parent);
  rb_gc_mark_maybe(image->attr);
}

static void
shoes_image_free(shoes_image *image)
{
  if (image->surface != NULL)
    cairo_surface_destroy(image->surface);
  free(image);
}

VALUE
shoes_image_new(VALUE klass, VALUE path, VALUE attr, VALUE parent)
{
  shoes_image *image;
  VALUE obj = shoes_image_alloc(klass);
  Data_Get_Struct(obj, shoes_image, image);
  image->path = path;
  image->surface = cairo_image_surface_create_from_png(RSTRING_PTR(path));
  image->width = cairo_image_surface_get_width(image->surface);
  image->height = cairo_image_surface_get_height(image->surface);
  image->attr = attr;
  image->parent = parent;
  return obj;
}

VALUE
shoes_image_alloc(VALUE klass)
{
  shoes_image *image;
  VALUE obj = Data_Make_Struct(klass, shoes_image, shoes_image_mark, shoes_image_free, image);
  image->path = Qnil;
  image->surface = NULL;
  image->attr = Qnil;
  image->parent = Qnil;
  image->width = 0.0;
  image->height = 0.0;
  return obj;
}

VALUE
shoes_image_draw(VALUE self, VALUE c, VALUE attr)
{
  SETUP(shoes_image, self_t->width, self_t->height);
  shoes_canvas_shape_do(canvas, x + (w / 2.), y + (h / 2.));
  if (w != self_t->width || h != self_t->height)
  {
    cairo_scale(canvas->cr, w/self_t->width, h/self_t->height);
  }
  cairo_set_source_surface(canvas->cr, self_t->surface, -w / 2., -h / 2.);
  cairo_paint(canvas->cr);
  cairo_restore(canvas->cr);
  FINISH();
  return self;
}

//
// Shoes::Pattern
//
static void
shoes_pattern_mark(shoes_pattern *pattern)
{
  rb_gc_mark_maybe(pattern->source);
  rb_gc_mark_maybe(pattern->parent);
  rb_gc_mark_maybe(pattern->attr);
}

static void
shoes_pattern_free(shoes_pattern *pattern)
{
  if (pattern->surface != NULL)
    cairo_surface_destroy(pattern->surface);
  if (pattern->pattern != NULL)
    cairo_pattern_destroy(pattern->pattern);
  free(pattern);
}

VALUE
shoes_pattern_new(VALUE klass, VALUE source, VALUE attr, VALUE parent)
{
  shoes_pattern *pattern;
  VALUE obj = shoes_pattern_alloc(klass);
  VALUE rgb = rb_funcall(source, s_match, 1, reRGB_SOURCE);
  Data_Get_Struct(obj, shoes_pattern, pattern);
  pattern->source = source;
  if (!NIL_P(rgb))
  {
    int r = NUM2INT(rb_Integer(rb_reg_nth_match(1, rgb)));
    int g = NUM2INT(rb_Integer(rb_reg_nth_match(2, rgb)));
    int b = NUM2INT(rb_Integer(rb_reg_nth_match(3, rgb)));
    pattern->pattern = cairo_pattern_create_rgb(r / 255., g / 255., b / 255.);
  }
  else
  {
    pattern->surface = cairo_image_surface_create_from_png(RSTRING_PTR(source));
    pattern->pattern = cairo_pattern_create_for_surface(pattern->surface);
  }
  cairo_pattern_set_extend(pattern->pattern, CAIRO_EXTEND_REPEAT);
  pattern->attr = attr;
  pattern->parent = parent;
  return obj;
}

VALUE
shoes_pattern_alloc(VALUE klass)
{
  shoes_pattern *pattern;
  VALUE obj = Data_Make_Struct(klass, shoes_pattern, shoes_pattern_mark, shoes_pattern_free, pattern);
  pattern->source = Qnil;
  pattern->surface = NULL;
  pattern->pattern = NULL;
  pattern->attr = Qnil;
  pattern->parent = Qnil;
  return obj;
}

VALUE
shoes_pattern_draw(VALUE self, VALUE c, VALUE attr)
{
  SETUP(shoes_pattern, 0, 0);
  shoes_canvas_shape_do(canvas, x + (w / 2.), y + (h / 2.));
  cairo_set_source_surface(canvas->cr, self_t->surface, -w / 2., -h / 2.);
  cairo_paint(canvas->cr);
  cairo_restore(canvas->cr);
  FINISH();
  return self;
}

VALUE
shoes_background_draw(VALUE self, VALUE c, VALUE attr)
{
  cairo_pattern_t *pattern;
  SETUP(shoes_pattern, 0, 0);
  shoes_canvas_shape_do(canvas, 0, 0);
  cairo_set_source(canvas->cr, self_t->pattern);
  cairo_rectangle(canvas->cr, 0, 0, 4000, 4000);
  cairo_fill(canvas->cr);
  w = 0; h = 0;
  FINISH();
  return self;
}

//
// Shoes::Link
//
static void
shoes_link_mark(shoes_link *link)
{
  rb_gc_mark_maybe(link->url);
}

static void
shoes_link_free(shoes_link *link)
{
  free(link);
}

VALUE
shoes_link_new(VALUE url, int start, int end)
{
  shoes_link *link;
  VALUE obj = shoes_link_alloc(cLink);
  Data_Get_Struct(obj, shoes_link, link);
  link->url = url;
  link->start = start;
  link->end = end;
  return obj;
}

VALUE
shoes_link_alloc(VALUE klass)
{
  shoes_link *link;
  VALUE obj = Data_Make_Struct(klass, shoes_link, shoes_link_mark, shoes_link_free, link);
  link->start = 0;
  link->end = 0;
  link->url = Qnil;
  return obj;
}

VALUE
shoes_link_at(VALUE self, int index)
{
  shoes_link *link;
  Data_Get_Struct(self, shoes_link, link);
  if (link->start <= index && link->end >= index)
  {
    return link->url;
  }
  return Qnil;
}

//
// Shoes::Text
//
static void
shoes_text_mark(shoes_text *text)
{
  rb_gc_mark_maybe(text->markup);
  rb_gc_mark_maybe(text->links);
  rb_gc_mark_maybe(text->parent);
  rb_gc_mark_maybe(text->attr);
  rb_gc_mark_maybe(text->linku);
}

static void
shoes_text_free(shoes_text *text)
{
  if (text->layout == NULL)
    g_object_unref(text->layout);
  free(text);
}

static void
start_element_handler(GMarkupParseContext *context,
					  const gchar *element_name,
					  const gchar **attribute_names,
					  const gchar **attribute_values,
					  gpointer user_data,
					  GError **error)
{
  shoes_text *self_t = (shoes_text *)user_data;

	if (!strcmp(element_name, "a"))
	{
		const gchar *url = NULL;
		int line_number;
		int char_number;
		int i;

		g_markup_parse_context_get_position(context, &line_number,
											&char_number);

		for (i = 0; attribute_names[i] != NULL; i++)
		{
			const gchar *attr = attribute_names[i];

			if (!strcmp(attr, "href"))
			{
				if (url != NULL)
				{
					g_set_error(error, G_MARKUP_ERROR,
								G_MARKUP_ERROR_INVALID_CONTENT,
								"Attribute '%s' occurs twice on <a> tag "
								"on line %d char %d, may only occur once",
								attribute_names[i], line_number, char_number);
					return;
				}

				url = attribute_values[i];
			}
			else
			{
				g_set_error(error, G_MARKUP_ERROR,
							G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
							"Attribute '%s' is not allowed on the <a> tag "
							"on line %d char %d",
							attribute_names[i], line_number, char_number);
				return;
			}
		}

		if (url == NULL)
		{
			g_set_error(error, G_MARKUP_ERROR,
						G_MARKUP_ERROR_INVALID_CONTENT,
						"Attribute 'href' was missing on the <a> tag "
						"on line %d char %d",
						line_number, char_number);
			return;
		}

		g_string_append(self_t->tmp,
						"<span color=\"blue\" underline=\"single\">");

    self_t->linku = rb_str_new2(url);
    self_t->linki = self_t->i;
	}
	else
	{
		int i;

		g_string_append_printf(self_t->tmp,
							   "<%s", element_name);

		for (i = 0; attribute_names[i] != NULL; i++)
		{
			const gchar *attr  = attribute_names[i];
			const gchar *value = attribute_values[i];

			g_string_append_printf(self_t->tmp,
								   " %s=\"%s\"",
								   attr, value);
		}

		g_string_append_c(self_t->tmp, '>');
	}
}

static void
end_element_handler(GMarkupParseContext *context,
					const gchar *element_name,
					gpointer user_data,
					GError **error)
{
  shoes_text *self_t = (shoes_text *)user_data;

	if (!strcmp(element_name, "a"))
	{
    if (self_t->linki >= 0)
    {
      rb_ary_push(self_t->links, shoes_link_new(self_t->linku, self_t->linki, self_t->i - 1));
      self_t->linku = Qnil;
      self_t->linki = -1;
    }
		g_string_append(self_t->tmp, "</span> ");
    self_t->i += 1;
	}
	else
	{
		g_string_append_printf(self_t->tmp,
							   "</%s>", element_name);
	}
}

static void
text_handler(GMarkupParseContext *context,
			 const gchar *text,
			 gsize text_len,
			 gpointer user_data,
			 GError **error)
{
  shoes_text *self_t = (shoes_text *)user_data;
  self_t->i += text_len;
	gchar *newtext = g_markup_escape_text(text, text_len);
	g_string_append_len(self_t->tmp, newtext, strlen (newtext));
	g_free (newtext);
}

static const GMarkupParser markup_parser =
{
  start_element_handler,
  end_element_handler,
  text_handler,
  NULL,
  NULL
};

VALUE
shoes_text_parse(VALUE self, VALUE markup)
{
  shoes_text *self_t;
  GMarkupParseContext *context = NULL;
  GError *error = NULL;
  const gchar *start;
  gsize len;

  Data_Get_Struct(self, shoes_text, self_t);
  start = RSTRING_PTR(markup);
  len = RSTRING_LEN(markup);

  self_t->tmp = g_string_new(NULL);
  self_t->links = rb_ary_new();
  self_t->i = 0;

  context = g_markup_parse_context_new(&markup_parser, (GMarkupParseFlags)0, self_t, NULL);
  if (g_markup_parse_context_parse(context, start, len, &error) &&
      g_markup_parse_context_end_parse(context, &error))
  {
    self_t->markup = rb_str_new2(g_string_free(self_t->tmp, FALSE));
  }
  else
  {
    g_string_free(self_t->tmp, TRUE);
  }

  self_t->tmp = NULL;
  if (error != NULL)
    g_error_free(error);

  g_markup_parse_context_free(context);

  return Qtrue;
}

VALUE
shoes_text_new(VALUE markup, VALUE attr, VALUE parent)
{
  shoes_text *text;
  VALUE obj = shoes_text_alloc(cTextClass);
  Data_Get_Struct(obj, shoes_text, text);
  text->attr = attr;
  text->parent = parent;
  text->linku = Qnil;
  text->linki = -1;
  shoes_text_parse(obj, markup);
  return obj;
}

VALUE
shoes_text_alloc(VALUE klass)
{
  shoes_text *text;
  VALUE obj = Data_Make_Struct(klass, shoes_text, shoes_text_mark, shoes_text_free, text);
  text->markup = Qnil;
  text->links = Qnil;
  text->attr = Qnil;
  text->parent = Qnil;
  text->layout = NULL;
  return obj;
}

VALUE
shoes_text_get_markup(VALUE self)
{
  shoes_text *self_t;
  Data_Get_Struct(self, shoes_text, self_t);
  return self_t->markup;
}

VALUE
shoes_text_set_markup(VALUE self, VALUE markup)
{
  shoes_text *self_t;
  Data_Get_Struct(self, shoes_text, self_t);
  shoes_text_parse(self, markup);
  shoes_canvas_repaint_all(self_t->parent);
  return self_t->markup;
}

VALUE
shoes_text_click(VALUE self, int button, int x, int y)
{
  int index, trailing, i;
  shoes_text *self_t;
  Data_Get_Struct(self, shoes_text, self_t);
  if (self_t->layout == NULL) return Qnil;

  x -= self_t->x;
  y -= self_t->y;
  if (pango_layout_xy_to_index(self_t->layout, x * PANGO_SCALE, y * PANGO_SCALE, &index, &trailing))
  {
    for (i = 0; i < RARRAY_LEN(self_t->links); i++)
    {
      VALUE url = shoes_link_at(rb_ary_entry(self_t->links, i), index);
      if (!NIL_P(url))
      {
        return url;
      }
    }
  }

  return Qnil;
}

VALUE
shoes_text_draw(VALUE self, VALUE c, VALUE attr)
{
  int px, py, pd, li, m;
  double cx, cy;
  char *font;
  shoes_text *self_t;
  shoes_canvas *canvas;
  PangoLayoutLine *last;
  PangoRectangle lrect;
  PangoFontDescription *desc;

  Data_Get_Struct(self, shoes_text, self_t);
  Data_Get_Struct(c, shoes_canvas, canvas);

  ATTR_MARGINS(4);
  self_t->x = ATTR2INT(x, canvas->cx) + lmargin;
  self_t->y = ATTR2INT(y, canvas->cy) + tmargin;
  self_t->w = ATTR2INT(width, canvas->width - (canvas->cx - self_t->x)) - (lmargin + rmargin);
  font = ATTR2CSTR(font, "Arial 12");

  if (self_t->layout != NULL)
    g_object_unref(self_t->layout);

  self_t->layout = pango_cairo_create_layout(canvas->cr);
  pd = 0;
  if (self_t->x == canvas->cx + lmargin)
  {
    if (self_t->x - canvas->x > (self_t->w - (lmargin + rmargin)) - 20)
    {
      self_t->x = canvas->x + lmargin;
      self_t->y = canvas->endy;
    }
    cairo_move_to(canvas->cr, canvas->x + lmargin, self_t->y);
    if (self_t->x > canvas->x) {
      pd = (self_t->x - (canvas->x + lmargin)) * PANGO_SCALE;
      pango_layout_set_indent(self_t->layout, pd);
      self_t->w = (canvas->width - (canvas->cx - canvas->x)) - rmargin;
    }
  }
  else
    cairo_move_to(canvas->cr, self_t->x, self_t->y);

  INFO("TEXT: %0.2f, %0.2f (%d, %d) / %d, %d / %d, %d\n", canvas->cx, canvas->cy,
    canvas->width, canvas->height, self_t->x, self_t->y, self_t->w, self_t->h);
  pango_layout_set_markup(self_t->layout, RSTRING_PTR(self_t->markup), -1);
  pango_layout_set_width(self_t->layout, self_t->w * PANGO_SCALE);
  desc = pango_font_description_from_string(font);
  pango_layout_set_font_description(self_t->layout, desc);
  pango_font_description_free(desc);

  pango_cairo_update_layout(canvas->cr, self_t->layout);
  pango_cairo_show_layout(canvas->cr, self_t->layout);

  li = pango_layout_get_line_count(self_t->layout) - 1;
  last = pango_layout_get_line(self_t->layout, li);
  pango_layout_line_get_pixel_extents(last, NULL, &lrect);
  pango_layout_get_pixel_size(self_t->layout, &px, &py);

  // newlines have an empty size
  if (lrect.width == 0) {
    canvas->cx = (double)lrect.x;
  } else {
    canvas->cx = (double)((lrect.x + lrect.width) + rmargin);
  }
  canvas->endx += px + rmargin;
  canvas->endy += py + bmargin;
  return self;
}

//
// Shoes::Button
//
static void
shoes_control_mark(shoes_control *control)
{
  rb_gc_mark_maybe(control->parent);
  rb_gc_mark_maybe(control->attr);
}

static void
shoes_control_free(shoes_control *control)
{
  free(control);
}

VALUE
shoes_control_new(VALUE klass, VALUE text, VALUE attr, VALUE parent)
{
  shoes_control *control;
  VALUE obj = shoes_control_alloc(klass);
  Data_Get_Struct(obj, shoes_control, control);
  control->text = text;
  control->attr = attr;
  control->parent = parent;
  return obj;
}

VALUE
shoes_control_alloc(VALUE klass)
{
  shoes_control *control;
  VALUE obj = Data_Make_Struct(klass, shoes_control, shoes_control_mark, shoes_control_free, control);
  control->x = control->y = control->w = control->h = 0;
  control->text = Qnil;
  control->ref = NULL;
  control->attr = Qnil;
  control->parent = Qnil;
  return obj;
}

void
shoes_control_send(VALUE self, ID event)
{
  VALUE click;
  shoes_control *self_t;
  Data_Get_Struct(self, shoes_control, self_t);

  click = ATTR(click);
  if (!NIL_P(click))
    rb_funcall(click, s_call, 0);
}

#ifdef SHOES_GTK
static gboolean
shoes_button_gtk_clicked(GtkButton *button, gpointer data)
{ 
  VALUE self = (VALUE)data;
  shoes_control_send(self, s_click);
  return TRUE;
}
#endif

VALUE
shoes_button_draw(VALUE self, VALUE c, VALUE attr)
{
  SETUP_CONTROL();

  if (self_t->ref == NULL)
  {

#ifdef SHOES_GTK
    self_t->ref = gtk_button_new_with_label(_(msg));
    g_signal_connect(G_OBJECT(self_t->ref), "clicked",
                     G_CALLBACK(shoes_button_gtk_clicked),
                     (gpointer)self);
#endif

#ifdef SHOES_QUARTZ
    Rect r = {y, x, y + h, x + w};
    CFStringRef cfmsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
    CreatePushButtonControl(NULL, &r, cfmsg, &self_t->ref);
    CFRelease(cfmsg);
#endif

#ifdef SHOES_WIN32
    int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot.controls);
    self_t->ref = CreateWindowEx(0, TEXT("BUTTON"), msg,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y, w, h, canvas->slot.window, (HMENU)cid, 
        (HINSTANCE)GetWindowLong(canvas->slot.window, GWL_HINSTANCE),
        NULL);
    shoes_win32_control_font(cid, canvas->slot.window);
    rb_ary_push(canvas->slot.controls, self);
#endif
    PLACE_CONTROL();
  }
  else
  {
    REPAINT_CONTROL();
  }

  FINISH();

  return self;
}

VALUE
shoes_edit_line_draw(VALUE self, VALUE c, VALUE attr)
{
  SETUP_CONTROL();

  if (self_t->ref == NULL)
  {

#ifdef SHOES_GTK
    self_t->ref = gtk_entry_new();
    if (!NIL_P(msg))
      gtk_entry_set_text(GTK_ENTRY(self_t->ref), _(msg));
#endif

#ifdef SHOES_QUARTZ
    Rect r;
    CFStringRef cfmsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
    SetRect(&r, x, y, x + w, y + h);
    CreateEditUnicodeTextControl(NULL, &r, cfmsg, false, NULL, &self_t->ref);
    CFRelease(cfmsg);
#endif

#ifdef SHOES_WIN32
    self_t->ref = CreateWindowEx(0, TEXT("EDIT"), NULL,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
        x, y, w, h, canvas->slot.window, NULL, 
        (HINSTANCE)GetWindowLong(canvas->slot.window, GWL_HINSTANCE),
        NULL);
    SendMessage(self_t->ref, WM_SETTEXT, 0, (LPARAM)msg);
#endif
    PLACE_CONTROL();
  }
  else
  {
    REPAINT_CONTROL();
  }

  FINISH();

  return self;
}


VALUE
shoes_edit_box_draw(VALUE self, VALUE c, VALUE attr)
{
  SETUP_CONTROL();

  if (self_t->ref == NULL)
  {

#ifdef SHOES_GTK
    GtkTextBuffer *buffer;
    self_t->ref = gtk_text_view_new();
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self_t->ref));
    gtk_text_buffer_set_text(buffer, _(msg), -1);
#endif

#ifdef SHOES_QUARTZ
    Rect r;
    CFStringRef cfmsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
    SetRect(&r, x, y, x + w, y + h);
    CreateEditUnicodeTextControl(NULL, &r, cfmsg, false, NULL, &self_t->ref);
    CFRelease(cfmsg);
#endif

#ifdef SHOES_WIN32
    self_t->ref = CreateWindowEx(0, TEXT("EDIT"), NULL,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
        x, y, w, h, canvas->slot.window, NULL, 
        (HINSTANCE)GetWindowLong(canvas->slot.window, GWL_HINSTANCE),
        NULL);
    SendMessage(self_t->ref, WM_SETTEXT, 0, (LPARAM)msg);
#endif
    PLACE_CONTROL();
  }
  else
  {
    REPAINT_CONTROL();
  }

  FINISH();

  return self;
}

VALUE
shoes_list_box_draw(VALUE self, VALUE c, VALUE attr)
{
  SETUP_CONTROL();

  if (self_t->ref == NULL)
  {
#ifdef SHOES_GTK
    self_t->ref = gtk_combo_box_new();
#endif

#ifdef SHOES_QUARTZ
    Rect r;
    SetRect(&r, x, y, x + w, y + h);
    CreateListBoxControl(NULL, &r, true, 40, 1, false, false, 20, 10, false, NULL, &self_t->ref);
#endif
    PLACE_CONTROL();
  }
  else
  {
    REPAINT_CONTROL();
  }

  FINISH();

  return self;
}

VALUE
shoes_progress_draw(VALUE self, VALUE c, VALUE attr)
{
  SETUP_CONTROL();

  if (self_t->ref == NULL)
  {
#ifdef SHOES_GTK
    self_t->ref = gtk_progress_bar_new();
    gtk_progress_bar_set_text(self_t->ref, _(msg));
#endif

#ifdef SHOES_QUARTZ
    Rect r;
    CFStringRef cfmsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
    SetRect(&r, x, y, x + w, y + h);
    CreatePushButtonControl(NULL, &r, cfmsg, &self_t->ref);
    CFRelease(cfmsg);
#endif

#ifdef SHOES_WIN32
    INITCOMMONCONTROLSEX InitCtrlEx;
    InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCtrlEx.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&InitCtrlEx);

    self_t->ref = CreateWindowEx(0, PROGRESS_CLASS, msg,
        WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
        x, y, w, h, canvas->slot.window, NULL, 
        (HINSTANCE)GetWindowLong(canvas->slot.window, GWL_HINSTANCE),
        NULL);
#endif
    PLACE_CONTROL();
  }
  else
  {
    REPAINT_CONTROL();
  }

  FINISH();

  return self;
}

#define C(n, s) \
  re##n = rb_eval_string(s); \
  rb_const_set(mShoes, rb_intern("" # n), re##n);

//
// Everything exposed to Ruby is exposed here.
//
void
shoes_ruby_init()
{
  instance_eval_proc = rb_eval_string("lambda{|o,b| o.instance_eval(&b)}");
  rb_gc_register_address(&instance_eval_proc);
  s_new = rb_intern("new");
  s_run = rb_intern("run");
  s_to_s = rb_intern("to_s");
  s_call = rb_intern("call");
  s_center = rb_intern("center");
  s_change = rb_intern("change");
  s_click = rb_intern("click");
  s_corner = rb_intern("corner");
  s_draw = rb_intern("draw");
  s_font = rb_intern("font");
  s_hidden = rb_intern("hidden");
  s_insert = rb_intern("insert");
  s_match = rb_intern("match");
  s_x = rb_intern("x");
  s_y = rb_intern("y");
  s_height = rb_intern("height");
  s_width = rb_intern("width");
  s_margin = rb_intern("margin");
  s_marginleft = rb_intern("marginleft");
  s_marginright = rb_intern("marginright");
  s_margintop = rb_intern("margintop");
  s_marginbottom = rb_intern("marginbottom");

  mShoes = rb_define_module("Shoes");
  C(RGB_SOURCE, "/^rgb\\((\\d+), *(\\d+), *(\\d+)\\)$/i");

  rb_define_singleton_method(mShoes, "app", CASTHOOK(shoes_app_main), -1);
  cCanvas = rb_define_class("Canvas", rb_cObject);
  rb_define_alloc_func(cCanvas, shoes_canvas_alloc);
  rb_define_method(cCanvas, "nostroke", CASTHOOK(shoes_canvas_nostroke), 0);
  rb_define_method(cCanvas, "stroke", CASTHOOK(shoes_canvas_stroke), -1);
  rb_define_method(cCanvas, "strokewidth", CASTHOOK(shoes_canvas_strokewidth), 1);
  rb_define_method(cCanvas, "nofill", CASTHOOK(shoes_canvas_nofill), 0);
  rb_define_method(cCanvas, "fill", CASTHOOK(shoes_canvas_fill), -1);
  rb_define_method(cCanvas, "rect", CASTHOOK(shoes_canvas_rect), -1);
  rb_define_method(cCanvas, "oval", CASTHOOK(shoes_canvas_oval), 4);
  rb_define_method(cCanvas, "line", CASTHOOK(shoes_canvas_line), 4);
  rb_define_method(cCanvas, "arrow", CASTHOOK(shoes_canvas_arrow), 3);
  rb_define_method(cCanvas, "star", CASTHOOK(shoes_canvas_star), -1);
  rb_define_method(cCanvas, "markup", CASTHOOK(shoes_canvas_markup), -1);
  rb_define_method(cCanvas, "background", CASTHOOK(shoes_canvas_background), -1);
  rb_define_method(cCanvas, "image", CASTHOOK(shoes_canvas_image), -1);
  rb_define_method(cCanvas, "imagesize", CASTHOOK(shoes_canvas_imagesize), 1);
  rb_define_method(cCanvas, "path", CASTHOOK(shoes_canvas_path), -1);
  rb_define_method(cCanvas, "move_to", CASTHOOK(shoes_canvas_move_to), 2);
  rb_define_method(cCanvas, "line_to", CASTHOOK(shoes_canvas_line_to), 2);
  rb_define_method(cCanvas, "curve_to", CASTHOOK(shoes_canvas_curve_to), 6);
  rb_define_method(cCanvas, "transform", CASTHOOK(shoes_canvas_transform), 1);
  rb_define_method(cCanvas, "translate", CASTHOOK(shoes_canvas_translate), 2);
  rb_define_method(cCanvas, "rotate", CASTHOOK(shoes_canvas_rotate), 1);
  rb_define_method(cCanvas, "scale", CASTHOOK(shoes_canvas_scale), -1);
  rb_define_method(cCanvas, "skew", CASTHOOK(shoes_canvas_skew), -1);
  rb_define_method(cCanvas, "push", CASTHOOK(shoes_canvas_push), 0);
  rb_define_method(cCanvas, "pop", CASTHOOK(shoes_canvas_pop), 0);
  rb_define_method(cCanvas, "reset", CASTHOOK(shoes_canvas_reset), 0);
  rb_define_method(cCanvas, "button", CASTHOOK(shoes_canvas_button), -1);
  rb_define_method(cCanvas, "list_box", CASTHOOK(shoes_canvas_list_box), -1);
  rb_define_method(cCanvas, "edit_line", CASTHOOK(shoes_canvas_edit_line), -1);
  rb_define_method(cCanvas, "edit_box", CASTHOOK(shoes_canvas_edit_box), -1);
  rb_define_method(cCanvas, "progress", CASTHOOK(shoes_canvas_progress), -1);
  rb_define_method(cCanvas, "contents", CASTHOOK(shoes_canvas_contents), 0);
  rb_define_method(cCanvas, "draw", CASTHOOK(shoes_canvas_draw), 2);
  rb_define_method(cCanvas, "flow", CASTHOOK(shoes_canvas_flow), -1);
  rb_define_method(cCanvas, "stack", CASTHOOK(shoes_canvas_stack), -1);
  rb_define_method(cCanvas, "hide", CASTHOOK(shoes_canvas_hide), 0);
  rb_define_method(cCanvas, "show", CASTHOOK(shoes_canvas_show), 0);
  rb_define_method(cCanvas, "toggle", CASTHOOK(shoes_canvas_toggle), 0);
  rb_define_method(cCanvas, "click", CASTHOOK(shoes_canvas_click), -1);
  rb_define_method(cCanvas, "release", CASTHOOK(shoes_canvas_release), -1);
  rb_define_method(cCanvas, "motion", CASTHOOK(shoes_canvas_motion), -1);
  rb_define_method(cCanvas, "keypress", CASTHOOK(shoes_canvas_keypress), -1);
  rb_define_method(cCanvas, "quit", CASTHOOK(shoes_app_quit), 0);

  cFlow    = rb_define_class_under(cCanvas, "Flow", cCanvas);
  cStack   = rb_define_class_under(cCanvas, "Stack", cCanvas);

  cPath    = rb_define_class_under(cCanvas, "Path", rb_cObject);
  rb_define_alloc_func(cPath, shoes_path_alloc);
  rb_define_method(cPath, "draw", CASTHOOK(shoes_path_draw), 2);
  rb_define_method(cPath, "move", CASTHOOK(shoes_path_move), 2);

  cImage    = rb_define_class_under(cCanvas, "Image", rb_cObject);
  rb_define_alloc_func(cImage, shoes_image_alloc);
  rb_define_method(cImage, "draw", CASTHOOK(shoes_image_draw), 2);
  cBackground = rb_define_class_under(cCanvas, "Background", rb_cObject);
  rb_define_alloc_func(cBackground, shoes_image_alloc);
  rb_define_method(cBackground, "draw", CASTHOOK(shoes_background_draw), 2);
  cTextClass = rb_define_class_under(cCanvas, "Text", rb_cObject);
  rb_define_alloc_func(cTextClass, shoes_text_alloc);
  rb_define_method(cTextClass, "draw", CASTHOOK(shoes_text_draw), 2);
  rb_define_method(cTextClass, "markup", CASTHOOK(shoes_text_get_markup), 0);
  rb_define_method(cTextClass, "markup=", CASTHOOK(shoes_text_set_markup), 1);

  cButton  = rb_define_class_under(cCanvas, "Button", rb_cObject);
  rb_define_alloc_func(cButton, shoes_control_alloc);
  rb_define_method(cButton, "draw", CASTHOOK(shoes_button_draw), 2);
  cEditLine  = rb_define_class_under(cCanvas, "EditLine", rb_cObject);
  rb_define_alloc_func(cEditLine, shoes_control_alloc);
  rb_define_method(cEditLine, "draw", CASTHOOK(shoes_edit_line_draw), 2);
  cEditBox  = rb_define_class_under(cCanvas, "EditBox", rb_cObject);
  rb_define_alloc_func(cEditBox, shoes_control_alloc);
  rb_define_method(cEditBox, "draw", CASTHOOK(shoes_edit_box_draw), 2);
  cListBox  = rb_define_class_under(cCanvas, "ListBox", rb_cObject);
  rb_define_alloc_func(cListBox, shoes_control_alloc);
  rb_define_method(cListBox, "draw", CASTHOOK(shoes_list_box_draw), 2);
  cProgress  = rb_define_class_under(cCanvas, "Progress", rb_cObject);
  rb_define_alloc_func(cProgress, shoes_control_alloc);
  rb_define_method(cProgress, "draw", CASTHOOK(shoes_progress_draw), 2);

  cColor   = rb_define_class_under(cCanvas, "Color", rb_cObject);
  cLink    = rb_define_class_under(cCanvas, "Link", rb_cObject);

  rb_define_method(rb_mKernel, "ask", CASTHOOK(shoes_dialog_ask), 1);
  rb_define_method(rb_mKernel, "confirm", CASTHOOK(shoes_dialog_confirm), 1);
  rb_define_method(rb_mKernel, "ask_color", CASTHOOK(shoes_dialog_color), 1);
  rb_define_method(rb_mKernel, "ask_open_file", CASTHOOK(shoes_dialog_open), 0);
  rb_define_method(rb_mKernel, "ask_save_file", CASTHOOK(shoes_dialog_save), 0);
}
