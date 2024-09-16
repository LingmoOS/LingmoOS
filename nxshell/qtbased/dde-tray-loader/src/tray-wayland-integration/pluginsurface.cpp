// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include "pluginsurface_p.h"
#include "pluginmanager_p.h"

#include "qwayland-plugin-manager-v1.h"

#include <QTimer>

namespace Plugin {
PluginSurface::PluginSurface(PluginManager *manager, QtWaylandClient::QWaylandWindow *window)
    : QtWaylandClient::QWaylandShellSurface(window)
    , QtWayland::plugin()
    , m_plugin(EmbedPlugin::get(window->window()))
    , m_window(window->window())
{
    init(manager->create_plugin(m_plugin->pluginId(), m_plugin->itemKey(), m_plugin->displayName(), m_plugin->pluginFlags(), m_plugin->pluginType(), m_plugin->pluginSizePolicy(), window->wlSurface()));
    dcc_icon(m_plugin->dccIcon());
    connect(manager, &PluginManager::dockPositionChanged, m_plugin, &EmbedPlugin::dockPositionChanged);
    connect(manager, &PluginManager::dockColorThemeChanged, m_plugin, &EmbedPlugin::dockColorThemeChanged);
    connect(manager, &PluginManager::eventMessage, m_plugin, &EmbedPlugin::eventMessage);

    connect(m_plugin, &EmbedPlugin::requestMessage, manager, [manager, this](const QString &msg) {
        manager->requestMessage(m_plugin->pluginId(), m_plugin->itemKey(), msg);
    });

    connect(m_plugin, &EmbedPlugin::pluginRecvMouseEvent, this, [this] (int type){
        mouse_event(type);
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

    m_window->setGeometry(rect);
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

PluginPopupSurface::PluginPopupSurface(PluginManager *manager, QtWaylandClient::QWaylandWindow *window)
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

    m_window->setGeometry(rect);
    if (plugin) {
        Q_EMIT plugin->eventGeometry(rect);
    }
}

void PluginPopupSurface::dirtyPosition()
{
    m_dirtyTimer->start();
}

}
