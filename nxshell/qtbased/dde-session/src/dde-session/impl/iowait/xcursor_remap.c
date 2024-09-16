// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <gio/gio.h>

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xfixes.h>

#include "xcursor_remap.h"

static int xc_remap(char *theme, char *src, char *dst, int size);

static XcursorImages*
xc_load_images(const char *theme, const char *name, int size)
{
    printf("Will load cursor file: %s - %s\n", theme, name);
    return XcursorLibraryLoadImages(name, theme, size);
}

static void
xc_change_cursor_by_name(Display *dpy, const char *theme,
                 const char *src, const char* dst, int size)
{
    if (size == -1) {
        size = XcursorGetDefaultSize(dpy);
    }

    // load cursor images
    XcursorImages *dst_images = xc_load_images(theme, dst, size);
    if (!dst_images) {
        XcursorImagesDestroy(dst_images);
        fprintf(stderr, "Failed to load cursor images: %s/%s\n", src, dst);
        return;
    }

    Cursor dst_cursor = XcursorImagesLoadCursor(dpy, dst_images);

    Window root = XDefaultRootWindow(dpy);
    XUndefineCursor(dpy, root);
    XDefineCursor(dpy, root, dst_cursor);
    printf("Will change cursor: %s --> %lu(%s)\n", src, dst_cursor, dst);
    XFixesChangeCursorByName(dpy, dst_cursor, src);

    XcursorImagesDestroy(dst_images);
    XFreeCursor(dpy, dst_cursor);
    return ;
}

static int
xc_remap(char *theme, char *src, char *dst, int size)
{
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Failed to open display\n");
        return -1;
    }

    xc_change_cursor_by_name(dpy, theme, src, dst, size);

    XCloseDisplay(dpy);
    return 0;
}

int
xc_left_ptr_to_watch(int enabled)
{
    char *theme = "deepin";
    int size = 24;
    double scale = 1.0;

    GSettings *s = g_settings_new("com.deepin.xsettings");
    if (s) {
        theme = g_settings_get_string(s, "gtk-cursor-theme-name");
        size = g_settings_get_int(s, "gtk-cursor-theme-size");
        scale = g_settings_get_double(s, "scale-factor");
        g_object_unref(s);
    }

    // if 1.7 < scale < 2, scale = 2
    int tmp = (int)(trunc((scale+0.3)*10) / 10);
    if (tmp < 1) {
        tmp = 1;
    }
    size *= tmp;

    if (enabled) {
        return xc_remap(theme, "left_ptr", "left_ptr_watch", size);
    } else {
        return xc_remap(theme, "left_ptr_watch", "left_ptr", size);
    }
}
