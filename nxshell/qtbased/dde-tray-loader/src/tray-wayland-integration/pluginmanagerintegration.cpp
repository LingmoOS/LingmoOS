// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanagerintegration_p.h"
#include <qwayland-plugin-manager-v1.h>

namespace Plugin {
PluginManagerIntegration::PluginManagerIntegration()
    : QWaylandShellIntegration()
{
}

PluginManagerIntegration::~PluginManagerIntegration()
{
}

bool PluginManagerIntegration::initialize(QtWaylandClient::QWaylandDisplay *display)
{
    QWaylandShellIntegration::initialize(display);
    display->addRegistryListener(registryPluginManager, this);
    return m_pluginManager != nullptr;
}

QtWaylandClient::QWaylandShellSurface *PluginManagerIntegration::createShellSurface(QtWaylandClient::QWaylandWindow *window)
{
    return m_pluginManager->createPluginSurface(window);
}

void PluginManagerIntegration::registryPluginManager(void *data, struct wl_registry *registry, uint32_t id, const QString &interface, uint32_t version)
{
    PluginManagerIntegration *shell = static_cast<PluginManagerIntegration *>(data);
    if (interface == plugin_manager_v1_interface.name) {
        shell->m_pluginManager.reset(new PluginManager(registry, id, std::min(version, 1u)));   
    }
}
}
