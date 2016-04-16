#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <signal.h>

#include "tesi.h"
#include <gdk/gdkkeysyms.h>
/*
 * heavily modified from https://github.com/alanszlosek/tesi/
 * for use in Shoes/Linux
*/


static gboolean keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	struct tesiObject *tobj = (struct tesiObject*)data;
    char *c = ((GdkEventKey*)event)->string;
	char s = *c;
	if (event->keyval == GDK_KEY_BackSpace) {
		s = 010;
    }
	write(tobj->fd_input, &s, 1);
	return TRUE;
}

static gboolean clear_console(GtkWidget *widget, GdkEvent *event, gpointer data) {
	struct tesiObject *tobj = (struct tesiObject*) data;
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *newbuf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(view, newbuf);
	// set a mark to the end? get focus
	gtk_widget_grab_focus(GTK_WIDGET(view));
	return TRUE;
}

static gboolean copy_console(GtkWidget *widget, GdkEvent *event, gpointer data) {
	struct tesiObject *tobj = (struct tesiObject*) data;
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	GtkTextIter iter_s, iter_e;
    gtk_text_buffer_get_bounds(buffer, &iter_s, &iter_e);
	gchar *bigstr = gtk_text_buffer_get_slice(buffer, &iter_s, &iter_e, TRUE);
	GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(primary, bigstr, strlen(bigstr));
	return TRUE;
}

/*
 * This is called to handle characters received from the pty
 * in response to a puts/printf/write from Shoes,Ruby, & C
 * It does't manage escape seq, x,y or deal with width and height.
 * Just write to the end of the buffer and let the gtk_text_view manage it.
*/
void console_haveChar(void *p, char c) {
	struct tesiObject *tobj = (struct tesiObject*)p;
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
  GtkTextIter iter_e;
  GtkTextMark *insert_mark;
  char in[8];

	snprintf(in, 7, "%c", c);
	if (c >= 32 && c != 127) {
	    buffer = gtk_text_view_get_buffer(view);
	    gtk_text_buffer_insert_at_cursor(buffer, in, 1);
		return;
    }
	switch (c) {
		case '\x1B': // begin escape sequence (aborting previous one if any)
			//tobj->partialSequence = 1;
			// possibly flush buffer
			break;

		case '\r': // carriage return ('M' - '@'). Move cursor to first column.
			// odds are high this preceeds a \n. Move to the begining of
			// last line in buffer line.  What happens if we insert
			break;

		case '\n':  // line feed ('J' - '@'). Move cursor down line and to first column.
		    // just insert '\n' into the buffer.
		    gtk_text_buffer_insert_at_cursor(buffer, in, 1);
			break;

		case '\t': // ht - horizontal tab, ('I' - '@')
		    // textview can handle tabs - it claims.
	        gtk_text_buffer_insert_at_cursor(buffer, in, 1);
	 		break;

		case '\a': // bell ('G' - '@')
	 		// do nothing for now... maybe a visual bell would be nice?
			break;

	 	case 8: // backspace cub1 cursor back 1 ('H' - '@')
            gtk_text_buffer_get_end_iter (buffer, &iter_e);
            gtk_text_buffer_backspace(buffer, &iter_e, 1, 1);
			break;

		default:
#ifdef DEBUG
			fprintf(stderr, "Unrecognized control char: %d (^%c)\n", c, c + '@');
#endif
			break;
	}
	// tell the view to show the newest position
	// from http://www.gtkforums.com/viewtopic.php?t=1307
	gtk_text_buffer_get_end_iter (buffer, &iter_e);
	insert_mark = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_place_cursor(buffer, &iter_e);
  gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (view),
      insert_mark, 0.0, TRUE, 0.0, 1.0);
}

// for more control, implement this

void console_visAscii(struct tesiObject *tobj, char c, int x, int y) {
	char in[8];
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
  GtkTextIter iter_e;
  GtkTextMark *insert_mark;
  
	snprintf(in, 7, "%c", c);
  gtk_text_buffer_insert_at_cursor(buffer, in, 1);
  // update on screen
	gtk_text_buffer_get_end_iter (buffer, &iter_e);
	insert_mark = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_place_cursor(buffer, &iter_e);
  gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (view),
      insert_mark, 0.0, TRUE, 0.0, 1.0);
}

void console_return(struct tesiObject *tobj, int x, int y) {
  // do nothing for now
}

void console_newline(struct tesiObject *tobj, int x, int y) {
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
  GtkTextIter iter_e;
  GtkTextMark *insert_mark;

  gtk_text_buffer_insert_at_cursor(buffer, "\n", 1);
  // update on screen
	gtk_text_buffer_get_end_iter (buffer, &iter_e);
	insert_mark = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_place_cursor(buffer, &iter_e);
  gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (view),
      insert_mark, 0.0, TRUE, 0.0, 1.0);
}

