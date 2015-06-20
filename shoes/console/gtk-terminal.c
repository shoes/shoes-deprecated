#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <signal.h>

#include "tesi.h"

//GtkTextBuffer *buffer;
//GtkTextIter iter;  //cjc this changes too much to be global

/* 
 * modified from https://github.com/alanszlosek/tesi/ 
*/
 
/*
 * The widget needs to be non-editable, with the cursor focusing at the end of the buffer.
 * Need to make use of a tag table for the colors
 * Track key presses to window/canvas
 * */

static gboolean keypress_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
	struct tesiObject *t = (struct tesiObject*) data;
	char *c = ((GdkEventKey*)event)->string;
	char s = *c;
    
	write(t->fd_input, &s, 1);
	return TRUE;
}

static gboolean clear_console(GtkWidget *widget, GdkEvent *event, gpointer data) {
	struct tesiObject *tobj = (struct tesiObject*) data;
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	printf("cleared buffer and view\n"); // FIXME: do stuff
	return TRUE;
}

static gboolean copy_console(GtkWidget *widget, GdkEvent *event, gpointer data) {
	struct tesiObject *tobj = (struct tesiObject*) data;
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	printf("copied to clipboard\n"); // FIXME: do stuff
	return TRUE;
}

void tesi_printCharacter(void *p, char c, int x, int y) { 
	char in[129];
	GtkTextView *view = GTK_TEXT_VIEW(p);
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	snprintf(in, 128, "%c", c);
	buffer = gtk_text_view_get_buffer(view);
	gtk_text_buffer_insert_at_cursor(buffer, in, 1);
	
	//gtk_text_buffer_get_end_iter(buffer, &iter);
	//gtk_text_buffer_insert(buffer, &iter, in, 1); 
}
void tesi_eraseCharacter(void *p, int x, int y) {
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(p));
	//gtk_text_iter_forward_to_line_end(&i);
	gtk_text_buffer_get_iter_at_line_index(buffer, &iter, y, x);
	//gtk_text_buffer_delete(buffer, &iter, &iter);  // cjc
	gtk_text_buffer_backspace(buffer, &iter, 1, 1);  // cjc 
}

 
void tesi_scrollUp(void *p) {
	// add line to buffer, scroll up 
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	gint lcnt;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(p));
	lcnt = gtk_text_buffer_get_line_count(buffer);
	//gtk_text_buffer_get_iter_at_line_index(buffer, &iter, lcnt, 0);
	gtk_text_buffer_get_iter_at_line_index(buffer, &iter, lcnt+1, 0);
	// scroll view (*p)
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(p), &iter, 0.0, false, 0.0, 0.0);
	//gtk_text_buffer_insert(buffer, &iter, "\n", 1);
	// Move cursor done by test_limitCursor call back to tesi_moveCursor below
	// gtk_text_view_place_cursor_onscreen (GTK_TEXT_VIEW(p));
}

void tesi_moveCursor(void *p, int x, int y) {
	/*
	Force moving of cursor
	If line doesn't exist, start at last line and loop while adding newlines
	If line does exist, but column doesn't, go to line and add spaces at end of line
	*/
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(p));
    GtkTextIter iter;
	//printf("Move Cursor to x,y: %d,%d\n", x, y);

	gtk_text_buffer_get_iter_at_line_index(buffer, &iter, y, 0);
	while(gtk_text_iter_get_line(&iter) < y) { // loop and fill out contents to destination line
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert(buffer, &iter, "\n", 1);  
	}
    int lcnt = gtk_text_buffer_get_line_count(buffer);
	gtk_text_buffer_get_iter_at_line_index(buffer, &iter, y, x);
	gtk_text_iter_forward_to_line_end(&iter);
	while(gtk_text_iter_get_line_offset(&iter) < x) { // loop and fill out contents to destination column
		gtk_text_buffer_insert(buffer, &iter, " ", 1);
    }
    gtk_text_buffer_place_cursor(buffer, &iter);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(p), &iter, 0.0, false, 0.0, 0.0);

	
}
void tesi_insertLine(void *p) {
	printf("Insert Line\n");
}

