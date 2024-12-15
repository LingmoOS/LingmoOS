// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-plugin-manager-v1.h"

#include <QtWaylandClient/private/qwaylandshellintegration_p.h>

namespace Plugin {
class PluginManagerIntegration : public QtWaylandClient::QWaylandShellIntegrationTemplate<PluginManagerIntegration>, public QtWayland::plugin_manager_v1
{
    Q_OBJECT
public:
    PluginManagerIntegration();
    ~PluginManagerIntegration() override;

    QtWaylandClient::QWaylandShellSurface *createShellSurface(QtWaylandClient::QWaylandWindow *window) override;

public:
    void requestMessage(const QString &plugin_id, const QString &item_key, const QString &msg);

Q_SIGNALS:
    void eventMessage(const QString &msg);
    void dockPositionChanged(uint32_t position);
    void dockColorThemeChanged(uint32_t colorType);

protected:
    void plugin_manager_v1_position_changed(uint32_t dock_position) override;
    void plugin_manager_v1_color_theme_changed(uint32_t dock_color_theme) override;
    void plugin_manager_v1_event_message(const QString &msg) override;

private:
    bool tryCreatePopupForSubWindow(QWindow *window);

private:
    uint32_t m_dockPosition;
    uint32_t m_dockColorType;
};
}
