#ifndef SHOES_GTK_TIMERBASE_H
#define SHOES_GTK_TIMERBASE_H

SHOES_TIMER_REF shoes_native_timer_start(VALUE self, shoes_canvas *canvas, unsigned int interval);
void shoes_native_timer_remove(shoes_canvas *canvas, SHOES_TIMER_REF ref);
  
#endif