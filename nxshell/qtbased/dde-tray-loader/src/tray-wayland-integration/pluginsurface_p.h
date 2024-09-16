// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "plugin.h"
#include "qwayland-plugin-manager-v1.h"

#include <QtWaylandClient/private/qwaylandshellsurface_p.h>

namespace Plugin {

class PluginManager;
class PluginSurface : public QtWaylandClient::QWaylandShellSurface, public QtWayland::plugin
{
    Q_OBJECT

public:
    PluginSurface(PluginManager *manager, QtWaylandClient::QWaylandWindow *window);
    ~PluginSurface() override;

protected:
    virtual void plugin_close() override;
    virtual void plugin_geometry(int32_t x, int32_t y, int32_t width, int32_t height) override;
    virtual void plugin_margin(int32_t spacing) override;
    virtual void plugin_raw_global_pos(int32_t x, int32_t y) override;

private:
    EmbedPlugin* m_plugin;
    QWindow* m_window;
};

class PluginPopupSurface : public QtWaylandClient::QWaylandShellSurface, public QtWayland::plugin_popup
{
    Q_OBJECT

public:
    PluginPopupSurface(PluginManager *manager, QtWaylandClient::QWaylandWindow *window);
    ~PluginPopupSurface() override;

protected:
    virtual void plugin_popup_close() override;
    virtual void plugin_popup_geometry(int32_t x, int32_t y, int32_t width, int32_t height) override;

private:
    void dirtyPosition();

private:
    PluginPopup* m_popup;
    QWindow* m_window;
    QTimer *m_dirtyTimer = nullptr;
};

}
