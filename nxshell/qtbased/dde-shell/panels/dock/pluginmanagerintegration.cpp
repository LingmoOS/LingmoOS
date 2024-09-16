// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanagerextension_p.h"
#include "pluginmanagerintegration_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandQuickShellSurfaceItem>

PluginManagerIntegration::PluginManagerIntegration(QWaylandQuickShellSurfaceItem *item)
    : QWaylandQuickShellIntegration(item)
    , m_item(item)
    , m_pluginSurface(qobject_cast<PluginSurface *>(item->shellSurface()))
{
    m_item->setSurface(m_pluginSurface->surface());
    connect(m_pluginSurface, &QWaylandShellSurface::destroyed,
            this, &PluginManagerIntegration::handleDockPluginSurfaceDestroyed);
}

PluginManagerIntegration::~PluginManagerIntegration()
{
    m_item->setSurface(nullptr);
}

void PluginManagerIntegration::handleDockPluginSurfaceDestroyed()
{
    m_pluginSurface = nullptr;
}


PluginPopupIntegration::PluginPopupIntegration(QWaylandQuickShellSurfaceItem *item)
    : QWaylandQuickShellIntegration(item)
    , m_item(item)
    , m_pluginSurface(qobject_cast<PluginPopup *>(item->shellSurface()))
{
    m_item->setSurface(m_pluginSurface->surface());
    connect(m_pluginSurface, &QWaylandShellSurface::destroyed,
            this, &PluginPopupIntegration::handleDockPluginSurfaceDestroyed);
}

PluginPopupIntegration::~PluginPopupIntegration()
{
    m_item->setSurface(nullptr);
}

void PluginPopupIntegration::handleDockPluginSurfaceDestroyed()
{
    m_pluginSurface = nullptr;
}
