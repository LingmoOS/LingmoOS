// Copyright 2015  Marco Martin <mart@kde.org>
// Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

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
