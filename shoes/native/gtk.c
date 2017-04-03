//
// shoes/native-gtk.c
// GTK+ code for Shoes.
//   Modified for Gtk-3.0 by Cecil Coupe (cjc)
//
#ifndef GTK3
// fail only used for shoes_native_window_color will be deleted
#define GTK3
#endif
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/types/text.h"
#include "shoes/types/text_link.h"
#include "shoes/types/download.h"
#include "shoes/internal.h"

#include <fontconfig/fontconfig.h>
#ifndef RUBY_HTTP
#ifndef SHOES_GTK_WIN32
#include <curl/curl.h>
#endif
#endif
#include <pthread.h>
#include <glib/gprintf.h>

#include "gtk.h"
#include "shoes/native/gtk/gtkfixedalt.h"
#include "shoes/native/gtk/gtkentryalt.h"
#include "shoes/native/gtk/gtkcomboboxtextalt.h"
#include "shoes/native/gtk/gtkbuttonalt.h"
#include "shoes/native/gtk/gtkscrolledwindowalt.h"
#include "shoes/native/gtk/gtkprogressbaralt.h"

#define HEIGHT_PAD 0

#define SHOES_GTK_INVISIBLE_CHAR (gunichar)0x2022

#ifndef SHOES_GTK_WIN32
static VALUE shoes_make_font_list(FcFontSet *fonts, VALUE ary) {
    int i = 0;
    for (i = 0; i < fonts->nfont; i++) {
        FcValue val;
        FcPattern *p = fonts->fonts[i];
        if (FcPatternGet(p, FC_FAMILY, 0, &val) == FcResultMatch)
            rb_ary_push(ary, rb_str_new2((char *)val.u.s));
    }
    rb_funcall(ary, rb_intern("uniq!"), 0);
    rb_funcall(ary, rb_intern("sort!"), 0);
    return ary;
}

VALUE shoes_font_list() {
    VALUE ary = rb_ary_new();
    FcConfig *fc = FcConfigGetCurrent();
    FcFontSet *fonts = FcConfigGetFonts(fc, FcSetApplication);
    if (fonts) shoes_make_font_list(fonts, ary);
    fonts = FcConfigGetFonts(fc, FcSetSystem);
    if (fonts) shoes_make_font_list(fonts, ary);
    return ary;
}

VALUE shoes_load_font(const char *filename) {
    FcConfig *fc = FcConfigGetCurrent();
    FcFontSet *fonts = FcFontSetCreate();
    if (!FcFileScan(fonts, NULL, NULL, NULL, (const FcChar8 *)filename, FcTrue))
        return Qnil;

    VALUE ary = rb_ary_new();
    shoes_make_font_list(fonts, ary);
    FcFontSetDestroy(fonts);

    if (!FcConfigAppFontAddFile(fc, (const FcChar8 *)filename))
        return Qnil;

    // refresh the FONTS list
    shoes_update_fonts(shoes_font_list());
    return ary;
}
#endif

#if 0
// FIXME: experiment with font settings
// Borrowed from http://ricardo.ecn.wfu.edu/~cottrell/gtk_win32/
#ifdef G_OS_WIN32
static char appfontname[128] = "sans-serif 12"; /* fallback value */
#else
static char appfontname[128] = "Sans-Serif 10";  // gtk doc says 'Sans 10'
#endif

void set_app_font (const char *fontname) {
    GtkSettings *settings;
    gchar *themedir;
    gchar *themename;

    if (fontname != NULL && *fontname == 0) return;

    settings = gtk_settings_get_default();
    themedir = gtk_rc_get_theme_dir ();
    g_object_get(settings, "gtk-theme-name", &themename, NULL);
    printf("dir=%s, name: %s\n", themedir,themename);


    if (fontname == NULL) {
        g_object_set(G_OBJECT(settings), "gtk-font-name", appfontname, NULL);
    } else {
        GtkWidget *w;
        PangoFontDescription *pfd;
        PangoContext *pc;
        PangoFont *pfont;

        w = gtk_label_new(NULL);
        pfd = pango_font_description_from_string(fontname);
        pc = gtk_widget_get_pango_context(w);
        pfont = pango_context_load_font(pc, pfd);

        if (pfont != NULL) {
            strcpy(appfontname, fontname);
            g_object_set(G_OBJECT(settings), "gtk-font-name", appfontname, NULL);
        }

        gtk_widget_destroy(w);
        pango_font_description_free(pfd);
    }
}

void shoes_native_print_env() {
    GtkSettings *settings;
    gchar *themedir;
    gchar *themename;

    gchar *rcfile = "shoesgtk.rc";
    //gchar *rcfiles[] = {rcfile, NULL};
    gchar **defs;
    //gtk_rc_set_default_files(rcfiles);
    //gtk_rc_parse(rcfile);
    //printf("ask theme = %s\n", rcfile);
    defs = gtk_rc_get_default_files ();
    int i = 0;
    while ((rcfile = defs[i]) != NULL) {
        printf("%d: %s\n", i+1, rcfile);
        i++;
    }
    settings = gtk_settings_get_default();
    themedir = gtk_rc_get_theme_dir ();
    g_object_get(settings, "gtk-theme-name", &themename, NULL);
    printf("dir=%s, name: %s\n", themedir,themename);
}
#endif

void shoes_native_init() {
#if !defined(RUBY_HTTP) && !defined(SHOES_GTK_WIN32)
    curl_global_init(CURL_GLOBAL_ALL);
#endif
    gtk_init(NULL, NULL);
    //set_app_font(NULL);  // experiment failed
    //shoes_native_print_env();
}

void shoes_native_cleanup(shoes_world_t *world) {
#if !defined(RUBY_HTTP) && !defined(SHOES_GTK_WIN32)
    curl_global_cleanup();
#endif
}

void shoes_native_quit() {
    gtk_main_quit();
}


#ifdef SHOES_GTK_WIN32
int shoes_win32_cmdvector(const char *cmdline, char ***argv) {
//  return rb_w32_cmdvector(cmdline, argv);
    return 0; // TODO: delete this function.
}

void shoes_get_time(SHOES_TIME *ts) {
    *ts = g_get_monotonic_time();  // Should work for GTK3 w/o win32
}

unsigned long shoes_diff_time(SHOES_TIME *start, SHOES_TIME *end) {
    return *end - *start;
}
#else
void shoes_get_time(SHOES_TIME *ts) {
#ifdef SHOES_GTK_OSX
    gettimeofday(ts, NULL);
#else
    clock_gettime(CLOCK_REALTIME, ts);
#endif
}

unsigned long shoes_diff_time(SHOES_TIME *start, SHOES_TIME *end) {
    unsigned long usec;
    if ((end->tv_nsec-start->tv_nsec)<0) {
        usec = (end->tv_sec-start->tv_sec - 1) * 1000;
        usec += (1000000000 + end->tv_nsec - start->tv_nsec) / 1000000;
    } else {
        usec = (end->tv_sec - start->tv_sec) * 1000;
        usec += (end->tv_nsec - start->tv_nsec) / 1000000;
    }
    return usec;
}
#endif

