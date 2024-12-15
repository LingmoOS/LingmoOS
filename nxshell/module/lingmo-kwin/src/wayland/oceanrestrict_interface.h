/********************************************************************
Copyright 2020  pengwenhao <pengwenhao@uniontech.com>

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
*********************************************************************/
#pragma once

#include <QObject>

#include "lingmo-kwin_export.h"

struct ocean_restrict;
struct wl_resource;

namespace KWaylandServer
{

class Display;
class SurfaceInterface;

class OCEANRestrictInterface;
class OCEANRestrictInterfacePrivate;

class KWIN_EXPORT OCEANRestrictInterface : public QObject
{
    Q_OBJECT
public:
    explicit OCEANRestrictInterface(Display *display, QObject *parent = nullptr);
    virtual ~OCEANRestrictInterface();

    OCEANRestrictInterface *oceanRestrict() const;

    // static OCEANRestrictInterface *get(wl_resource *native);

    bool prohibitScreencast();
    QList<QByteArray> clientWhitelists();
    QList<int32_t> protectedWindowIdLists();
    void removeProtectedWindow(int32_t window);

Q_SIGNALS:

private:
    friend class OCEANRestrictInterfacePrivate;
    // explicit OCEANRestrictInterface(Display *display, QObject *parent = nullptr);
    QScopedPointer<OCEANRestrictInterfacePrivate> d;
};

}
