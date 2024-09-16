/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "utils.h"
#include <QX11Info>

namespace KWin
{
xcb_atom_t Utils::internAtom(const char *name, bool only_if_exists)
{
    if (!name || *name == 0)
        return XCB_NONE;

    if (!QX11Info::isPlatformX11())
        return XCB_NONE;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(QX11Info::connection(), only_if_exists, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(QX11Info::connection(), cookie, 0);

    if (!reply)
        return XCB_NONE;

    xcb_atom_t atom = reply->atom;
    free(reply);

    return atom;
}

QByteArray Utils::readWindowProperty(xcb_window_t win, xcb_atom_t atom, xcb_atom_t type)
{
    if (win == XCB_WINDOW_NONE) {
        return QByteArray();
    }
    xcb_get_property_cookie_t cookie = xcb_get_property(QX11Info::connection(), false, win,
                                                            atom, type, 0, 256);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(QX11Info::connection(), cookie, NULL);
    QByteArray data;
    if (reply) {
        int remaining = 0;
        if (reply->type == type) {
            int len = xcb_get_property_value_length(reply);
            char *datas = (char *)xcb_get_property_value(reply);
            data.append(datas, len);
        }

        free(reply);
    }

    return data;
}

}