void console_backspace(struct tesiObject *tobj, int x, int y) {
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
  GtkTextIter iter_e;
  GtkTextMark *insert_mark;

  gtk_text_buffer_get_end_iter (buffer, &iter_e);
  gtk_text_buffer_backspace(buffer, &iter_e, 1, 1);
  
  // update on screen
	gtk_text_buffer_get_end_iter (buffer, &iter_e);
	insert_mark = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_place_cursor(buffer, &iter_e);
  gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (view),
      insert_mark, 0.0, TRUE, 0.0, 1.0);
}
void console_tab(struct tesiObject *tobj, int x, int y) {
  return console_visAscii(tobj, '\t', x, y);
}

/*  
 * Handle terminal attributes to Gtk Textview/buffer settings
 * 
*/

static GdkRGBA colortable[10];
static void initattr() {
  gdk_rgba_parse (&colortable[0], "black");
  gdk_rgba_parse (&colortable[1], "red");
  gdk_rgba_parse (&colortable[2], "green");
  gdk_rgba_parse (&colortable[3], "brown");
  gdk_rgba_parse (&colortable[4], "blue");
  gdk_rgba_parse (&colortable[5], "magenta");
  gdk_rgba_parse (&colortable[6], "cyan");
  gdk_rgba_parse (&colortable[7], "white");
  gdk_rgba_parse (&colortable[8], "black");  // weird one not used? default?
  gdk_rgba_parse (&colortable[9], "black");  // weird one not used? default?
  // set tabs ? 
}

void terminal_attreset(struct tesiObject *tobj) {
  // reset all attibutes (color, blink,...)
  GtkWidget *view = GTK_WIDGET(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (view));
  // This changes the whole buffer/view which is not what we want.    
  //gtk_widget_override_color (view, GTK_STATE_FLAG_NORMAL, NULL);
}

void terminal_setfgcolor(struct tesiObject *tobj, int fg) {
  GtkWidget *view = GTK_WIDGET(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (view));
  // This changes the whole buffer/view which is not what we want. 
  // Might be useful for initializing the whole    
  GdkRGBA rgba = colortable[fg - 30];
  gtk_widget_override_color (view, GTK_STATE_FLAG_NORMAL, &rgba);
}

// functions that haven't been tested. May not work or incomplete

void console_eraseCharacter(struct tesiObject *tobj, int x, int y) {
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
  GtkTextIter iter_e;
  
	//buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(p)); ?
	gtk_text_buffer_get_iter_at_line_index(buffer, &iter_e, y, x);
	gtk_text_buffer_backspace(buffer, &iter_e, 1, 1);  
}


void console_scrollUp(struct tesiObject *tobj) {
	// add line to buffer, scroll up
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	gint lcnt;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tobj->pointer));
	lcnt = gtk_text_buffer_get_line_count(buffer);
	gtk_text_buffer_get_iter_at_line_index(buffer, &iter, lcnt+1, 0);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(tobj->pointer), &iter, 0.0, false, 0.0, 0.0);
}

void console_moveCursor(struct tesiObject *tobj, int x, int y) {
	/*
	Force moving of cursor
	If line doesn't exist, start at last line and loop while adding newlines
	If line does exist, but column doesn't, go to line and add spaces at end of line
	*/
  
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tobj->pointer));
    GtkTextIter iter;
	//printf("Move Cursor to x,y: %d,%d\n", x, y);

	gtk_text_buffer_get_iter_at_line_index(buffer, &iter, y, 0);
	while(gtk_text_iter_get_line(&iter) < y) { // loop and fill out contents to destination line
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert(buffer, &iter, "\n", 1);
	}
 
	gtk_text_buffer_get_iter_at_line_index(buffer, &iter, y, x);
	gtk_text_iter_forward_to_line_end(&iter);
	while(gtk_text_iter_get_line_offset(&iter) < x) { // loop and fill out contents to destination column
		gtk_text_buffer_insert(buffer, &iter, " ", 1);
    }
    gtk_text_buffer_place_cursor(buffer, &iter);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(tobj->pointer), &iter, 0.0, false, 0.0, 0.0);


}
void console_insertLine(struct tesiObject *tobj, int y) {
	printf("Insert Line\n");
}

void console_eraseLine(struct tesiObject *tobj, int y) {
	GtkTextIter iter, s, e;
	GtkTextBuffer *buffer;
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	buffer = gtk_text_view_get_buffer(view);

	gtk_text_buffer_get_end_iter(buffer, &iter);
	s = e = iter;
	gtk_text_iter_backward_line(&s); // move to start of current line (delimiter)
	gtk_text_iter_forward_to_line_end(&e);
	gtk_text_iter_backward_char(&e);
	gtk_text_buffer_delete(buffer, &s, &e);
	printf("Erase Line\n");
}
// end of incomplete/untested  functions 

gboolean g_tesi_handleInput(gpointer data) {
	tesi_handleInput( (struct tesiObject*) data);
	return TRUE;
}

// access to shoes_global_console
#include "shoes/app.h"

static gboolean clean(GtkWidget *widget, GdkEvent *event, gpointer data) {
  struct tesiObject *to = (struct tesiObject*)data;
  g_source_remove(to->ides); // remove g_timeout_add
  g_source_destroy(g_main_context_find_source_by_id(NULL, to->ides));
  
  deleteTesiObject(data); // FIXME see deleteTesiObject
  
  shoes_global_console = 0;
  return FALSE;
}

