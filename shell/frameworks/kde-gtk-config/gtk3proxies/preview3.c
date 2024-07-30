/* 
 * SPDX-FileCopyrightText: 2011 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 * 
 * #include <gtk/gtk.h>
 */

#include <gtk/gtkx.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void printHelp()
{
    printf(
        "Small utility for previewing Gtk3 theme.\n"
        "\n"
        "Using:\n"
        "gtk3_preview <args>\n"
        "\n"
        "Arguments:\n"
        "  -h, --help\tShows this help\n"
        "  -V, --version\tPrints the program version\n"
        "  <winid>\t Creates a window that can be embedded using XEmbed\n"
    );
}

static void on_dlg_response(GtkDialog* dlg, int res, gpointer user_data)
{
    switch(res)
    {
    default:
        gtk_main_quit();
    }
}

int main(int argc, char **argv)
{
    GError     *error = NULL;
    unsigned long wid=0;
    gtk_init( &argc, &argv );
    int i;
    for(i=0; i<argc; i++) {
        if(strcmp("-h", argv[i])==0 || strcmp("--help", argv[i])==0) {
            printHelp();
            return 0;
        }
        else if(strcmp("-V", argv[i])==0 || strcmp("--version", argv[i])==0) {
            printf("gtk3_preview version 1.0\n");
            return 0;
        } else if(argc>1)
            sscanf(argv[1], "%ld", &wid);
    }
    
    const char* ui_file = DATA_DIR "/preview.ui";
 
    GtkBuilder *builder = gtk_builder_new();
    if( ! gtk_builder_add_from_file( builder, ui_file, &error ) ) {
        g_warning( "%s", error->message );
        g_free( error );
        return 1;
    }
    
    GtkWidget *previewUI = GTK_WIDGET( gtk_builder_get_object( builder, "frame1" ) );
    gtk_builder_connect_signals( builder, NULL );
    
    /* a plug when embedded, a window when a window */
    GtkWidget* window;
    
    if(wid==0) {
        window = gtk_dialog_new();
        gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(window))), previewUI);
        g_signal_connect(window, "response", G_CALLBACK(on_dlg_response), NULL);
    } else {
        window = gtk_plug_new(wid);
        gtk_container_add (GTK_CONTAINER (window), previewUI);
    }
    
    g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &window);
    
    gtk_widget_show_all ( window );
    g_object_unref( G_OBJECT( builder ) );
    
    if(wid)
        fprintf(stderr, "--- is embedded gtk3: %d\n", gtk_plug_get_embedded(GTK_PLUG(window)));
    
    gtk_main();
    
    return 0;
}
