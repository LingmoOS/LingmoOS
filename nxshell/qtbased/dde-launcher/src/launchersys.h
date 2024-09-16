// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LAUNCHERSYS_H
#define LAUNCHERSYS_H

#include "calculate_util.h"
#include "controller/plugincontroller.h"

#include <DRegionMonitor>

#include <QObject>
#include <QTimer>

#include "sessionmanager_interface.h"

DGUI_USE_NAMESPACE

class LauncherInterface;
class WindowedFrame;
class FullScreenFrame;
class AMDBusLauncherInter;
class AMDBusDockInter;

using SessionManager = org::deepin::dde::SessionManager1;

class LauncherSys : public QObject
{
    Q_OBJECT

public:
    explicit LauncherSys(QObject *parent = nullptr);
    ~LauncherSys() override;

    bool visible();
    void showLauncher();
    void hideLauncher();
    void uninstallApp(const QString &desktopPath);

signals:
    void visibleChanged(bool visible);

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void displayModeChanged();
    void onAutoExitTimeout();
    void onVisibleChanged();
    void onDisplayModeChanged();
    void onFrontendRectChanged();
    void onButtonPress(const QPoint &p, const int flag);
    void onValueChanged();

private:
    void registerRegion();
    void unRegisterRegion();

private:
    AppsManager *m_appManager;
    LauncherInterface *m_launcherInter;                     // 启动器界面处理基类
    SessionManager *m_sessionManagerInter;                  // dbus访问远程服务类 业务逻辑处理

    WindowedFrame *m_windowLauncher;                        // 启动器小窗口界面处理类
    FullScreenFrame *m_fullLauncher;                        // 启动器全屏界面处理类
    DRegionMonitor *m_regionMonitor;                        // deepin tool kit中core模块的内容
    QTimer *m_autoExitTimer;
    QTimer *m_ignoreRepeatVisibleChangeTimer;               // 添加200ms延时操作，避开重复显示、隐藏启动器
    QMetaObject::Connection m_regionMonitorConnect;         // 信号和槽连接返回的对象
    CalculateUtil *m_calcUtil;                              // 界面布局计算处理类
    AMDBusLauncherInter *m_amDbusLauncher;
    AMDBusDockInter *m_amDbusDockInter;
    LauncherPluginController *m_launcherPlugin;
};

#endif // LAUNCHERSYS_H
