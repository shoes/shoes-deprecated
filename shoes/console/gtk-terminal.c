#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <signal.h>

#include "tesi.h"

GtkTextBuffer *buffer;
GtkTextIter iter;

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

void tesi_printCharacter(void *p, char c) {
	char in[129];
	
	GtkTextBuffer *buffer;
	snprintf(in, 128, "%c", c);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(p));
	//gtk_text_buffer_get_end_iter(buffer, &i);
	//printf("Print %c\n", c);

	gtk_text_buffer_insert(buffer, &iter, in, 1);
}
void tesi_eraseCharacter(void *p) {
	GtkTextBuffer *buffer;
	GtkTextIter i;
	/*
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(p));
	gtk_text_iter_forward_to_line_end(&i);
	gtk_text_buffer_get_iter_at_line_index(buffer, &iter, y, x);
	gtk_text_buffer_delete(buffer, &iter, &iter);
	*/
}
void tesi_moveCursor(void *p, int x, int y) {
	/*
	Force moving of cursor
	If line doesn't exist, start at last line and loop while adding newlines
	If line does exist, but column doesn't, go to line and add spaces at end of line
	*/
	//GtkTextBuffer *buffer;
	//buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(p));

	//printf("Move Cursor to x,y: %d,%d\n", x, y);

	gtk_text_buffer_get_iter_at_line_index(buffer, &iter, y, 0);
	while(gtk_text_iter_get_line(&iter) < y) { // loop and fill out contents to destination line
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert(buffer, &iter, "\n", 1);
	}

	//gtk_text_buffer_get_iter_at_line_index(buffer, &iter, y, x);
	gtk_text_iter_forward_to_line_end(&iter);
	while(gtk_text_iter_get_line_offset(&iter) < x) { // loop and fill out contents to destination column
		gtk_text_buffer_insert(buffer, &iter, " ", 1);
        }

	
}
void tesi_insertLine(void *p) {
	printf("Insert Line\n");
}
void tesi_eraseLine(void *p) {
	GtkTextIter s, e;
	//GtkTextBuffer *buffer;
	//buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(p));
	
	//gtk_text_buffer_get_end_iter(buffer, &iter);
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
	gtk_widget_set_size_request (GTK_WIDGET (window), 800, 800);
	gtk_window_set_title (GTK_WINDOW (window), "TESI Gtk Terminal");
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
	g_signal_connect_swapped (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_widget_destroy), G_OBJECT (window));

	vbox = gtk_vbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(sw), 1, 1, 0);

	canvas = gtk_text_view_new();
	gtk_container_add (GTK_CONTAINER (sw), canvas);
	//pfd = pango_font_description_from_string ("courier");
	pfd = pango_font_description_from_string ("monospace");
	gtk_widget_modify_font (canvas, pfd);

	t = newTesiObject("/bin/bash", 70, 24);
	t->pointer = canvas;
	t->callback_printCharacter = &tesi_printCharacter;
	t->callback_eraseCharacter = &tesi_eraseCharacter;
	t->callback_moveCursor = &tesi_moveCursor;
	t->callback_insertLine = &tesi_insertLine;
	t->callback_eraseLine = &tesi_eraseLine;

	g_signal_connect (G_OBJECT (canvas), "key-press-event", G_CALLBACK (keypress_event), t);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(canvas));
	//gtk_text_buffer_set_text(buffer, "This is a\ntest", 14);
	gtk_text_buffer_get_iter_at_line(buffer, &iter, 0);
	//gtk_text_buffer_insert(buffer, &i, "M", 1);

	/*
	gtk_text_buffer_get_iter_at_line_index(buffer, &i, 2, 0);
	if(gtk_text_iter_get_line(&i) != 2)
		gtk_text_buffer_get_end_iter(buffer, &i);
	gtk_text_buffer_insert(buffer, &i, "\na", 2);
	*/
	//tesi_handleInput(t);
	//tesi_handleInput(t);

	g_timeout_add(100, &g_tesi_handleInput, t);

	gtk_widget_show_all (window);
	//gtk_main();

	//kill(t->pid, SIGKILL);
	//deleteTesiObject(t);
	return;
}
