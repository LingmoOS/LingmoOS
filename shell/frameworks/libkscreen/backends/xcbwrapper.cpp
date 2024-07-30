/*
    K Win - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2012, 2013 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xcbwrapper.h"

static xcb_connection_t *sXRandR11XCBConnection = nullptr;

xcb_connection_t *XCB::connection()
{
    // Use our own connection to make sure that we won't mess up Qt's connection
    // if something goes wrong on our side.
    if (sXRandR11XCBConnection == nullptr) {
        sXRandR11XCBConnection = xcb_connect(nullptr, nullptr);
    }
    return sXRandR11XCBConnection;
}

void XCB::closeConnection()
{
    if (sXRandR11XCBConnection) {
        xcb_disconnect(sXRandR11XCBConnection);
        sXRandR11XCBConnection = nullptr;
    }
}

xcb_screen_t *XCB::screenOfDisplay(xcb_connection_t *c, int screen)
{
    for (auto iter = xcb_setup_roots_iterator(xcb_get_setup(c)); iter.rem; --screen, xcb_screen_next(&iter)) {
        if (screen == 0) {
            return iter.data;
        }
    }

    return nullptr;
}

XCB::GrabServer::GrabServer()
{
    xcb_grab_server(connection());
}

XCB::GrabServer::~GrabServer()
{
    xcb_ungrab_server(connection());
    xcb_flush(connection());
}
