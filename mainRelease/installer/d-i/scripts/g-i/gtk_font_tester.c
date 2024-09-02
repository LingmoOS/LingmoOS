/**
 * @file gtk_font_tester.c Font appearance tester for the graphical
 * debian-installer 
 *
 * Copyright (C) 2006 Attilio Fiandrotti <fiandro@tiscali.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This small application is meant to be compiled against GTKDFB libraries
 * and user to test how the translated strings found in the debian installer
 * look with different fonts.
 *
 * You can compile this application with
 * gcc gtk_font_tester.c -o gtk_font_tester `pkg-config --cflags --libs gtk+-2.0 gthread-2.0`
 *
 * It can be run either interactively or in batch mode:
 *   ./gtk_font_tester test.txt  1024 768
 *
 * Images will be saved in /var/log
**/

#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PADDING 6

GtkWidget *window, *scroll, *view, *button, *button_screenshot, *buttonbox_screenshot, *hbox, *vbox, *buttonbox, *entrybox, *entry, *loadfilebutton, *clearfilebutton;
GtkTextBuffer *buffer;

char preset_filename[1024];
int width=800 , height=600;

void screenshot_callback(GtkWidget *view );

static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    return FALSE;
} 

void append_text(char *filename)
{
	int fd, i; 
	char buf[1024];
    if ( access(filename, R_OK) )
    	return;
    	
	fd=open(filename,O_RDONLY );
	while ( (i=read(fd, buf, 1024)) > 0) {
		gtk_text_buffer_insert_at_cursor (buffer, buf, i);
	}
	
}

void loadfile_callback(GtkWidget *button)
{
    GtkTextIter *end_iter;
	char *filename;
	filename = (char*)gtk_entry_get_text(GTK_ENTRY(entry));
    append_text(filename);

}

void clear_callback(GtkWidget *button)
{
    gtk_text_buffer_set_text (buffer, "", -1);
}

void reload_gtkrc_callback(GtkWidget *button, GtkWidget *view)
{
    gtk_rc_reparse_all();
    
}

void screenshot_thread()
{
    sleep(1);
    gdk_threads_enter();

    screenshot_callback(view);
    gtk_main_quit();    

    gdk_threads_leave();
}

void screenshot_callback(GtkWidget *view )
{
    GdkWindow *gdk_window;
    GdkPixbuf *gdk_pixbuf;
    GtkWidget *window, *frame, *message_label, *title_label, *h_box, *v_box, *v_box_outer, *close_button, *actionbox, *separator;
    gint x, y, width, height, depth;
    int i, j;
    char screenshot_name[256], popup_message[256];
    char *label_title_string;
	
    gdk_window = gtk_widget_get_parent_window ( view );
    gdk_window_get_geometry( gdk_window, &x, &y, &width, &height, &depth);
    gdk_pixbuf = gdk_pixbuf_get_from_drawable(NULL, gdk_window, gdk_colormap_get_system(),0,0,0,0, width, height);
    i=0;
	while (TRUE) {
        sprintf(screenshot_name, "test_%d.png", i );
        sprintf(popup_message, "/var/log/%s", screenshot_name );
        sprintf(screenshot_name, "%s", popup_message );
        if ( ! access(screenshot_name, R_OK) )
            i++;
        else {
            /* printf ("name: %s.png\nx: %d\ny: %d\nwidth: %d\nheight: %d\ndepth=%d\n", screenshot_name, x, y, width, height, depth); */
            break;
        }
    }
    gdk_pixbuf_save (gdk_pixbuf, screenshot_name, "png", NULL, NULL);
    g_object_unref(gdk_pixbuf);

    if(strlen(preset_filename) == 0) {
	    /* A message inside a popup window tells the user the screenshot has
	     * been saved correctly
	     */
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
        gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
        gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
        gtk_container_set_border_width (GTK_CONTAINER (window), 0);

        title_label = gtk_label_new ("Screenshot");
        gtk_misc_set_alignment(GTK_MISC(title_label), 0, 0);
        label_title_string = malloc( strlen("Screenshot") + 8 );
        sprintf(label_title_string,"<b>Screenshot</b>");
        gtk_label_set_markup(GTK_LABEL(title_label), label_title_string);
        sprintf(popup_message, "Screenshot saved as %s", screenshot_name );
        message_label = gtk_label_new (popup_message);

        actionbox = gtk_hbutton_box_new();
        gtk_button_box_set_layout (GTK_BUTTON_BOX(actionbox), GTK_BUTTONBOX_END);
        close_button = gtk_button_new_with_label ("Continue");
        g_signal_connect_swapped (G_OBJECT (close_button), "clicked", G_CALLBACK (gtk_widget_destroy), G_OBJECT (window));
        gtk_box_pack_end (GTK_BOX(actionbox), close_button, TRUE, TRUE, DEFAULT_PADDING);

        v_box = gtk_vbox_new(FALSE, DEFAULT_PADDING);
        gtk_box_pack_start(GTK_BOX (v_box), title_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX (v_box), message_label, FALSE, FALSE, DEFAULT_PADDING);
        separator = gtk_hseparator_new();
        gtk_box_pack_start(GTK_BOX (v_box), separator, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX (v_box), actionbox, FALSE, FALSE, 0);
        h_box = gtk_hbox_new(FALSE, DEFAULT_PADDING);
        gtk_box_pack_start(GTK_BOX (h_box), v_box, FALSE, FALSE, DEFAULT_PADDING);
        v_box_outer = gtk_vbox_new(FALSE, DEFAULT_PADDING);
        gtk_box_pack_start(GTK_BOX (v_box_outer), h_box, FALSE, FALSE, DEFAULT_PADDING);
	    
        frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_OUT);
        gtk_container_add (GTK_CONTAINER (frame), v_box_outer);
        gtk_container_add (GTK_CONTAINER (window), frame);
        gtk_widget_show_all (window);

	    free(label_title_string);
    }
    else {
        printf("Screenshot saved as \"%s\"\n", screenshot_name);
    }
}

