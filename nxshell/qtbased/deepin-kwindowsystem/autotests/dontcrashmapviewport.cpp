/*
 Copyright 2016 Martin Gräßlin <mgraesslin@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
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

