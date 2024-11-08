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

#ifndef LINGMO_PANEL_ABSTRACT_WINDOW_MANAGER_H
#define LINGMO_PANEL_ABSTRACT_WINDOW_MANAGER_H
#include <QObject>
#include <QQuickItem>
namespace LingmoUIQuick {

class AbstractWindowManager : public QObject
{
    Q_OBJECT
public:
    explicit AbstractWindowManager(QObject* parent=nullptr): QObject(parent){}
    virtual ~AbstractWindowManager() = default;

    virtual QStringList windows() = 0;
    virtual QIcon windowIcon(const QString &wid) = 0;
    virtual QString windowTitle(const QString &wid) = 0;
    virtual bool skipTaskBar(const QString &wid) = 0;
    virtual QString windowGroup(const QString &wid) = 0;

    virtual bool isMaximizable(const QString& wid) = 0;
    virtual bool isMaximized(const QString& wid) = 0;
    virtual void maximizeWindow(const QString& wid) = 0;

    virtual bool isMinimizable(const QString& wid) = 0;
    virtual bool isMinimized(const QString& wid) = 0;
    virtual void minimizeWindow(const QString& wid) = 0;

    virtual bool isKeepAbove(const QString& wid) = 0;
    virtual void keepAboveWindow(const QString& wid) = 0;

    virtual bool isOnAllDesktops(const QString& wid) = 0;
    virtual bool isOnCurrentDesktop(const QString& wid) = 0;

    virtual void activateWindow(const QString& wid) = 0;
    virtual QString currentActiveWindow() = 0;
    virtual void closeWindow(const QString& wid) = 0;
    virtual void restoreWindow(const QString& wid) = 0;

    virtual bool isDemandsAttention(const QString& wid) = 0;
    virtual quint32 pid(const QString& wid) = 0;
    virtual QString appId(const QString& wid) = 0;

    virtual QRect geometry(const QString& wid) = 0;
    virtual void setStartupGeometry(const QString& wid, QQuickItem *item) = 0;
    virtual void setMinimizedGeometry(const QString& wid, QQuickItem *item)  = 0;
    virtual void unsetMinimizedGeometry(const QString& wid, QQuickItem *item) = 0;

    virtual void activateWindowView(const QStringList &wids);


Q_SIGNALS:
    void currentDesktopChanged();
    void windowDesktopChanged(QString wid);
    void windowAdded(QString wid);
    void windowRemoved(QString wid);
    void titleChanged(QString wid);
    void iconChanged(QString wid);
    void skipTaskbarChanged(QString wid);
    void onAllDesktopsChanged(QString wid);
    void demandsAttentionChanged(QString wid);
    void geometryChanged(QString wid);
    void activeWindowChanged(QString id);
    void maximizedChanged(QString wid);

    //xcb only
    void windowStateChanged(QString wid);
};
}

#endif //LINGMO_PANEL_ABSTRACT_WINDOW_MANAGER_H
