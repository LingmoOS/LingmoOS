/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2015  Marco Martin <mart@kde.org>

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
#include "strut_interface.h"
#include "display.h"
#include "surface_interface_p.h"
#include <qwayland-server-wayland.h>
#include <qwayland-server-strut.h>

namespace KWaylandServer
{

static const quint32 s_version = 1;

class StrutInterfacePrivate: public QtWaylandServer::com_deepin_kwin_strut
{
public:
    StrutInterfacePrivate(StrutInterface *q, Display *d);
    StrutInterface *q;

private:
   void com_deepin_kwin_strut_set_strut_partial(Resource *resource,
                                                struct ::wl_resource *surface,
                                                int32_t left,
                                                int32_t right,
                                                int32_t top,
                                                int32_t bottom,
                                                int32_t left_start_y,
                                                int32_t left_end_y,
                                                int32_t right_start_y,
                                                int32_t right_end_y,
                                                int32_t top_start_x,
                                                int32_t top_end_x,
                                                int32_t bottom_start_x,
                                                int32_t bottom_end_x) override;
};

StrutInterfacePrivate::StrutInterfacePrivate(StrutInterface *q, Display *d)
    : QtWaylandServer::com_deepin_kwin_strut(*d, s_version)
    , q(q)
{
}

void StrutInterfacePrivate::com_deepin_kwin_strut_set_strut_partial(Resource *resource,
                                                                    struct ::wl_resource *surface,
                                                                    int32_t left,
                                                                    int32_t right,
                                                                    int32_t top,
                                                                    int32_t bottom,
                                                                    int32_t left_start_y,
                                                                    int32_t left_end_y,
                                                                    int32_t right_start_y,
                                                                    int32_t right_end_y,
                                                                    int32_t top_start_x,
                                                                    int32_t top_end_x,
                                                                    int32_t bottom_start_x,
                                                                    int32_t bottom_end_x)
{
    Q_UNUSED(resource);

    struct deepinKwinStrut kwinStrut(left,
                                     right,
                                     top,
                                     bottom,
                                     left_start_y,
                                     left_end_y,
                                     right_start_y,
                                     right_end_y,
                                     top_start_x,
                                     top_end_x,
                                     bottom_start_x,
                                     bottom_end_x);
    SurfaceInterface *si = SurfaceInterface::get(surface);

    Q_EMIT q->setStrut(si, kwinStrut);
}

StrutInterface *StrutInterface::get(wl_resource *native)
{
    if (auto surfacePrivate = resource_cast<StrutInterfacePrivate *>(native)) {
        return surfacePrivate->q;
    }
    return nullptr;
}

StrutInterface::StrutInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new StrutInterfacePrivate(this, display))
{
}

StrutInterface::~StrutInterface() = default;

}