typedef struct {
    unsigned int name;
    VALUE obj;
    void *data;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int ret;
} shoes_gtk_msg;


static gboolean shoes_gtk_catch_message(gpointer user) {
    shoes_gtk_msg *msg = (shoes_gtk_msg *)user;
    pthread_mutex_lock(&msg->mutex);
    msg->ret = shoes_catch_message(msg->name, msg->obj, msg->data);
    pthread_cond_signal(&msg->cond);
    pthread_mutex_unlock(&msg->mutex);
    return FALSE;
}

// Only called by image.c
int shoes_throw_message(unsigned int name, VALUE obj, void *data) {
    int ret;
    shoes_gtk_msg *msg = SHOE_ALLOC(shoes_gtk_msg);
    msg->name = name;
    msg->obj = obj;
    msg->data = data;
    pthread_mutex_init(&msg->mutex, NULL);
    pthread_cond_init(&msg->cond, NULL);
    msg->ret = 0;

    pthread_mutex_lock(&msg->mutex);
    g_idle_add_full(G_PRIORITY_DEFAULT, shoes_gtk_catch_message, msg, NULL);
    pthread_cond_wait(&msg->cond, &msg->mutex);
    ret = msg->ret;
    pthread_mutex_unlock(&msg->mutex);

    free(msg);
    return ret;
}

void shoes_native_slot_mark(SHOES_SLOT_OS *slot) {}
void shoes_native_slot_reset(SHOES_SLOT_OS *slot) {}
void shoes_native_slot_clear(shoes_canvas *canvas) {
    if (canvas->slot->vscroll) {
        GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
        gtk_adjustment_set_value(adj, gtk_adjustment_get_lower(adj));
    }
}

void shoes_native_slot_paint(SHOES_SLOT_OS *slot) {
    gtk_widget_queue_draw(slot->oscanvas);
}

void shoes_native_slot_lengthen(SHOES_SLOT_OS *slot, int height, int endy) {
    if (slot->vscroll) {
        GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(slot->vscroll));
        if (gtk_adjustment_get_upper(adj) != (gdouble)endy) {
            gtk_range_set_range(GTK_RANGE(slot->vscroll), 0., (gdouble)endy);

            if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
                gtk_widget_hide(slot->vscroll);
            else
                gtk_widget_show(slot->vscroll);
        }
    }
}

void shoes_native_slot_scroll_top(SHOES_SLOT_OS *slot) {
    if (slot->vscroll)
        gtk_range_set_value(GTK_RANGE(slot->vscroll), slot->scrolly);
}

int shoes_native_slot_gutter(SHOES_SLOT_OS *slot) {
    if (slot->vscroll) {
        GtkRequisition rnat;
        gtk_widget_get_preferred_size(slot->vscroll, NULL, &rnat);
        return rnat.width;
    }
    return 0;
}

void shoes_native_remove_item(SHOES_SLOT_OS *slot, VALUE item, char c) {
}

//
// Window-level events
//
static gboolean shoes_app_gtk_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    GdkModifierType state;
    shoes_app *app = (shoes_app *)data;
    if (!event->is_hint) {
        shoes_canvas *canvas;
        Data_Get_Struct(app->canvas, shoes_canvas, canvas);
        state = (GdkModifierType)event->state;
        shoes_app_motion(app, (int)event->x, (int)event->y + canvas->slot->scrolly);
    }
    return TRUE;
}

static gboolean shoes_app_gtk_button(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    shoes_app *app = (shoes_app *)data;
    shoes_canvas *canvas;
    Data_Get_Struct(app->canvas, shoes_canvas, canvas);
    if (event->type == GDK_BUTTON_PRESS) {
        shoes_app_click(app, event->button, event->x, event->y + canvas->slot->scrolly);
    } else if (event->type == GDK_BUTTON_RELEASE) {
        shoes_app_release(app, event->button, event->x, event->y + canvas->slot->scrolly);
    }
    return TRUE;
}

static gboolean shoes_app_gtk_wheel(GtkWidget *widget, GdkEventScroll *event, gpointer data) {
    ID wheel;
    shoes_app *app = (shoes_app *)data;

    switch (event->direction) {
        case GDK_SCROLL_UP:
            wheel = s_up;
            break;
        case GDK_SCROLL_DOWN:
            wheel = s_down;
            break;
        case GDK_SCROLL_LEFT:
            wheel = s_left;
            break;
        case GDK_SCROLL_RIGHT:
            wheel = s_right;
            break;
        default:
            return TRUE;
    }

    shoes_app_wheel(app, wheel, event->x, event->y);
    return TRUE;
}

static void shoes_app_gtk_paint(GtkWidget *widget, cairo_t *cr, gpointer data) {
    shoes_app *app = (shoes_app *)data;
    gtk_window_get_size(GTK_WINDOW(app->os.window), &app->width, &app->height);
    shoes_canvas_size(app->canvas, app->width, app->height);
}

