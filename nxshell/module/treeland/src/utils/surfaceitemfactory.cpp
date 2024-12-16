// Copyright (C) 2024 Yicheng Zhong <zhongyicheng@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "surfaceitemfactory.h"

#include <QQmlContext>
#include <wxwaylandsurface.h>
#include <wxdgsurface.h>
#include <wxwaylandsurfaceitem.h>
#include <wxdgsurfaceitem.h>

SurfaceItemFactory::SurfaceItemFactory(QQuickItem *parent)
    : QQuickItem(parent)
{
}
constexpr auto typePropName = "surfType";
void SurfaceItemFactory::classBegin()
{
    Q_ASSERT(qmlContext(this));
    Q_ASSERT(qmlContext(this)->contextProperty(typePropName).isValid());
    if (auto c = qmlContext(this); c->contextProperty(typePropName).toString() == "xdg") {
        m_surfaceItem = new Waylib::Server::WXdgSurfaceItem(this);
    } else {
        m_surfaceItem = new Waylib::Server::WXWaylandSurfaceItem(this);
    }
}
