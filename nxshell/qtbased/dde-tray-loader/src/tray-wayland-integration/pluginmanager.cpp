// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include "pluginmanager_p.h"
#include "pluginsurface_p.h"

namespace Plugin {
PluginManager::PluginManager(::wl_registry *registry, uint32_t id, uint32_t version)
    : QtWayland::plugin_manager_v1(registry, id, version)
    , QObject()
{
    init(registry, id, version);
}

PluginManager::~PluginManager()
{
}


void PluginManager::plugin_manager_v1_position_changed(uint32_t dock_position)
{
    if (dock_position != m_dockPosition) {
        m_dockPosition = dock_position;
        Q_EMIT dockPositionChanged(m_dockPosition);
    }
}

void PluginManager::plugin_manager_v1_color_theme_changed(uint32_t dock_color_theme)
{
    if (dock_color_theme != m_dockColorType) {
        m_dockColorType = dock_color_theme;
        Q_EMIT dockColorThemeChanged(m_dockColorType);
    }
}

void PluginManager::plugin_manager_v1_event_message(const QString &msg)
{
    // qInfo() << "plugin receive event message" << msg;
    Q_UNUSED(msg);
    Q_EMIT eventMessage(msg);
}

bool PluginManager::tryCreatePopupForSubWindow(QWindow *window)
{
    auto parentWindow = window->transientParent();
    if (!parentWindow)
        return false;

    if (auto plugin = EmbedPlugin::getWithoutCreating(parentWindow)) {
        auto pluginPopup = PluginPopup::get(window);
        pluginPopup->setPopupType(Plugin::PluginPopup::PopupTypeTooltip);
        pluginPopup->setPluginId(plugin->pluginId());
        pluginPopup->setItemKey(plugin->itemKey());
        pluginPopup->setX(parentWindow->x() + window->x());
        pluginPopup->setY(parentWindow->y() + window->y());
        return true;
    }

    if (auto plugin = PluginPopup::getWithoutCreating(parentWindow)) {
        auto pluginPopup = PluginPopup::get(window);
        pluginPopup->setPopupType(Plugin::PluginPopup::PopupTypeSubPopup);
        pluginPopup->setPluginId(plugin->pluginId());
        pluginPopup->setItemKey(plugin->itemKey());
        auto geometry = parentWindow->geometry();
        // TODO move to parentWindow's right position.
        pluginPopup->setX(geometry.right());
        pluginPopup->setY(window->y());
        return true;
    }

    return false;
}

void PluginManager::requestMessage(const QString &plugin_id, const QString &item_key, const QString &msg)
{
    request_message(plugin_id, item_key, msg);
}

QtWaylandClient::QWaylandShellSurface* PluginManager::createPluginSurface(QtWaylandClient::QWaylandWindow *window)
{
    if (EmbedPlugin::contains(window->window())) {
        return new PluginSurface(this, window);
    }

    if (PluginPopup::contains(window->window())) {
        return new PluginPopupSurface(this, window);
    }

    if (tryCreatePopupForSubWindow(window->window())) {
        return new PluginPopupSurface(this, window);
    }

    qWarning() << "create plugin surface failed, unknown window type";
    return nullptr;
}
}