#define KEY_SYM(name, sym) \
  else if (event->keyval == GDK_KEY_##name) \
    v = ID2SYM(rb_intern("" # sym))

static gboolean shoes_app_gtk_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    VALUE v = Qnil;
    guint modifiers = event->state;
    shoes_app *app = (shoes_app *)data;
    if (event->keyval == GDK_KEY_Return) {
        v = rb_str_new2("\n");
    }
    KEY_SYM(BackSpace, backspace);	// GTK3 <bs> has length of 1. Go figure.
    KEY_SYM(Escape, escape);
    else if (event->length > 0) {
        if ((event->state & GDK_CONTROL_MASK) || (event->state & GDK_MOD1_MASK)) {
            gint len;
            gunichar ch;
            char chbuf[7] = {0};

            ch = gdk_keyval_to_unicode(event->keyval);
            len = g_unichar_to_utf8(ch, chbuf);
            chbuf[len] = '\0';

            v = ID2SYM(rb_intern(chbuf));
            if (modifiers & GDK_SHIFT_MASK) modifiers ^= GDK_SHIFT_MASK;
        } else {
            if (event->string[0] == '\r' && event->length == 1)
                v = rb_str_new2("\n");
            else
                v = rb_str_new(event->string, event->length);
        }
    }
    KEY_SYM(Insert, insert);
    KEY_SYM(Delete, delete);
    KEY_SYM(Tab, tab);
    KEY_SYM(ISO_Left_Tab, tab);
    KEY_SYM(Page_Up, page_up);
    KEY_SYM(Page_Down, page_down);
    KEY_SYM(Home, home);
    KEY_SYM(End, end);
    KEY_SYM(Left, left);
    KEY_SYM(Up, up);
    KEY_SYM(Right, right);
    KEY_SYM(Down, down);
    KEY_SYM(F1, f1);
    KEY_SYM(F2, f2);
    KEY_SYM(F3, f3);
    KEY_SYM(F4, f4);
    KEY_SYM(F5, f5);
    KEY_SYM(F6, f6);
    KEY_SYM(F7, f7);
    KEY_SYM(F8, f8);
    KEY_SYM(F9, f9);
    KEY_SYM(F10, f10);
    KEY_SYM(F11, f11);
    KEY_SYM(F12, f12);

    if (v != Qnil) {
        if (event->type == GDK_KEY_PRESS) {
            shoes_app_keydown(app, v);

            if (event->keyval == GDK_KEY_Return)
                if ((event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0)
                    v = ID2SYM(rb_intern("enter"));

            if (SYMBOL_P(v)) {
                if (modifiers & GDK_MOD1_MASK)
                    KEY_STATE(alt);
                if (modifiers & GDK_SHIFT_MASK)
                    KEY_STATE(shift);
                if (modifiers & GDK_CONTROL_MASK)
                    KEY_STATE(control);
            }

            shoes_app_keypress(app, v);
        } else {
            shoes_app_keyup(app, v);
        }
    }

    return FALSE;
}

static gboolean shoes_app_gtk_quit(GtkWidget *widget, GdkEvent *event, gpointer data) {
    shoes_app *app = (shoes_app *)data;
    if (shoes_app_remove(app))
        gtk_main_quit();
    return FALSE;
}

static void shoes_canvas_gtk_paint_children(GtkWidget *widget, gpointer data) {
    shoes_canvas *canvas = (shoes_canvas *)data;
    gtk_container_propagate_draw(GTK_CONTAINER(canvas->slot->oscanvas), widget,
                                 canvas->slot->drawevent);
}

static void shoes_canvas_gtk_paint(GtkWidget *widget, cairo_t *cr, gpointer data) {
    VALUE c = (VALUE)data;
    shoes_canvas *canvas;
    Data_Get_Struct(c, shoes_canvas, canvas);
    canvas->slot->drawevent = cr;		// stash it for the children

    // getting widget dirty area, already clipped
    cairo_rectangle_int_t rect;
    gdk_cairo_get_clip_rectangle(cr, &rect);

    shoes_canvas_paint(c);
    // Gtk3 doc says gtk_container_foreach is preferable over gtk_container_forall
    gtk_container_foreach(GTK_CONTAINER(widget), shoes_canvas_gtk_paint_children, canvas);

    canvas->slot->drawevent = NULL;
}

static void shoes_canvas_gtk_size(GtkWidget *widget, GtkAllocation *size, gpointer data) {
    VALUE c = (VALUE)data;
    shoes_canvas *canvas;
    Data_Get_Struct(c, shoes_canvas, canvas);
    if (canvas->slot->vscroll &&
            (size->height != canvas->slot->scrollh || size->width != canvas->slot->scrollw)) {
        GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
        gtk_widget_set_size_request(canvas->slot->vscroll, -1, size->height);

        //gtk_widget_set_size_request(GTK_CONTAINER(widget), canvas->app->width, size->height);

        GtkAllocation alloc;
        gtk_widget_get_allocation((GtkWidget *)canvas->slot->vscroll, &alloc);
        gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas), canvas->slot->vscroll,
                       size->width - alloc.width, 0);
        gtk_adjustment_set_page_size(adj, size->height);
        gtk_adjustment_set_page_increment(adj, size->height - 32);

        if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
            gtk_widget_hide(canvas->slot->vscroll);
        else
            gtk_widget_show(canvas->slot->vscroll);

        canvas->slot->scrollh = size->height;
        canvas->slot->scrollw = size->width;
    }
}

static void shoes_canvas_gtk_scroll(GtkRange *r, gpointer data) {
    VALUE c = (VALUE)data;
    shoes_canvas *canvas;
    Data_Get_Struct(c, shoes_canvas, canvas);
    canvas->slot->scrolly = (int)gtk_range_get_value(r);
    shoes_slot_repaint(canvas->app->slot);
}

// TODO:  remove dead code? warning: 'shoes_app_g_poll' defined but not used
#ifndef SHOES_GTK_WIN32
static gint shoes_app_g_poll(GPollFD *fds, guint nfds, gint timeout) {
    struct timeval tv;

    rb_fdset_t rset, wset, xset;
    GPollFD *f;
    int ready;
    int maxfd = 0;

    rb_fd_init(&rset); // was FD_ZERO()
    rb_fd_init(&wset);
    rb_fd_init(&xset);

    for (f = fds; f < &fds[nfds]; ++f)
        if (f->fd >= 0) {
            if (f->events & G_IO_IN)
                //FD_SET (f->fd, &rset);
                rb_fd_set(f->fd, &rset);
            if (f->events & G_IO_OUT)
                //FD_SET (f->fd, &wset);
                rb_fd_set(f->fd, &wset);
            if (f->events & G_IO_PRI)
                //FD_SET (f->fd, &xset);
                rb_fd_set(f->fd, &xset);
            if (f->fd > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
                maxfd = f->fd;
        }

    //
    // If we poll indefinitely, then the window updates will
    // pile up for as long as Ruby is churning away.
    //
    // Give Ruby half-seconds in which to work, in order to
    // keep it from completely blocking the GUI.
    //
    // cjc: timeout appears to be in millisecs.
    if (timeout == -1 || timeout > 500)
        timeout = 500;

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    ready = rb_thread_fd_select (maxfd + 1, &rset, &wset, &xset, &tv);
    if (ready > 0)
        // I think the idea here is that the Ruby threads might cause Gtk events
        // and that's supposed to pass back in the f->revents
        // Does Ruby/Windows execute this code more often or incorrectly?
        for (f = fds; f < &fds[nfds]; ++f) {
            f->revents = 0;
            if (f->fd >= 0) {
                //if (FD_ISSET (f->fd, &rset))
                if (rb_fd_isset (f->fd, &rset))
                    f->revents |= G_IO_IN;
                //if (FD_ISSET (f->fd, &wset))
                if (rb_fd_isset (f->fd, &wset))
                    f->revents |= G_IO_OUT;
                //if (FD_ISSET (f->fd, &xset))
                if (rb_fd_isset (f->fd, &xset))
                    f->revents |= G_IO_PRI;
            }
        }
    // Free the allocated storage from rb_fd_init
    rb_fd_term(&rset);
    rb_fd_term(&wset);
    rb_fd_term(&xset);
    return ready;
}
#else
/*
 * Fake a rb_fd to always have something to read
 * Ruby run select
 * if nothing selected (only the fake is returned) then
 *   run a poll/select on the Gtk fds
 *   and return that number.
 */
/*
static gint shoes_app_g_poll(GPollFD *fds, guint nfds, gint timeout) {
    struct timeval tv;
    rb_fdset_t rset, wset, xset; //ruby
    GPollFD *f;
    int ready;
    int maxfd = 0;

    rb_fd_init(&rset); // These are ruby rd (sets
    rb_fd_init(&wset);
    rb_fd_init(&xset);

    int i;
    // On Windows GTK GPollFD.fd could be a handle, not a small int.
    for (i = 0; i < nfds; i++) {
        f = &fds[i];
        if (f->fd >= 0) {
            if (f->events & G_IO_IN)
                rb_fd_set(f->fd, &rset);
            if (f->events & G_IO_OUT)
                rb_fd_set (f->fd, &wset);
            if (f->events & G_IO_PRI)
                rb_fd_set (f->fd, &xset);
            // if (f->fd > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
            //   maxfd = f->fd;
            if (i > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
                maxfd = i;
        }
    }
    //
    // If we poll indefinitely, then the window updates will
    // pile up for as long as Ruby is churning away.
    //
    // Give Ruby half-seconds in which to work, in order to
    // keep it from completely blocking the GUI.
    //
    if (timeout == -1 || timeout > 500)
        timeout = 500;

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    ready = rb_fd_select (maxfd + 1, &rset, &wset, &xset, &tv); // NOT deprecated in 2.1.0
    int really_ready = 0;
    if (ready == 0) {
        for (f = fds; f < &fds[nfds]; ++f) {
            f->revents = 0;
            if (f->fd >= 0) {
                if (rb_fd_isset (f->fd, &rset))
                    f->revents |= G_IO_IN;
                if (rb_fd_isset (f->fd, &wset))
                    f->revents |= G_IO_OUT;
                if (rb_fd_isset (f->fd, &xset))
                    f->revents |= G_IO_PRI;
                if (f->revents > 0) really_ready++;
            }
        }
    }

    rb_fd_term(&rset);
    rb_fd_term(&wset);
    rb_fd_term(&xset);
    if (ready < 0) {
        printf("Poll fail %d\n", errno);
        ready = 0;
    }
    if (ready != really_ready) {
        printf("R %d. RR %d\n", ready, really_ready);
        ready = really_ready;
    }
    return ready;
}
*/
#endif

shoes_code shoes_app_cursor(shoes_app *app, ID cursor) {
    if (app->os.window == NULL || gtk_widget_get_window(app->os.window)== NULL || app->cursor == cursor)
        goto done;

    GdkCursor *c;
    if (cursor == s_hand || cursor == s_link) {
        c = gdk_cursor_new(GDK_HAND2);
    } else if (cursor == s_arrow) {
        c = gdk_cursor_new(GDK_ARROW);
    } else if (cursor == s_text) {
        c = gdk_cursor_new(GDK_XTERM);
    } else
        goto done;

    gdk_window_set_cursor(gtk_widget_get_window(app->os.window), c);
    app->cursor = cursor;

done:
    return SHOES_OK;
}

void shoes_native_app_resized(shoes_app *app) {
    // Not needed anymore ?
    //  if (app->os.window != NULL)
    //    gtk_widget_set_size_request(app->os.window, app->width, app->height);
}

void shoes_native_app_title(shoes_app *app, char *msg) {
    gtk_window_set_title(GTK_WINDOW(app->os.window), _(msg));
}

void shoes_native_app_fullscreen(shoes_app *app, char yn) {
    gtk_window_set_keep_above(GTK_WINDOW(app->os.window), (gboolean)yn);
    if (yn)
        gtk_window_fullscreen(GTK_WINDOW(app->os.window));
    else
        gtk_window_unfullscreen(GTK_WINDOW(app->os.window));
}

// new in 3.2.19
void shoes_native_app_set_icon(shoes_app *app, char *icon_path) {
    // replace default icon
    gboolean err;
    err = gtk_window_set_icon_from_file((GtkWindow *) app->slot->oscanvas, icon_path, NULL);
    err = gtk_window_set_default_icon_from_file(icon_path, NULL);
}

// new in 3.2.19
void shoes_native_app_set_wtitle(shoes_app *app, char *wtitle) {
    gtk_window_set_title(GTK_WINDOW(app->slot->oscanvas), _(wtitle));
}


void shoes_native_app_set_opacity(shoes_app *app, double opacity) {
#if GTK_CHECK_VERSION(3,8,0)
    gtk_widget_set_opacity(GTK_WIDGET(app->os.window), opacity);
#endif
}

double shoes_native_app_get_opacity(shoes_app *app) {
#if GTK_CHECK_VERSION(3,8,0)
    return gtk_widget_get_opacity(GTK_WIDGET(app->os.window));
#else 
    return 1.0;
#endif 
}


void shoes_native_app_set_decoration(shoes_app *app, gboolean decorated) {
    gtk_window_set_decorated(GTK_WINDOW(app->os.window), decorated);
}

gboolean shoes_native_app_get_decoration(shoes_app *app) {
    return gtk_window_get_decorated(GTK_WINDOW(app->os.window));
}

shoes_code shoes_native_app_open(shoes_app *app, char *path, int dialog) {
#if !defined(SHOES_GTK_WIN32)
    char icon_path[SHOES_BUFSIZE];
#endif

    shoes_app_gtk *gk = &app->os;

#if !defined(SHOES_GTK_WIN32)
    sprintf(icon_path, "%s/static/app-icon.png", shoes_world->path);
    gtk_window_set_default_icon_from_file(icon_path, NULL);
#endif
    gk->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(gk->window), GTK_WIN_POS_CENTER);
    // commit https://github.com/shoes/shoes/commit/4e7982ddcc8713298b6959804dab8d20111c0038
    if (!app->resizable) {
        gtk_widget_set_size_request(gk->window, app->width, app->height);
        gtk_window_set_resizable(GTK_WINDOW(gk->window), FALSE);
    } else if (app->minwidth < app->width || app->minheight < app->height) {
        GdkGeometry hints;
        hints.min_width = app->minwidth;
        hints.min_height = app->minheight;
        gtk_window_set_geometry_hints(GTK_WINDOW(gk->window), gk->window,
                                      &hints, GDK_HINT_MIN_SIZE);
    }
    gtk_window_set_default_size(GTK_WINDOW(gk->window), app->width, app->height);

    if (app->fullscreen) shoes_native_app_fullscreen(app, 1);

    gtk_window_set_decorated(GTK_WINDOW(gk->window), app->decorated);
#if GTK_CHECK_VERSION(3,8,0)
    gtk_widget_set_opacity(GTK_WIDGET(gk->window), app->opacity);
#endif
    gtk_widget_set_events(gk->window, GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    g_signal_connect(G_OBJECT(gk->window), "size-allocate",
                     G_CALLBACK(shoes_app_gtk_paint), app);
    g_signal_connect(G_OBJECT(gk->window), "motion-notify-event",
                     G_CALLBACK(shoes_app_gtk_motion), app);
    g_signal_connect(G_OBJECT(gk->window), "button-press-event",
                     G_CALLBACK(shoes_app_gtk_button), app);
    g_signal_connect(G_OBJECT(gk->window), "button-release-event",
                     G_CALLBACK(shoes_app_gtk_button), app);
    g_signal_connect(G_OBJECT(gk->window), "scroll-event",
                     G_CALLBACK(shoes_app_gtk_wheel), app);
    g_signal_connect(G_OBJECT(gk->window), "key-press-event",
                     G_CALLBACK(shoes_app_gtk_keypress), app);
    g_signal_connect(G_OBJECT(gk->window), "key-release-event",
                     G_CALLBACK(shoes_app_gtk_keypress), app);
    g_signal_connect(G_OBJECT(gk->window), "delete-event",
                     G_CALLBACK(shoes_app_gtk_quit), app);

    app->slot->oscanvas = gk->window;
    return SHOES_OK;
}

void shoes_native_app_show(shoes_app *app) {
    gtk_widget_show_all(app->os.window);
}

#ifdef SHOES_GTK_WIN32
/*static GSource *gtkrb_source;
static GSource *gtkrb_init_source();
static  GSourceFuncs gtkrb_func_tbl;*/
/*
static GSource *gtkrb_init_source()
{
  // fill in the struct
  gtkrb_source = g_source_new(&gtkrb_func_tbl, (guint) sizeof(gtkrb_func_tbl));
}
*/

static gboolean gtkrb_idle() {
    rb_thread_schedule();
    return 1; // keep timeout
}
#endif

void shoes_native_loop() {
#ifndef SHOES_GTK_WIN32
    g_main_context_set_poll_func(g_main_context_default(), shoes_app_g_poll);
#else
    /* Win32 (should work for Linux too when finished)
     * Build: struct GSourceFuncs
     * Call: g_source_new() with that struct
     * Call: g_source_attach() with that
     */
    //gtkrb_source = gtkrb_init_source();
    //g_source_attach(gtkrb_source, (gpointer) NULL);
    //g_idle_add(gtkrb_idle, NULL);
    g_timeout_add(100, gtkrb_idle, NULL);
#endif
    GLOBAL_APP(app);
    if (APP_WINDOW(app)) gtk_main();
}

void shoes_native_app_close(shoes_app *app) {
    shoes_app_gtk_quit(app->os.window, NULL, (gpointer)app);
    gtk_widget_destroy(app->os.window);
    app->os.window = NULL;
}

// Below function doesn't work - /etc/alternatives doesn't exist.
// TODO: Appears to not be used at Shoe/ruby level
void shoes_browser_open(char *url) {
    VALUE browser = rb_str_new2("/etc/alternatives/x-www-browser '");
    rb_str_cat2(browser, url);
    rb_str_cat2(browser, "' 2>/dev/null &");
    shoes_sys(RSTRING_PTR(browser), 1);
}

void shoes_slot_init(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width, int height, int scrolls, int toplevel) {
    shoes_canvas *canvas;
    SHOES_SLOT_OS *slot;
    Data_Get_Struct(c, shoes_canvas, canvas);

    slot = shoes_slot_alloc(canvas, parent, toplevel);

    /* Subclassing GtkFixed so we can override gtk3 size management which creates
       problems with slot height being always tied to inside widgets cumulative heights
       creating heights overflow with no scrollbar !
    */
    slot->oscanvas = gtkfixed_alt_new();

    g_signal_connect(G_OBJECT(slot->oscanvas), "draw",
                     G_CALLBACK(shoes_canvas_gtk_paint), (gpointer)c);

    g_signal_connect(G_OBJECT(slot->oscanvas), "size-allocate",
                     G_CALLBACK(shoes_canvas_gtk_size), (gpointer)c);
    INFO("shoes_slot_init(%lu)\n", c);

    if (toplevel)
        gtk_container_add(GTK_CONTAINER(parent->oscanvas), slot->oscanvas);
    else
        gtk_fixed_put(GTK_FIXED(parent->oscanvas), slot->oscanvas, x, y);

    slot->scrollh = slot->scrollw = 0;
    slot->vscroll = NULL;
    if (scrolls) {
        slot->vscroll = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,NULL);
        GtkAdjustment *adj;
        adj = gtk_range_get_adjustment(GTK_RANGE(slot->vscroll));
        gtk_adjustment_set_step_increment(adj, 16);
        gtk_adjustment_set_page_increment(adj, height - 32);
        slot->drawevent = NULL;

        g_signal_connect(G_OBJECT(slot->vscroll), "value-changed",
                         G_CALLBACK(shoes_canvas_gtk_scroll), (gpointer)c);
        gtk_fixed_put(GTK_FIXED(slot->oscanvas), slot->vscroll, -100, -100);

        gtk_widget_set_size_request(slot->oscanvas, width, height);
        //gtk_widget_set_size_request(slot->oscanvas, canvas->app->minwidth, canvas->app->minheight);

        if (!toplevel) ATTRSET(canvas->attr, wheel, scrolls);
    }

    if (toplevel) shoes_canvas_size(c, width, height);
    else {
        gtk_widget_show_all(slot->oscanvas);
        canvas->width = 100;
        canvas->height = 100;
    }
}

void shoes_slot_destroy(shoes_canvas *canvas, shoes_canvas *pc) {
    if (canvas->slot->vscroll)
        gtk_container_remove(GTK_CONTAINER(canvas->slot->oscanvas), canvas->slot->vscroll);
    gtk_container_remove(GTK_CONTAINER(pc->slot->oscanvas), canvas->slot->oscanvas);
}

cairo_t *shoes_cairo_create(shoes_canvas *canvas) {
    GdkWindow *win = gtk_widget_get_window(canvas->slot->oscanvas);
    cairo_t *cr = gdk_cairo_create(win);
    if (canvas->slot->drawevent != NULL &&
            gtk_cairo_should_draw_window(canvas->slot->drawevent, win)) {
        cairo_rectangle_int_t alloc;
        gtk_widget_get_allocation((GtkWidget *)canvas->slot->oscanvas, &alloc);
        cairo_region_t *region = cairo_region_create_rectangle(&alloc);
//    cairo_rectangle_int_t w_rect = {canvas->place.ix, canvas->place.iy, canvas->place.w, canvas->place.h};
        cairo_rectangle_int_t w_rect;
        gdk_cairo_get_clip_rectangle(cr, &w_rect);
        cairo_region_intersect_rectangle(region, &w_rect);
        gdk_cairo_region(cr, region);
        cairo_clip(cr);
        cairo_region_destroy(region);

        cairo_translate(cr, alloc.x, alloc.y - canvas->slot->scrolly);
    }
    return cr;
}

void shoes_cairo_destroy(shoes_canvas *canvas) {
}

void shoes_group_clear(SHOES_GROUP_OS *group) {
    group->radios = NULL;
    group->layout = NULL;
}

void shoes_native_canvas_place(shoes_canvas *self_t, shoes_canvas *pc) {
    int x, y, newy;

    GtkAllocation a;
    gtk_widget_get_allocation(self_t->slot->oscanvas, &a);
    gtk_widget_translate_coordinates(self_t->slot->oscanvas, pc->slot->oscanvas, 0, 0, &x, &y);
    newy = (self_t->place.iy + self_t->place.dy) - pc->slot->scrolly;

    if (x != self_t->place.ix + self_t->place.dx || y != newy)
        gtk_fixed_move(GTK_FIXED(pc->slot->oscanvas), self_t->slot->oscanvas,
                       self_t->place.ix + self_t->place.dx, newy);
    if (a.width != self_t->place.iw || a.height != self_t->place.ih)
        gtk_widget_set_size_request(self_t->slot->oscanvas, self_t->place.iw, self_t->place.ih);
}

void shoes_native_canvas_resize(shoes_canvas *canvas) {
}

/*
 * one shot timer for start{} on slot. Canvas internal use.
*/
static gboolean start_wait(gpointer data) {
    VALUE rbcanvas = (VALUE)data;
    shoes_canvas *canvas;
    Data_Get_Struct(rbcanvas, shoes_canvas, canvas);

    shoes_safe_block(rbcanvas, ATTR(canvas->attr, start), rb_ary_new3(1, rbcanvas));
    return FALSE; // timeout will be stopped and destroyed
}

void shoes_native_canvas_oneshot(int ms, VALUE canvas) {
    g_timeout_add_full(G_PRIORITY_HIGH, 1, start_wait, (gpointer)canvas, NULL);
}

void shoes_widget_changed(GtkWidget *ref, gpointer data) {
    VALUE self = (VALUE)data;
    shoes_control_send(self, s_change);
}

void shoes_native_control_hide(SHOES_CONTROL_REF ref) {
    gtk_widget_hide(ref);
}

void shoes_native_control_show(SHOES_CONTROL_REF ref) {
    gtk_widget_show(ref);
}

void shoes_native_control_position(SHOES_CONTROL_REF ref, shoes_place *p1, VALUE self,
                              shoes_canvas *canvas, shoes_place *p2) {
    PLACE_COORDS();
    gtk_widget_set_size_request(ref, p2->iw, p2->ih);
    gtk_fixed_put(GTK_FIXED(canvas->slot->oscanvas), ref, p2->ix + p2->dx, p2->iy + p2->dy);
    gtk_widget_show_all(ref);
}

void shoes_native_control_repaint(SHOES_CONTROL_REF ref, shoes_place *p1,
                             shoes_canvas *canvas, shoes_place *p2) {
    p2->iy -= canvas->slot->scrolly;
    if (CHANGED_COORDS()) {
        PLACE_COORDS();
        gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas),
                       ref, p2->ix + p2->dx, p2->iy + p2->dy);
        gtk_widget_set_size_request(ref, p2->iw, p2->ih);
    }
    p2->iy += canvas->slot->scrolly;
}

