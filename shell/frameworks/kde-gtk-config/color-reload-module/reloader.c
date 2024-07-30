/*
 * SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <gio/gio.h>
#include <gdk/gdk.h>

gchar *path;
GFile *file;
GFileMonitor *mon;
GtkCssProvider *provider;

void reload_colours() {
    if (provider != NULL) {
        gtk_style_context_remove_provider_for_screen(
            gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(provider)
        );
        g_clear_object(&provider);
    }
    provider = gtk_css_provider_new();
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER (provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER+1
    );
    gtk_css_provider_load_from_file(
        provider,
        file,
        NULL
    );
}

__attribute__((visibility("default"))) void gtk_module_init(gint *argc, gchar ***argv[]) {
    path = g_strconcat(g_get_user_config_dir(), "/gtk-3.0/colors.css", NULL);
    file = g_file_new_for_path(path);

    mon = g_file_monitor_file(
        file,
        G_FILE_MONITOR_NONE,
        NULL,
        NULL
    );

    g_signal_connect(mon, "changed", G_CALLBACK(reload_colours), NULL);
}
