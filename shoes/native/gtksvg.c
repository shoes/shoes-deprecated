
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

gboolean
shoes_native_svg_draw_event(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  guint width, height;
  GdkRGBA color;

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);
  cairo_arc (cr,
             width / 2.0, height / 2.0,
             MIN (width, height) / 2.0,
             0, 2 * G_PI);

  gtk_style_context_get_color (gtk_widget_get_style_context (widget),
                               0,
                               &color);
  gdk_cairo_set_source_rgba (cr, &color);

  cairo_fill (cr);
  printf("draw event\n");
 return FALSE;
}

gboolean
shoes_native_svg_draw_handle(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  shoes_svg *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(data, shoes_svg, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  cairo_scale(cr, (self_t->place.w * 1.0) / self_t->svgdim.width, 
    (self_t->place.h * 1.0) / self_t->svgdim.height);
  rsvg_handle_render_cairo(self_t->handle, cr);
  printf("draw handle\n");
 return FALSE;
}

SHOES_SURFACE_REF
shoes_native_svg_new(shoes_canvas *canvas, VALUE self, shoes_place *place)
{
  GtkWidget *drawing_area = gtk_drawing_area_new();
  gtk_widget_set_size_request (drawing_area, 200, 200);
  g_signal_connect (G_OBJECT (drawing_area), "draw",
                    G_CALLBACK (shoes_native_svg_draw_handle), (gpointer)self);
  return drawing_area;
}

void
shoes_native_svg_position(SHOES_SURFACE_REF ref, shoes_place *p1,
  VALUE self, shoes_canvas *canvas, shoes_place *p2)
{
  shoes_native_control_position(ref, p1, self, canvas, p2);
}

void
shoes_native_svg_hide(SHOES_SURFACE_REF ref)
{
  shoes_native_control_hide(ref);
}

void
shoes_native_svg_show(SHOES_SURFACE_REF ref)
{
  shoes_native_control_show(ref);
}

void
shoes_native_svg_remove(shoes_canvas *canvas, SHOES_SURFACE_REF ref)
{
  gtk_container_remove(GTK_CONTAINER(canvas->slot->oscanvas), ref);
}