void shoes_native_control_focus(SHOES_CONTROL_REF ref) {
    if (gtk_widget_get_can_focus(ref)) gtk_widget_grab_focus(ref);
}

void shoes_native_control_state(SHOES_CONTROL_REF ref, gboolean sensitive, gboolean setting) {
    gtk_widget_set_sensitive(ref, sensitive);
    if (GTK_IS_EDITABLE(ref))
        gtk_editable_set_editable(GTK_EDITABLE(ref), setting);
    else if (GTK_IS_SCROLLED_WINDOW(ref)) {
        GtkWidget *textview;
        GTK_CHILD(textview, ref);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), setting);
    }
}

void shoes_native_control_remove(SHOES_CONTROL_REF ref, shoes_canvas *canvas) {
    gtk_container_remove(GTK_CONTAINER(canvas->slot->oscanvas), ref);
}

void shoes_native_control_free(SHOES_CONTROL_REF ref) {
    //
    // no need to free gtk widgets, since gtk seems
    // to garbage collect them fine.  and memory
    // addresses often get reused.
    //
}

void shoes_native_control_set_tooltip(SHOES_CONTROL_REF ref, VALUE tooltip) {
    gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_native_to_s(tooltip)));
}

VALUE shoes_native_control_get_tooltip(SHOES_CONTROL_REF ref) {
    return rb_str_new2(gtk_widget_get_tooltip_text(GTK_WIDGET(ref)));
}