void shoes_native_app_console() {
  GtkWidget *window;
  GtkWidget *canvas;
  GtkWidget *vbox;
  GtkScrolledWindow *sw;
  PangoFontDescription *pfd;  // for terminal
  PangoFontDescription *bpfd; // for Label in button panel
  
  // get DIR (a path) from Shoes.
  char *app_path;

  struct tesiObject *t;

  //gtk_init (&argc, &argv); // Nope. it's already running

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  // size set below based on font (80x24)
  gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
  gtk_window_set_title (GTK_WINDOW (window), "Shoes Linux");
  
  // like a Shoes stack at the top.
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  // need a panel with a string (icon?), copy button and clear button
  GdkColor bg_color, color_white;
  gdk_color_parse ("black", &bg_color);
  gdk_color_parse ("white", &color_white);

  GtkWidget *btnpnl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2); // think flow layout

  GtkWidget *announce = gtk_label_new("Shoes Terminal");
  bpfd = pango_font_description_from_string ("Sans-Serif 14");
  gtk_widget_override_font (announce, bpfd);

  gtk_box_pack_start(GTK_BOX(btnpnl), announce, 1, 0, 0);
  // create widgets for btnpnl
  // icon is first
  GdkPixbuf * icon_pixbuf = gtk_window_get_icon (window);
  GtkWidget *icon = gtk_image_new_from_pixbuf (icon_pixbuf);
  gtk_box_pack_start (GTK_BOX(btnpnl), icon, 1, 0, 0);
  
  GtkWidget *clrbtn = gtk_button_new_with_label ("Clear");
  gtk_box_pack_start (GTK_BOX(btnpnl), clrbtn, 1, 0, 0);
  GtkWidget *cpybtn = gtk_button_new_with_label ("Copy");
  gtk_box_pack_start (GTK_BOX(btnpnl), cpybtn, 1, 0, 0);
  gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(btnpnl), 0, 0, 0);

  // then a widget/panel for the terminal
  sw = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
  gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(sw),
    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(sw), 1, 1, 0);

  canvas = gtk_text_view_new();
  gtk_container_add (GTK_CONTAINER (sw), canvas);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(canvas), TRUE);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(canvas), 4);
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(canvas), 4);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(canvas), GTK_WRAP_CHAR);
  
  // init attributes
  initattr();

  // set font for scrollable window
  pfd = pango_font_description_from_string ("monospace 10");
  gtk_widget_override_font (announce, pfd);

  // compute 'char' width and tab settings.
  PangoLayout *playout;
  PangoTabArray *tab_array;
  gint charwidth, charheight, tabwidth;
  playout = gtk_widget_create_pango_layout(canvas, "M");
  pango_layout_set_font_description(playout, pfd);
  pango_layout_get_pixel_size(playout, &charwidth, &charheight);
  tabwidth = charwidth * 8;
  tab_array = pango_tab_array_new(1, TRUE);
  pango_tab_array_set_tab( tab_array, 0, PANGO_TAB_LEFT, tabwidth);
  gtk_text_view_set_tabs(GTK_TEXT_VIEW(canvas), tab_array);
  
  gtk_widget_set_size_request (GTK_WIDGET (sw), 80*charwidth, 24*charheight);

  t = newTesiObject("/bin/bash", 80, 24); // first arg not used
  t->pointer = canvas;
  //t->callback_haveCharacter = &console_haveChar;
  // cjc - haveCharacter short circuts much (all?) of these callbacks:
  t->callback_handleNL = &console_newline;
  t->callback_handleRTN = &console_return;
  t->callback_handleBS = &console_backspace;
  t->callback_handleTAB = &console_tab; 
  t->callback_handleBEL = NULL;
  t->callback_printCharacter = &console_visAscii;
  t->callback_attreset = &terminal_attreset;
  t->callback_charattr = NULL;
  t->callback_setfgcolor= &terminal_setfgcolor;
  t->callback_setbgcolor = NULL;
  t->callback_attributes = NULL; // old tesi

  t->callback_eraseCharacter = NULL; // &console_eraseCharacter;
  t->callback_moveCursor = NULL; // &console_moveCursor;
  t->callback_insertLine = NULL; //&console_insertLine;
  t->callback_eraseLine = NULL; //&console_eraseLine;
  t->callback_scrollUp = NULL; // &console_scrollUp;
  
  g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(clean), t);
//  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);

  g_signal_connect (G_OBJECT (canvas), "key-press-event", G_CALLBACK (keypress_event), t);
  g_signal_connect (G_OBJECT (clrbtn), "clicked", G_CALLBACK (clear_console), t);
  g_signal_connect (G_OBJECT (cpybtn), "clicked", G_CALLBACK (copy_console), t);
  
  gtk_widget_grab_focus(canvas);
  unsigned int ides = g_timeout_add(100, &g_tesi_handleInput, t);
  t->ides = ides;

  gtk_widget_show_all (window);
  
  // TODO: some clean up here. Complete ?
  pango_font_description_free(pfd);
  pango_font_description_free(bpfd);
  pango_tab_array_free(tab_array);
  g_object_unref(playout);
  return;
}
