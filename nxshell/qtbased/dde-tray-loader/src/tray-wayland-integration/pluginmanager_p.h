// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <qwayland-plugin-manager-v1.h>

#include <QtWaylandClient/private/qwaylandwindow_p.h>

namespace Plugin {

class PluginSurface;
class PluginManager : public QObject, public QtWayland::plugin_manager_v1
{
    Q_OBJECT

public:
    PluginManager(::wl_registry *registry, uint32_t id, uint32_t version);
    ~PluginManager();

    QtWaylandClient::QWaylandShellSurface* createPluginSurface(QtWaylandClient::QWaylandWindow *window);

    void requestMessage(const QString &plugin_id, const QString &item_key, const QString &msg);

Q_SIGNALS:
    void eventMessage(const QString &msg);
    void dockPositionChanged(uint32_t position);
    void dockColorThemeChanged(uint32_t colorType);

protected:
    virtual void plugin_manager_v1_position_changed(uint32_t dock_position);
    virtual void plugin_manager_v1_color_theme_changed(uint32_t dock_color_theme);
    virtual void plugin_manager_v1_event_message(const QString &msg);

private:
    bool tryCreatePopupForSubWindow(QWindow *window);

private:
    uint32_t m_dockPosition;
    uint32_t m_dockColorType;
};
}