gboolean shoes_button_gtk_clicked(GtkButton *button, gpointer data) {
    VALUE self = (VALUE)data;
    shoes_control_send(self, s_click);
    return TRUE;
}

SHOES_CONTROL_REF shoes_native_button(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_button_alt_new_with_label(_(msg));

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    g_signal_connect(G_OBJECT(ref), "clicked",
                     G_CALLBACK(shoes_button_gtk_clicked),
                     (gpointer)self);

    return ref;
}

void shoes_native_secrecy(SHOES_CONTROL_REF ref) {
    gtk_entry_set_visibility(GTK_ENTRY(ref), FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(ref), SHOES_GTK_INVISIBLE_CHAR);
}

SHOES_CONTROL_REF shoes_native_list_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    /*get bottom margin : following macro gives us bmargin (also lmargin,tmargin,rmargin)*/
    ATTR_MARGINS(attr, 0, canvas);

    SHOES_CONTROL_REF ref = gtk_combo_box_text_alt_new(attr, bmargin);

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    g_signal_connect(G_OBJECT(ref), "changed",
                     G_CALLBACK(shoes_widget_changed),
                     (gpointer)self);
    return ref;
}

void shoes_native_list_box_update(SHOES_CONTROL_REF combo, VALUE ary) {
    long i;
    gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo))));
    for (i = 0; i < RARRAY_LEN(ary); i++) {
        VALUE msg = shoes_native_to_s(rb_ary_entry(ary, i));
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, _(RSTRING_PTR(msg)));
    }
}

