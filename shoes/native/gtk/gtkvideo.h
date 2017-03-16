#include "shoes/config.h"
#include "shoes/canvas.h"

#ifndef SHOES_GTK_VIDEO_H
#define SHOES_GTK_VIDEO_H

SHOES_CONTROL_REF shoes_native_surface_new(VALUE, VALUE);
unsigned long shoes_native_surface_get_window_handle(SHOES_CONTROL_REF);
void shoes_native_surface_position(SHOES_SURFACE_REF, shoes_place *,
                                   VALUE, shoes_canvas *, shoes_place *);
void shoes_native_surface_hide(SHOES_SURFACE_REF);
void shoes_native_surface_show(SHOES_SURFACE_REF);
void shoes_native_surface_remove(SHOES_SURFACE_REF);

#endif