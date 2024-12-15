// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include "pluginsurface_p.h"
#include "pluginmanagerintegration_p.h"

#include "qwayland-plugin-manager-v1.h"

#include <QTimer>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

namespace Plugin {
PluginSurface::PluginSurface(PluginManagerIntegration *manager, QtWaylandClient::QWaylandWindow *window)
    : QtWaylandClient::QWaylandShellSurface(window)
    , QtWayland::plugin()
    , m_plugin(EmbedPlugin::get(window->window()))
    , m_window(window->window())
{
    init(manager->create_plugin(m_plugin->pluginId(), m_plugin->itemKey(), m_plugin->displayName(), m_plugin->pluginFlags(), m_plugin->pluginType(), m_plugin->pluginSizePolicy(), window->wlSurface()));
    oceanui_icon(m_plugin->oceanuiIcon());

    source_size(m_window->width(), m_window->height());
    connect(m_window, &QWindow::widthChanged, this, [this] (int width) {
        source_size(width, m_window->height());
    });

    connect(m_window, &QWindow::heightChanged, this, [this] (int height) {
        source_size(m_window->width(), height);
    });

    connect(manager, &PluginManagerIntegration::dockPositionChanged, m_plugin, &EmbedPlugin::dockPositionChanged);
    connect(manager, &PluginManagerIntegration::dockColorThemeChanged, m_plugin, &EmbedPlugin::dockColorThemeChanged);
    connect(manager, &PluginManagerIntegration::eventMessage, m_plugin, &EmbedPlugin::eventMessage);

    connect(m_plugin, &EmbedPlugin::requestMessage, manager, [manager, this](const QString &msg) {
        manager->requestMessage(m_plugin->pluginId(), m_plugin->itemKey(), msg);
    });

    connect(m_plugin, &EmbedPlugin::pluginRecvMouseEvent, this, [this] (int type){
        mouse_event(type);
    });

    connect(m_plugin, &EmbedPlugin::pluginRequestShutdown, this, [this] {
        request_shutdown();
    });
}

PluginSurface::~PluginSurface()
{
    destroy();
}

void PluginSurface::plugin_close()
{
    // it would be delete this object directly.
    m_window->close();
}

void PluginSurface::plugin_geometry(int32_t x, int32_t y, int32_t width, int32_t height)
{
    QRect rect(x, y, width, height);
    if (width <= 0)
        rect.setWidth(m_window->width());
    if (height <= 0)
        rect.setHeight(m_window->height());

    m_plugin->setPluginPos(QPoint(x, y));
    Q_EMIT m_plugin->eventGeometry(rect);
}

void PluginSurface::plugin_margin(int32_t spacing)
{
    if (spacing > 0) {
        Q_EMIT m_plugin->contentMarginChanged(spacing);
    }
}

void PluginSurface::plugin_raw_global_pos(int32_t x, int32_t y)
{
    m_plugin->setRawGlobalPos(QPoint(x, y));
}

PluginPopupSurface::PluginPopupSurface(PluginManagerIntegration *manager, QtWaylandClient::QWaylandWindow *window)
    : QtWaylandClient::QWaylandShellSurface(window)
    , QtWayland::plugin_popup()
    , m_popup(PluginPopup::get(window->window()))
    , m_window(window->window())
    , m_dirtyTimer(new QTimer(this))
{
    init(manager->create_popup_at(m_popup->pluginId(), m_popup->itemKey(), m_popup->popupType(), m_popup->x(), m_popup->y(), window->wlSurface()));

    // merge multi signal of position changed to one.
    m_dirtyTimer->setInterval(0);
    m_dirtyTimer->setSingleShot(true);

    source_size(m_window->width(), m_window->height());
    connect(m_window, &QWindow::widthChanged, this, [this] (int width) {
        source_size(width, m_window->height());
    });

    connect(m_window, &QWindow::heightChanged, this, [this] (int height) {
        source_size(m_window->width(), height);
    });

    connect(m_popup, &PluginPopup::xChanged, this, &PluginPopupSurface::dirtyPosition);
    connect(m_popup, &PluginPopup::yChanged, this, &PluginPopupSurface::dirtyPosition);
    connect(m_dirtyTimer, &QTimer::timeout, this, [this]{
        set_position(m_popup->x(), m_popup->y());
    });
}

PluginPopupSurface::~PluginPopupSurface()
{
    destroy();
}

void PluginPopupSurface::plugin_popup_close()
{
    // it would be delete this object directly.
    m_window->close();
}

void PluginPopupSurface::plugin_popup_geometry(int32_t x, int32_t y, int32_t width, int32_t height)
{
    auto plugin = PluginPopup::get(m_window);
    auto rect = QRect(x, y, width, height);
    if (rect.width() <= 0)
        rect.setWidth(m_window->width());

    if (rect.height() <= 0)
        rect.setHeight(m_window->height());

    m_popup->setPluginPos(QPoint(x, y));
    if (plugin) {
        Q_EMIT plugin->eventGeometry(rect);
    }
}

void PluginPopupSurface::dirtyPosition()
{
    m_dirtyTimer->start();
}

}