VALUE shoes_native_list_box_get_active(SHOES_CONTROL_REF ref, VALUE items) {
    int sel = gtk_combo_box_get_active(GTK_COMBO_BOX(ref));
    if (sel >= 0)
        return rb_ary_entry(items, sel);
    return Qnil;
}

void shoes_native_list_box_set_active(SHOES_CONTROL_REF combo, VALUE ary, VALUE item) {
    int idx = rb_ary_index_of(ary, item);
    if (idx < 0) return;
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), idx);
}

// TODO: should be moved into gtkprogress.c (or gtkprogressbaralt.c)
SHOES_CONTROL_REF shoes_native_progress(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_progress_bar_alt_new();

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ref), _(msg));
    return ref;
}

double shoes_native_progress_get_fraction(SHOES_CONTROL_REF ref) {
    return gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(ref));
}

void shoes_native_progress_set_fraction(SHOES_CONTROL_REF ref, double perc) {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ref), perc);
}

SHOES_CONTROL_REF shoes_native_slider(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0., 1., 0.01);

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    gtk_scale_set_draw_value(GTK_SCALE(ref), FALSE);
    g_signal_connect(G_OBJECT(ref), "value-changed",
                     G_CALLBACK(shoes_widget_changed), (gpointer)self);
    return ref;
}

double shoes_native_slider_get_fraction(SHOES_CONTROL_REF ref) {
    return gtk_range_get_value(GTK_RANGE(ref));
}

void shoes_native_slider_set_fraction(SHOES_CONTROL_REF ref, double perc) {
    gtk_range_set_value(GTK_RANGE(ref), perc);
}

VALUE shoes_native_clipboard_get(shoes_app *app) {
    //GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    if (gtk_clipboard_wait_is_text_available(primary)) {
        gchar *string = gtk_clipboard_wait_for_text(primary);
        return rb_str_new2(string);
    }
    return Qnil;
}

void shoes_native_clipboard_set(shoes_app *app, VALUE string) {
    //GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(primary, RSTRING_PTR(string), RSTRING_LEN(string));
}

