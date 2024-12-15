// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtWaylandCompositor/private/qwaylandquickshellsurfaceitem_p.h>

class PluginSurface;
class PluginPopup;

class PluginManagerIntegration : public QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    PluginManagerIntegration(QWaylandQuickShellSurfaceItem *item);
    ~PluginManagerIntegration() override;

private Q_SLOTS:
    void handleDockPluginSurfaceDestroyed();

private:
    QWaylandQuickShellSurfaceItem *m_item = nullptr;
    PluginSurface *m_pluginSurface = nullptr;
};

class PluginPopupIntegration : public QWaylandQuickShellIntegration
{
    Q_OBJECT
public:
    PluginPopupIntegration(QWaylandQuickShellSurfaceItem *item);
    ~PluginPopupIntegration() override;

private Q_SLOTS:
    void handleDockPluginSurfaceDestroyed();

private:
    QWaylandQuickShellSurfaceItem *m_item = nullptr;
    PluginPopup *m_pluginSurface = nullptr;
};
