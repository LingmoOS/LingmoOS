/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <QCoreApplication>
#include <xcb/xcb.h>

xcb_screen_t *defaultScreen(xcb_connection_t *c, int screen)
{
    for (auto it = xcb_setup_roots_iterator(xcb_get_setup(c)); it.rem; --screen, xcb_screen_next(&it)) {
        if (screen == 0) {
            return it.data;
        }
    }

    return nullptr;
}

xcb_window_t rootWindow(xcb_connection_t *c, int screen)
{
    xcb_screen_t *s = defaultScreen(c, screen);
    if (!s) {
        return XCB_WINDOW_NONE;
    }
    return s->root;
}

/**
 * This app grabs the pointer from X.
 * It is used from ksldtest to verify we report if grabbing failed when the pointer has been grabbed by another
 * X Client. It needs to be another application, otherwise the grab cannot fail.
 **/
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    // connect to xcb
    int screen = 0;
    xcb_connection_t *c = xcb_connect(nullptr, &screen);
    Q_ASSERT(c);

    const uint8_t events = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION;
    xcb_grab_pointer(c, 1, rootWindow(c, screen), events, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_WINDOW_NONE, XCB_CURSOR_NONE, XCB_CURRENT_TIME);
    xcb_flush(c);

    const int exitCode = app.exec();

    xcb_ungrab_pointer(c, XCB_CURRENT_TIME);
    xcb_flush(c);
    xcb_disconnect(c);

    return exitCode;
}