VALUE shoes_native_to_s(VALUE text) {
    text = rb_funcall(text, s_to_s, 0);
    return text;
}

#if defined(GTK3) // && !defined(SHOES_GTK_WIN32)
VALUE shoes_native_window_color(shoes_app *app) {
    GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(APP_WINDOW(app)));
    GdkRGBA bg;
    gtk_style_context_lookup_color(style, GTK_STATE_NORMAL, &bg);
    return shoes_color_new((int)(bg.red * 255), (int)(bg.green * 255),
                           (int)(bg.blue * 255), SHOES_COLOR_OPAQUE);
}

VALUE shoes_native_dialog_color(shoes_app *app) {
    GdkRGBA bg;
    GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(APP_WINDOW(app)));
    gtk_style_context_lookup_color(style, GTK_STATE_NORMAL, &bg);
    return shoes_color_new((int)(bg.red * 255), (int)(bg.green * 255),
                           (int)(bg.blue * 255), SHOES_COLOR_OPAQUE);
}
#else
VALUE shoes_native_window_color(shoes_app *app) {
    GtkStyle *style = gtk_widget_get_style(GTK_WIDGET(APP_WINDOW(app)));
    GdkColor bg = style->bg[GTK_STATE_NORMAL];
    return shoes_color_new(bg.red / 257, bg.green / 257, bg.blue / 257, SHOES_COLOR_OPAQUE);
}

VALUE shoes_native_dialog_color(shoes_app *app) {
    GtkStyle *style = gtk_widget_get_style(GTK_WIDGET(APP_WINDOW(app)));
    GdkColor bg = style->bg[GTK_STATE_NORMAL];
    return shoes_color_new(bg.red / 257, bg.green / 257, bg.blue / 257, SHOES_COLOR_OPAQUE);
}
#endif

VALUE shoes_dialog_alert(int argc, VALUE *argv, VALUE self) {
    GTK_APP_VAR(app);
    char atitle[50];
    g_sprintf(atitle, "%s says", title_app);
    rb_arg_list args;
    rb_parse_args(argc, argv, "S|h", &args);
    char *msg = RSTRING_PTR(shoes_native_to_s(args.a[0]));

    gchar *format_string = "<span size='larger'>%s</span>\n\n%s";
    if (argc == 2) {
        if (RTEST(ATTR(args.a[1], title))) {
            VALUE tmpstr = ATTR(args.a[1], title);
            strcpy(atitle,RSTRING_PTR(shoes_native_to_s(tmpstr)));
        } else {
            g_stpcpy(atitle," ");
        }
    }

    GtkWidget *dialog = gtk_message_dialog_new_with_markup(
                            window_app, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                            format_string, atitle, msg );

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return Qnil;
}

VALUE shoes_dialog_ask(int argc, VALUE *argv, VALUE self) {
    char atitle[50];
    GTK_APP_VAR(app);

    VALUE answer = Qnil;
    rb_arg_list args;
    rb_parse_args(argc, argv, "S|h", &args);

    switch(argc) {
        case 1:
            sprintf(atitle, "%s asks", title_app);
            break;
        case 2:
            if (RTEST(ATTR(args.a[1], title))) {
                VALUE tmpstr = ATTR(args.a[1], title);
                strcpy(atitle, RSTRING_PTR(shoes_native_to_s(tmpstr)));
            } else {
                g_stpcpy(atitle," ");
            }
            break;
    }

    //GtkWidget *dialog = gtk_dialog_new_with_buttons(atitle, APP_WINDOW(app), GTK_DIALOG_MODAL,
    //  _("_Cancel"), GTK_RESPONSE_CANCEL, _("_OK"), GTK_RESPONSE_OK, NULL);
    GtkWidget *dialog = gtk_dialog_new_with_buttons(atitle, window_app, GTK_DIALOG_MODAL,
                        _("_Cancel"), GTK_RESPONSE_CANCEL, _("_OK"), GTK_RESPONSE_OK, NULL);

    gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);
    gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 6);
    GtkWidget *question = gtk_label_new(RSTRING_PTR(shoes_native_to_s(args.a[0])));
    gtk_misc_set_alignment(GTK_MISC(question), 0, 0);
    GtkWidget *_answer = gtk_entry_new();
    if (RTEST(ATTR(args.a[1], secret))) shoes_native_secrecy(_answer);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), question, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), _answer, FALSE, TRUE, 3);

    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        const gchar *txt = gtk_entry_get_text(GTK_ENTRY(_answer));
        answer = rb_str_new2(txt);
    }
    gtk_widget_destroy(dialog);
    return answer;
}


VALUE shoes_dialog_confirm(int argc, VALUE *argv, VALUE self) {
    VALUE answer = Qfalse;
    char atitle[50];
    GTK_APP_VAR(app);
    //char *apptitle = RSTRING_PTR(app->title);
    rb_arg_list args;
    rb_parse_args(argc, argv, "S|h", &args);
    VALUE quiz = shoes_native_to_s(args.a[0]);

    switch(argc) {
        case 1:
            sprintf(atitle, "%s asks", title_app);
            break;
        case 2:
            if (RTEST(ATTR(args.a[1], title))) {
                VALUE tmpstr = ATTR(args.a[1], title);
                strcpy(atitle, RSTRING_PTR(shoes_native_to_s(tmpstr)));
            } else {
                g_stpcpy(atitle," ");
            }
            break;
    }



    //GtkWidget *dialog = gtk_dialog_new_with_buttons(atitle, APP_WINDOW(app), GTK_DIALOG_MODAL,
    //  _("_Cancel"), GTK_RESPONSE_CANCEL, _("_OK"), GTK_RESPONSE_OK, NULL);
    GtkWidget *dialog = gtk_dialog_new_with_buttons(atitle, window_app, GTK_DIALOG_MODAL,
                        _("_Cancel"), GTK_RESPONSE_CANCEL, _("_OK"), GTK_RESPONSE_OK, NULL);


    gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);
    gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 6);

    GtkWidget *question = gtk_label_new(RSTRING_PTR(quiz));
    gtk_misc_set_alignment(GTK_MISC(question), 0, 0);

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), question, FALSE, FALSE, 3);

    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK)
        answer = Qtrue;
    gtk_widget_destroy(dialog);
    return answer;

}

VALUE shoes_dialog_color(VALUE self, VALUE title) {
    VALUE color = Qnil;
    GTK_APP_VAR(app);
    title = shoes_native_to_s(title);
    GtkWidget *dialog = gtk_color_chooser_dialog_new(RSTRING_PTR(title), NULL);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        GdkRGBA _color;
        gtk_color_chooser_get_rgba((GtkColorChooser *)dialog, &_color);
        color = shoes_color_new((int)(_color.red*255), (int)(_color.green*255),
                                (int)(_color.blue*255), (int)(_color.alpha*255));
    }

    gtk_widget_destroy(dialog);
    return color;
}

