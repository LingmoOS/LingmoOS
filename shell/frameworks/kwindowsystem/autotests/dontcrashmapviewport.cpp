/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include <netwm.h>
#include <xcb/xcb.h>

int main(int, char **)
{
    xcb_connection_t *c = xcb_connect(nullptr, nullptr);
    Q_ASSERT(c);
    Q_ASSERT(!xcb_connection_has_error(c));
    NETRootInfo rootInfo(c, NET::CurrentDesktop);
    rootInfo.currentDesktop(true);
    rootInfo.currentDesktop(false);

    xcb_disconnect(c);

    return 0;
}
