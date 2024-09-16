// Copyright (C) 2023 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DSysInfo>
#include <QDebug>

namespace common{

class SystemState {
public:
    QString AppearanceService;
    QString AppearancePath ;
    QString AppearanceInterface;
    QString ControlCenterService;
    QString ControlCenterPath;
    QString ControlCenterInterface;

    QString AccountsService;
    QString AccountsPath;
    QString AccountsInterface;
    QString UserInterface;

    QString TrayManagerService;
    QString TrayManagerPath;
    QString TrayManagerInterface;

    QString DockService;
    QString DockPath;
    QString DockInterface;

    QString NotificationService;
    QString NotificationPath;
    QString NotificationInterface;

    QString MONITOR_SERVICE;

    QString DISPLAY_SERVICE;
    QString DISPLAY_PATH;
    QString DISPLAY_INTERFACE;
    QString DISPLAYMONITOR_INTERFACE;

    QString DOCK_SERVICE;
    QString DOCK_PATH;
    QString DOCK_INTERFACE;

    inline bool isTreeLand() const
    {
        return qEnvironmentVariable("DDE_CURRENT_COMPOSITOR") == "TreeLand";
    }
    inline bool isOldVersion() const
    {
        return m_isOldVersion;
    }
    inline void updateVersion(const bool isOld)
    {
        m_isOldVersion = isOld;
    }
private:
    bool m_isOldVersion = false;
};

class V23SystemState : public SystemState
{
public:
    V23SystemState() {
        AppearanceService = "org.deepin.dde.Appearance1";
        AppearancePath = "/org/deepin/dde/Appearance1";
        AppearanceInterface = "org.deepin.dde.Appearance1";

        ControlCenterService = "org.deepin.dde.ControlCenter1";
        ControlCenterPath = "/org/deepin/dde/ControlCenter1";
        ControlCenterInterface = "org.deepin.dde.ControlCenter1";

        AccountsService = "org.deepin.dde.Accounts1";
        AccountsPath = "/org/deepin/dde/Accounts1";
        AccountsInterface = "org.deepin.dde.Accounts1";
        UserInterface = "org.deepin.dde.Accounts1.User";

        TrayManagerService = "org.deepin.dde.TrayManager1";
        TrayManagerPath = "/org/deepin/dde/TrayManager1";
        TrayManagerInterface = "org.freedesktop.DBus.Properties";

        DockService = "org.deepin.dde.Dock1";
        DockPath = "/org/deepin/dde/Dock1";
        DockInterface = "org.freedesktop.DBus.Properties";

        NotificationService = "org.deepin.dde.Notification1";
        NotificationPath = "/org/deepin/dde/Notification1";
        NotificationInterface = "org.deepin.dde.Notification1";

        MONITOR_SERVICE = "org.deepin.dde.XEventMonitor1";

        DISPLAY_SERVICE = "org.deepin.dde.Display1";
        DISPLAY_PATH = "/org/deepin/dde/Display1";
        DISPLAY_INTERFACE = "org.deepin.dde.Display1";
        DISPLAYMONITOR_INTERFACE = "org.deepin.dde.Display1.Monitor";

        DOCK_SERVICE = "org.deepin.dde.daemon.Dock1";
        DOCK_PATH = "/org/deepin/dde/daemon/Dock1";
        DOCK_INTERFACE = "org.deepin.dde.daemon.Dock1";
    }
};

class V20SystemState : public SystemState
{
public:
    V20SystemState() {
        AppearanceService = "com.deepin.daemon.Appearance";
        AppearancePath = "/com/deepin/daemon/Appearance";
        AppearanceInterface = "com.deepin.daemon.Appearance";

        ControlCenterService = "com.deepin.dde.ControlCenter";
        ControlCenterPath = "/com/deepin/dde/ControlCenter";
        ControlCenterInterface = "com.deepin.dde.ControlCenter";

        AccountsService = "com.deepin.daemon.Accounts";
        AccountsPath = "/com/deepin/daemon/Accounts";
        AccountsInterface = "com.deepin.daemon.Accounts";
        UserInterface = "com.deepin.daemon.Accounts.User";

        TrayManagerService = "com.deepin.dde.TrayManager";
        TrayManagerPath = "/com/deepin/dde/TrayManager";
        TrayManagerInterface = "org.freedesktop.DBus.Properties";

        DockService = "com.deepin.dde.Dock";
        DockPath = "/com/deepin/dde/Dock";
        DockInterface = "org.freedesktop.DBus.Properties";

        NotificationService = "com.deepin.dde.Notification";
        NotificationPath = "/com/deepin/dde/Notification";
        NotificationInterface = "com.deepin.dde.Notification";

        MONITOR_SERVICE = "com.deepin.api.XEventMonitor";

        DISPLAY_SERVICE = "com.deepin.daemon.Display";
        DISPLAY_PATH = "/com/deepin/daemon/Display";
        DISPLAY_INTERFACE = "com.deepin.daemon.Display";
        DISPLAYMONITOR_INTERFACE = "com.deepin.daemon.Display.Monitor";

        DOCK_SERVICE = "com.deepin.dde.daemon.Dock";
        DOCK_PATH = "/com/deepin/dde/daemon/Dock";
        DOCK_INTERFACE = "com.deepin.dde.daemon.Dock";
    }
};

static inline const SystemState &systemInfo()
{
    static SystemState *state = nullptr;
    if (!state) {
        const auto version = DTK_CORE_NAMESPACE::DSysInfo::majorVersion();
        qInfo() << "Running desktop environment version is:" << version << ", versionNumber:" << version.toLong();
        if (!version.isEmpty() && version.toLong() <= 20) {
            state = new V20SystemState();
            state->updateVersion(true);
        } else {
            state = new V23SystemState();
            state->updateVersion(false);
        }
    }
    return *state;
}

}