VALUE shoes_dialog_chooser(VALUE self, char *title, GtkFileChooserAction act, const gchar *button, VALUE attr) {
    VALUE path = Qnil;
    GTK_APP_VAR(app);
    if (!NIL_P(attr) && !NIL_P(shoes_hash_get(attr, rb_intern("title"))))
        title = strdup(RSTRING_PTR(shoes_hash_get(attr, rb_intern("title"))));
    //GtkWidget *dialog = gtk_file_chooser_dialog_new(title, APP_WINDOW(app), act,
    //  _("_Cancel"), GTK_RESPONSE_CANCEL, button, GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *dialog = gtk_file_chooser_dialog_new(title, window_app, act,
                        _("_Cancel"), GTK_RESPONSE_CANCEL, button, GTK_RESPONSE_ACCEPT, NULL);
    if (act == GTK_FILE_CHOOSER_ACTION_SAVE)
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    if(RTEST(shoes_hash_get(attr, rb_intern("save"))))
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
                                          RSTRING_PTR(shoes_hash_get(attr, rb_intern("save"))));
    if(RTEST(shoes_hash_get(attr, rb_intern("types"))) && TYPE(shoes_hash_get(attr, rb_intern("types"))) == T_HASH) {
        VALUE hsh = shoes_hash_get(attr, rb_intern("types"));
        VALUE keys = rb_funcall(hsh, s_keys, 0);
        int i;
        for(i = 0; i < RARRAY_LEN(keys); i++) {
            VALUE key = rb_ary_entry(keys, i);
            VALUE val = rb_hash_aref(hsh, key);
            GtkFileFilter *ff = gtk_file_filter_new();
            gtk_file_filter_set_name(ff, RSTRING_PTR(key));
            gtk_file_filter_add_pattern(ff, RSTRING_PTR(val));
            gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ff);
        }
    }
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        path = rb_str_new2(filename);
    }
    if (!NIL_P(attr) && !NIL_P(shoes_hash_get(attr, rb_intern("title"))))
        SHOE_FREE(title);
    gtk_widget_destroy(dialog);
    return path;
}

VALUE shoes_dialog_open(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
#if 0
    VALUE attr = Qnil;
    char *title;
    switch (rb_parse_args(argc, argv, "|h", &args)) {
        case 0:
            title = strdup("Open file...");
            break;
        case 1:
            attr = args.a[0];
            title = strdup(RSTRING_PTR(shoes_hash_get(attr, rb_intern("title"))));
            break;
    }
    shoes_dialog_chooser(self, title, GTK_FILE_CHOOSER_ACTION_OPEN,
                         _("_Open"), args.a[0]);
    free(title);
    return;
#else
    rb_parse_args(argc, argv, "|h", &args);
    return shoes_dialog_chooser(self, "Open file...", GTK_FILE_CHOOSER_ACTION_OPEN,
                                _("_Open"), args.a[0]);
#endif
}

VALUE shoes_dialog_save(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    rb_parse_args(argc, argv, "|h", &args);
    return shoes_dialog_chooser(self, "Save file...", GTK_FILE_CHOOSER_ACTION_SAVE,
                                _("_Save"), args.a[0]);
}

VALUE shoes_dialog_open_folder(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    rb_parse_args(argc, argv, "|h", &args);
    return shoes_dialog_chooser(self, "Open folder...", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                _("_Open"), args.a[0]);
}

VALUE shoes_dialog_save_folder(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    rb_parse_args(argc, argv, "|h", &args);
    return shoes_dialog_chooser(self, "Save folder...", GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
                                _("_Save"), args.a[0]);
}

// July 2016 - Windows Fun!
#ifdef SHOES_GTK_WIN32
/*
 * This is only called when a shoe script uses the font(filename) command
 * so the file name is lacuna.ttf, coolvetica.ttf (Shoes splash or the
 * Shoes manual) or a user supplied font
*/
VALUE shoes_load_font(const char *filename) {
    VALUE allfonts, newfonts, oldfonts;
    // get current fconfig. Add the font to it so pango can find it.
    FcConfig *fc = FcConfigGetCurrent();
    FcBool yay = FcConfigAppFontAddFile(fc, (const FcChar8 *)filename);
    if (yay == FcFalse) {
        printf("failed to add font %s ?\n", filename);
    }
    // the Shoes api says an array of all fonts is returned. After a
    // font load, the Shoes fontlist must be updated. Use the much faster
    // Windows api. First, make sure Windows knows about the new one.
    int fonts = AddFontResourceEx(filename, FR_PRIVATE, 0);
    if (!fonts) return Qnil;
    allfonts = shoes_font_list();
    oldfonts = rb_const_get(cShoes, rb_intern("FONTS"));
    newfonts = rb_funcall(allfonts, rb_intern("-"), 1, oldfonts);
    shoes_update_fonts(allfonts);
    return newfonts;
}

static int CALLBACK shoes_font_list_iter(const ENUMLOGFONTEX *font, const NEWTEXTMETRICA *pfont, DWORD type, LPARAM l) {
    //VALUE ary = (VALUE)l; // TODO: compiler warning, remove because unused
    rb_ary_push(l, rb_str_new2(font->elfLogFont.lfFaceName));
    return TRUE;
}

VALUE shoes_font_list() {
    LOGFONT font = {0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, ""};
    VALUE ary = rb_ary_new();
    HDC dc = GetDC(NULL);
    EnumFontFamiliesEx(dc, &font, (FONTENUMPROC)shoes_font_list_iter, (LPARAM)ary, 0);
    ReleaseDC(NULL, dc);
    rb_funcall(ary, rb_intern("uniq!"), 0);
    rb_funcall(ary, rb_intern("sort!"), 0);
    return ary;
}

// hat tip: https://justcheckingonall.wordpress.com/2008/08/29/console-window-win32-app/
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

// called from main.c(skel) on Windows - works fine
static FILE* shoes_console_out = NULL;
static FILE* shoes_console_in = NULL;

int shoes_win32_console() {

    if (AllocConsole() == 0) {
        // cshoes.exe can get here
        printf("Already have console\n");
        return 0;
    }

    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;
    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;

    //* stash handles
    shoes_console_out = hf_out;
    shoes_console_in = hf_in;
    return 1;
}

// Called by Shoes after ruby/gtk/shoes is initialized and running
int shoes_native_terminal() {
    // has a console been setup by --console flag?
    if (shoes_console_out == NULL) {
        if (shoes_win32_console() == 0) // cshoes.exe can do this
            return 1;
    }
    // convert the (cached) FILE * for what ruby wants for fd[0], [1]...
    if (dup2(_fileno(shoes_console_out), 1) == -1)
        printf("failed dup2 of stdout\n");
    if (dup2(_fileno(shoes_console_out), 2) == -1)
        printf("failed dup2 of stderr\n");
    if (dup2(_fileno(shoes_console_in), 0) == -1)
        printf("failed dup2 of stdin\n");
    printf("created win32 console\n");
    return 1;
}
#else
/*
int shoes_native_console(char *app_path)
{
  //printf("init gtk console\n");
  shoes_native_app_console(app_path);
  printf("gtk\010k\t console \t\tcreated\n"); //test \b \t in string
  return 1;
}
*/
#endif
