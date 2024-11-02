/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ADAPTSCREENINFO_H
#define ADAPTSCREENINFO_H

#include <QObject>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <QRect>
#include <QDBusInterface>

#define ENV_X11       "x11"
#define ENV_WAYLAND   "wayland"
#define ENV_XDG_SESSION_TYPE "XDG_SESSION_TYPE"

#define DBUS_NAME       "org.lingmo.SettingsDaemon"
#define DBUS_PATH       "/org/lingmo/SettingsDaemon/wayland"
#define DBUS_INTERFACE  "org.lingmo.SettingsDaemon.wayland"

class adaptScreenInfo : public QObject
{
    Q_OBJECT
public:
    explicit adaptScreenInfo(QObject *parent = nullptr);
    void screenNumChange();
    void modifyMemberVariable();
    void InitializeHomeScreenGeometry();
    void initHwDbusScreen();


    QDesktopWidget              *m_pDeskWgt;                                    // 桌面问题
    int                          m_screenWidth;                                 // 桌面宽度
    int                          m_screenHeight;                                // 桌面高度
    int                          m_screenNum;                                   // 屏幕数量
    int                          m_nScreen_x;                                   // 主屏起始坐标X
    int                          m_nScreen_y;                                   // 主屏起始坐标Y
    QDBusInterface              *m_pDbusXrandInter;                             // HW的dbus接口

signals:

private:
    void initScreenSize();
    bool initHuaWeiDbus();
    void initOsDbusScreen();
    int getScreenGeometry(QString methodName);

private slots:
    void primaryScreenChangedSlot();
    void onResolutionChanged(const QRect argc);
    void screenCountChangedSlots(int count);
    void priScreenChanged(int x, int y, int width, int height);

private:
    QList<QScreen *> m_pListScreen;
    QStringList  ScreenName;
};

#endif // ADAPTSCREENINFO_H
