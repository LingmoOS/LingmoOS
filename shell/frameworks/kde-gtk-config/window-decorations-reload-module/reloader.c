/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <gdk/gdk.h>
#include <gio/gio.h>
#include <glib.h>
#include <gtk/gtk.h>

#define UNUSED(x) (void)(x)

gchar *window_decorations_css_path;
GFile *window_decorations_css_file;

GFileMonitor *window_decorations_css_monitor;

GtkCssProvider *css_provider;

void manage_css_provider(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, gpointer user_data);
void reload_css_provider();
void remove_css_provider();
int theme_is_breeze();
void theme_changed(GtkSettings *settings, GParamSpec *param_spec, void *user_data);

__attribute__((visibility("default"))) void gtk_module_init(gint *argc, gchar ***argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    window_decorations_css_path = g_strconcat(g_get_user_config_dir(), "/gtk-3.0/window_decorations.css", NULL);
    window_decorations_css_file = g_file_new_for_path(window_decorations_css_path);

    window_decorations_css_monitor = g_file_monitor_file(window_decorations_css_file, G_FILE_MONITOR_NONE, NULL, NULL);

    css_provider = NULL;

    g_signal_connect(window_decorations_css_monitor, "changed", G_CALLBACK(manage_css_provider), NULL);
    g_signal_connect(gtk_settings_get_default(), "notify::gtk-theme-name", G_CALLBACK(theme_changed), NULL);

    reload_css_provider();
}

void manage_css_provider(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, gpointer user_data)
{
    UNUSED(monitor);
    UNUSED(file);
    UNUSED(other_file);
    UNUSED(user_data);

    if (event_type == G_FILE_MONITOR_EVENT_CREATED || event_type == G_FILE_MONITOR_EVENT_CHANGED) {
        reload_css_provider();
    } else if (event_type == G_FILE_MONITOR_EVENT_DELETED) {
        remove_css_provider();
    }
}

void reload_css_provider()
{
    remove_css_provider();

    if (!theme_is_breeze()) {
        return;
    }

    css_provider = gtk_css_provider_new();

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER + 1);
    if (g_file_query_exists(window_decorations_css_file, NULL)) {
        gtk_css_provider_load_from_file(css_provider, window_decorations_css_file, NULL);
    }
}

void remove_css_provider()
{
    if (!css_provider) {
        return;
    }
    gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css_provider));
    g_clear_object(&css_provider);
}

int theme_is_breeze()
{
    GtkSettings *settings = gtk_settings_get_default();
    char *theme_name = NULL;
    g_object_get(settings, "gtk-theme-name", &theme_name, NULL);
    if (!theme_name) {
        return 0;
    }
    int ret = !strcmp(theme_name, "Breeze");
    g_free(theme_name);
    return ret;
}

void theme_changed(GtkSettings *settings, GParamSpec *param_spec, void *user_data)
{
    UNUSED(settings);
    UNUSED(param_spec);
    UNUSED(user_data);
    reload_css_provider();
}