int main( int argc, char *argv[] )
{
	GThread *thread_screenshot; 
	GError *err1 = NULL;
	
	int run_mode = 0;	/* 0 - interactive mode (default), 1 - batch mode */

    if  ( argc == 1 )
        strcpy( preset_filename, "");
        
    else if ( argc == 4 ) {
        if ( access(argv[1], R_OK) ) {
            printf ("File %s could not be open\n", argv[1]);
            exit(0);
            
        }
        else {
	    run_mode = 1;
            strcpy(preset_filename, argv[1]);
            width = atoi(argv[2]);
            height = atoi(argv[3]);
        }
    }
   
    else {
        printf ("Usage: %s [FILE WIDTH HEIGHT]\n", argv[0]);
        exit (0);
    }

    gtk_init (&argc, &argv);

    if( !g_thread_supported() )
    {
       g_thread_init(NULL);
       gdk_threads_init();
    }
  
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (delete_event), NULL);
    g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);
    gtk_widget_set_size_request (window, width, height);

    view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW(view), DEFAULT_PADDING);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW(view), DEFAULT_PADDING);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

    if (!run_mode)
      gtk_text_buffer_set_text (buffer, "You can type some text inside this GTKTextView or load text from a file by entering its name inside the above GtkEntry and see it rendered here", -1);

    hbox = gtk_hbox_new (FALSE, DEFAULT_PADDING);
    vbox = gtk_vbox_new (FALSE, DEFAULT_PADDING);

    if(strlen(preset_filename) == 0) {
        scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW (scroll), view);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, DEFAULT_PADDING);
    	
        buttonbox = gtk_hbox_new (TRUE, DEFAULT_PADDING);
        button = gtk_button_new_with_label("Reload gtkrc");
        g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (reload_gtkrc_callback), G_OBJECT (window));
        gtk_box_pack_start(GTK_BOX(buttonbox), button, TRUE, TRUE, DEFAULT_PADDING);
        button = gtk_button_new_with_label("Screenshot");
        g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (screenshot_callback), G_OBJECT (view));
        gtk_box_pack_start(GTK_BOX(buttonbox), button, TRUE, TRUE, DEFAULT_PADDING);  
        button = gtk_button_new_with_label("Quit");
        g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (gtk_widget_destroy), G_OBJECT (view));
        gtk_box_pack_start(GTK_BOX(buttonbox), button, TRUE, TRUE, DEFAULT_PADDING); 

        entrybox = gtk_hbox_new (FALSE, DEFAULT_PADDING);
        entry = gtk_entry_new ();
        gtk_entry_set_text (GTK_ENTRY(entry), "Enter here name of file to be displayed");
	    gtk_entry_set_max_length (GTK_ENTRY (entry), 256 );
        loadfilebutton = gtk_button_new_with_label("Append file");
        g_signal_connect (G_OBJECT (loadfilebutton), "clicked", G_CALLBACK (loadfile_callback), NULL);
        clearfilebutton = gtk_button_new_with_label("Clear");
        g_signal_connect (G_OBJECT (clearfilebutton), "clicked", G_CALLBACK (clear_callback), NULL);
        gtk_box_pack_start(GTK_BOX(entrybox), entry, TRUE, TRUE, DEFAULT_PADDING); 
        gtk_box_pack_start(GTK_BOX(entrybox), loadfilebutton, FALSE, TRUE, DEFAULT_PADDING); 
        gtk_box_pack_start(GTK_BOX(entrybox), clearfilebutton, FALSE, TRUE, DEFAULT_PADDING);
        gtk_box_pack_start(GTK_BOX(vbox), entrybox, FALSE, TRUE, DEFAULT_PADDING);
        gtk_box_pack_start(GTK_BOX(vbox), buttonbox, FALSE, TRUE, DEFAULT_PADDING);
    }
    else {
        gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, DEFAULT_PADDING);
        append_text(preset_filename);
    }

    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, DEFAULT_PADDING);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    gtk_widget_show_all (window);

    if(strlen(preset_filename) > 0) {
    	if( (thread_screenshot = g_thread_create((GThreadFunc)screenshot_thread, NULL, TRUE, &err1)) == NULL) {
		    printf("Thread create failed: %s!!\n", err1->message );
            g_error_free ( err1 ) ;
            exit(0);
        }   
    }

    gtk_main ();
    
    return 0;
}