void tesi_eraseLine(void *p) {
	GtkTextIter iter, s, e;
	GtkTextBuffer *buffer;
	GtkTextView *view = GTK_TEXT_VIEW(p);
	buffer = gtk_text_view_get_buffer(view);
	
	gtk_text_buffer_get_end_iter(buffer, &iter);
	s = e = iter;
	gtk_text_iter_backward_line(&s); // move to start of current line (delimiter)
	gtk_text_iter_forward_to_line_end(&e);
	gtk_text_iter_backward_char(&e);
	gtk_text_buffer_delete(buffer, &s, &e);
	printf("Erase Line\n");
}

gboolean g_tesi_handleInput(gpointer data) {
	tesi_handleInput( (struct tesiObject*) data);
	return TRUE;
}

shoes_native_app_console () {  //int main(int argc, char *argv[]) {
	GtkWidget *window;
	GtkWidget *canvas;
	GtkWidget *vbox;
	GtkScrolledWindow *sw;
	PangoFontDescription *pfd;

	//GtkTextBuffer *buffer;
	//GtkTextIter i;

	struct tesiObject *t;

	//gtk_init (&argc, &argv);

	/* create a new window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request (GTK_WIDGET (window), 800, 468);
	gtk_window_set_title (GTK_WINDOW (window), "Shoes Terminal");
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
	g_signal_connect_swapped (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_widget_destroy), G_OBJECT (window));
    // like a Shoes stack at the top. 
	vbox = gtk_vbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	// need a box with a string, copy button and clear button
	GtkWidget *btnpnl = gtk_hbox_new(false, 2); // think flow layout
	GtkWidget *announce = gtk_label_new("Shoes console");
	gtk_box_pack_start(GTK_BOX(btnpnl), announce, 1, 0, 0);
	GtkWidget *clrbtn = gtk_button_new_with_label ("Clear");
	gtk_box_pack_start (GTK_BOX(btnpnl), clrbtn, 1, 0, 0);
 	GtkWidget *cpybtn = gtk_button_new_with_label ("Copy");
	gtk_box_pack_start (GTK_BOX(btnpnl), cpybtn, 1, 0, 0);
    gtk_box_pack_start (GTK_BOX(vbox), GTK_CONTAINER(btnpnl), 0, 0, 0);

    // then a widget/panel for the terminal 

	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(sw), 
	  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(sw), 1, 1, 0);

	canvas = gtk_text_view_new();
	gtk_container_add (GTK_CONTAINER (sw), canvas);
	//pfd = pango_font_description_from_string ("courier");
	pfd = pango_font_description_from_string ("monospace 12");
	
    PangoContext *pc;
	PangoFont *pfont;
	PangoFontMetrics *metrics;
	pc = gtk_widget_get_pango_context(sw);
	pfont = pango_context_load_font(pc, pfd);

    
    metrics = pango_font_get_metrics(pfont, NULL);
    int cwidth = pango_font_metrics_get_approximate_char_width(metrics);
	

	gtk_widget_modify_font (canvas, pfd);

	t = newTesiObject("/bin/bash", 80, 24); // first arg not used
	t->pointer = canvas;
	t->callback_printCharacter = &tesi_printCharacter;
	t->callback_eraseCharacter = &tesi_eraseCharacter;
	t->callback_moveCursor = &tesi_moveCursor;
	t->callback_insertLine = &tesi_insertLine;
	t->callback_eraseLine = &tesi_eraseLine;
	t->callback_scrollUp = &tesi_scrollUp;

	g_signal_connect (G_OBJECT (canvas), "key-press-event", G_CALLBACK (keypress_event), t);
    g_signal_connect (G_OBJECT (clrbtn), "clicked", G_CALLBACK (clear_console), t);
    g_signal_connect (G_OBJECT (cpybtn), "clicked", G_CALLBACK (copy_console), t);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(canvas));
    gtk_widget_grab_focus(canvas);
	g_timeout_add(100, &g_tesi_handleInput, t);

	gtk_widget_show_all (window);
	// should do some clean up here. Free fontdescription etc
	//gtk_main();

	//kill(t->pid, SIGKILL);
	//deleteTesiObject(t);
	return;
}
