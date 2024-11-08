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
#ifndef LINGMO_PANEL_XCB_WINDOW_MANAGER_H
#define LINGMO_PANEL_XCB_WINDOW_MANAGER_H

#include "abstract-window-manager.h"
#include <netwm_def.h>
#include <QIcon>
#include <QHash>
namespace LingmoUIQuick {
class XcbWindowManager: public AbstractWindowManager
{
    Q_OBJECT
public:
    /**
     * 需要缓存的窗口属性
     */
    enum WindowProperty{
        SkipTaskbar,
        DemandsAttention,
        OnAllDesktops,
        OnCurrentDesktop,
        Maximized
    };

    Q_ENUM(WindowProperty)
    explicit XcbWindowManager(QObject *parent = nullptr);
    ~XcbWindowManager();

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

private:
    void windowAddedProxy(WId wid);
    void windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2);
    void updateWindowProperties(WId wid);
    QHash<WId, QHash<WindowProperty, QVariant>> m_windowProperties;

};
}

#endif //LINGMO_PANEL_XCB_WINDOW_MANAGER_H
