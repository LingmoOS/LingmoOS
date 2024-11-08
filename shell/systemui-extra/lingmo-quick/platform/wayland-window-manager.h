/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_PANEL_WAYLAND_WINDOW_MANAGER_H
#define LINGMO_PANEL_WAYLAND_WINDOW_MANAGER_H

#include "abstract-window-manager.h"
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/plasmavirtualdesktop.h>
#include <windowmanager/lingmowindowmanagement.h>
#include "registry.h"
#include <QMap>
namespace LingmoUIQuick {
class WaylandWindowManager : public AbstractWindowManager
{
    Q_OBJECT
public:
    explicit WaylandWindowManager(QObject *parent = nullptr);
    ~WaylandWindowManager();

    QStringList windows() override;
    QIcon windowIcon(const QString &wid) override;
    QString windowTitle(const QString &wid) override;
    bool skipTaskBar(const QString &wid) override;
    QString windowGroup(const QString &wid) override;

    bool isMaximizable(const QString &wid) override;
    bool isMaximized(const QString& wid) override;
    void maximizeWindow(const QString& wid) override;

    bool isMinimizable(const QString &wid) override;
    bool isMinimized(const QString& wid) override;
    void minimizeWindow(const QString& wid) override;

    bool isKeepAbove(const QString& wid) override;
    void keepAboveWindow(const QString& wid) override;

    bool isOnAllDesktops(const QString& wid) override;
    bool isOnCurrentDesktop(const QString& wid) override;

    void activateWindow(const QString& wid) override;
    QString currentActiveWindow() override;
    void closeWindow(const QString& wid) override;
    void restoreWindow(const QString& wid) override;

    bool isDemandsAttention(const QString& wid) override;
    quint32 pid(const QString& wid) override;
    QString appId(const QString& wid) override;

    QRect geometry(const QString& wid) override;

    void setStartupGeometry(const QString& wid, QQuickItem *item) override;
    void setMinimizedGeometry(const QString& wid, QQuickItem *item) override;
    void unsetMinimizedGeometry(const QString& wid, QQuickItem *item) override;

    void activateWindowView(const QStringList &wids) override;

private:
    void addWindow(LingmoUIWindow *window);
    void desktopCreated(const QString &id, quint32 position);
    void desktopRemoved(const QString &id);
    void setCurrentDesktop(const QString &id);
    LingmoUIQuick::WaylandClient::Registry *m_registry = nullptr;
    LingmoUIWindowManagement *m_windowManagement = nullptr;
    KWayland::Client::PlasmaVirtualDesktopManagement *m_virtualDesktopManagement = nullptr;
    KWayland::Client::ConnectionThread *m_connection = nullptr;

    QMap<QString, LingmoUIWindow *> m_uuidToWindow;
    QList<LingmoUIWindow *> m_windows;
    QStringList m_desktops;
    QString m_currentDesktop;
};
}

#endif //LINGMO_PANEL_WAYLAND_WINDOW_MANAGER_H
