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

#ifndef LINGMO_PANEL_WINDOW_MANAGER_H
#define LINGMO_PANEL_WINDOW_MANAGER_H

#include <QObject>
#include <QQuickItem>
namespace LingmoUIQuick {
class WindowManagerPrivate;
class WindowManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isWaylandSession READ isWaylandSession CONSTANT)
public:
    static WindowManager *self();

    static QStringList windows();
    static QIcon windowIcon(const QString &wid);
    static QString windowTitle(const QString &wid);
    static bool skipTaskBar(const QString &wid);
    static QString windowGroup(const QString &wid);

    static bool isMaximizable(const QString& wid);
    static bool isMaximized(const QString& wid);
    static void maximizeWindow(const QString& wid);

    static bool isMinimizable(const QString& wid);
    static bool isMinimized(const QString& wid);
    static void minimizeWindow(const QString& wid);

    static bool isKeepAbove(const QString& wid);
    static void keepAboveWindow(const QString& wid);

    static bool isOnAllDesktops(const QString& wid);
    static bool isOnCurrentDesktop(const QString& wid);

    static void activateWindow(const QString& wid);
    static QString currentActiveWindow();
    static void closeWindow(const QString& wid);
    static void restoreWindow(const QString& wid);

    static bool isDemandsAttention(const QString& wid);
    static QString appId(const QString& wid);
    static bool isWaylandSession();
    Q_INVOKABLE static quint32 pid(const QString&wid);
    Q_INVOKABLE static QRect geometry(const QString& wid);
    Q_INVOKABLE void setStartupGeometry(const QString& wid, QQuickItem *item);
    Q_INVOKABLE void setMinimizedGeometry(const QString& wid, QQuickItem *item);
    Q_INVOKABLE void setMinimizedGeometry(const QStringList& winds, QQuickItem *item);
    Q_INVOKABLE void unsetMinimizedGeometry(const QString& wid, QQuickItem *item);
    Q_INVOKABLE void activateWindowView(const QStringList &wids);

    Q_SIGNALS:
        void currentDesktopChanged();
    void windowAdded(QString wid);
    void windowRemoved(QString wid);
    void titleChanged(QString wid);
    void iconChanged(QString wid);
    void skipTaskbarChanged(QString wid);
    void onAllDesktopsChanged(QString wid);
    void windowDesktopChanged(QString wid);
    void demandsAttentionChanged(QString wid);
    void geometryChanged(QString wid);
    void activeWindowChanged(QString id);
    void maximizedChanged(QString id);
    //xcb only
    void windowStateChanged(QString wid);

private:
    WindowManager(QObject *parent = nullptr);
};
}

#endif //LINGMO_PANEL_WINDOW_MANAGER_H
