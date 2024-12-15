// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanagerintegration_p.h"
#include "plugin.h"
#include "pluginsurface_p.h"

#include <qwayland-plugin-manager-v1.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

namespace Plugin {
PluginManagerIntegration::PluginManagerIntegration()
    : QWaylandShellIntegrationTemplate<PluginManagerIntegration>(1)
{
    qInfo() << "PluginManagerIntegration22";
}

PluginManagerIntegration::~PluginManagerIntegration()
{

}

QtWaylandClient::QWaylandShellSurface *PluginManagerIntegration::createShellSurface(QtWaylandClient::QWaylandWindow *window)
{
    qInfo() << "PluginManagerIntegration2::createShellSurface";
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


void PluginManagerIntegration::requestMessage(const QString &plugin_id, const QString &item_key, const QString &msg)
{
    request_message(plugin_id, item_key, msg);
}

void PluginManagerIntegration::plugin_manager_v1_position_changed(uint32_t dock_position)
{
    if (dock_position != m_dockPosition) {
        m_dockPosition = dock_position;
        Q_EMIT dockPositionChanged(m_dockPosition);
    }
}

void PluginManagerIntegration::plugin_manager_v1_color_theme_changed(uint32_t dock_color_theme)
{
    if (dock_color_theme != m_dockColorType) {
        m_dockColorType = dock_color_theme;
        Q_EMIT dockColorThemeChanged(m_dockColorType);
    }
}

void PluginManagerIntegration::plugin_manager_v1_event_message(const QString &msg)
{
    // qInfo() << "plugin receive event message" << msg;
    Q_UNUSED(msg);
    Q_EMIT eventMessage(msg);
}

bool PluginManagerIntegration::tryCreatePopupForSubWindow(QWindow *window)
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
}
