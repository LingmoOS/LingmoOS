// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *box;
    GtkWidget *input_box;
    GtkWidget *button_box;
    GtkWidget *label;
    GtkWidget *entry;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "dimDemo");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    gtk_window_set_child(GTK_WINDOW(window), box);

    input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(input_box, TRUE);
    gtk_widget_set_halign(input_box, GTK_ALIGN_CENTER);

    label = gtk_label_new("请输入：");
    entry = gtk_entry_new();

    gtk_box_append(GTK_BOX(input_box), label);
    gtk_box_set_spacing(GTK_BOX(input_box), 5);
    gtk_box_append(GTK_BOX(input_box), entry);
    gtk_box_append(GTK_BOX(box), input_box);

    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    button = gtk_button_new_with_label("Close");

    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_window_destroy), window);

    gtk_box_append(GTK_BOX(button_box), button);
    gtk_box_set_spacing(GTK_BOX(box), 10);
    gtk_box_append(GTK_BOX(box), button_box);

    gtk_widget_show(window);
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("dim.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}