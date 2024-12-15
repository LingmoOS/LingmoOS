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
        return qEnvironmentVariable("OCEAN_CURRENT_COMPOSITOR") == "TreeLand";
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
        AppearanceService = "org.lingmo.ocean.Appearance1";
        AppearancePath = "/org/lingmo/ocean/Appearance1";
        AppearanceInterface = "org.lingmo.ocean.Appearance1";

        ControlCenterService = "org.lingmo.ocean.ControlCenter1";
        ControlCenterPath = "/org/lingmo/ocean/ControlCenter1";
        ControlCenterInterface = "org.lingmo.ocean.ControlCenter1";

        AccountsService = "org.lingmo.ocean.Accounts1";
        AccountsPath = "/org/lingmo/ocean/Accounts1";
        AccountsInterface = "org.lingmo.ocean.Accounts1";
        UserInterface = "org.lingmo.ocean.Accounts1.User";

        TrayManagerService = "org.lingmo.ocean.TrayManager1";
        TrayManagerPath = "/org/lingmo/ocean/TrayManager1";
        TrayManagerInterface = "org.freedesktop.DBus.Properties";

        DockService = "org.lingmo.ocean.Dock1";
        DockPath = "/org/lingmo/ocean/Dock1";
        DockInterface = "org.freedesktop.DBus.Properties";

        NotificationService = "org.lingmo.ocean.Notification1";
        NotificationPath = "/org/lingmo/ocean/Notification1";
        NotificationInterface = "org.lingmo.ocean.Notification1";

        MONITOR_SERVICE = "org.lingmo.ocean.XEventMonitor1";

        DISPLAY_SERVICE = "org.lingmo.ocean.Display1";
        DISPLAY_PATH = "/org/lingmo/ocean/Display1";
        DISPLAY_INTERFACE = "org.lingmo.ocean.Display1";
        DISPLAYMONITOR_INTERFACE = "org.lingmo.ocean.Display1.Monitor";

        DOCK_SERVICE = "org.lingmo.ocean.daemon.Dock1";
        DOCK_PATH = "/org/lingmo/ocean/daemon/Dock1";
        DOCK_INTERFACE = "org.lingmo.ocean.daemon.Dock1";
    }
};

class V20SystemState : public SystemState
{
public:
    V20SystemState() {
        AppearanceService = "com.lingmo.daemon.Appearance";
        AppearancePath = "/com/lingmo/daemon/Appearance";
        AppearanceInterface = "com.lingmo.daemon.Appearance";

        ControlCenterService = "com.lingmo.ocean.ControlCenter";
        ControlCenterPath = "/com/lingmo/ocean/ControlCenter";
        ControlCenterInterface = "com.lingmo.ocean.ControlCenter";

        AccountsService = "com.lingmo.daemon.Accounts";
        AccountsPath = "/com/lingmo/daemon/Accounts";
        AccountsInterface = "com.lingmo.daemon.Accounts";
        UserInterface = "com.lingmo.daemon.Accounts.User";

        TrayManagerService = "com.lingmo.ocean.TrayManager";
        TrayManagerPath = "/com/lingmo/ocean/TrayManager";
        TrayManagerInterface = "org.freedesktop.DBus.Properties";

        DockService = "com.lingmo.ocean.Dock";
        DockPath = "/com/lingmo/ocean/Dock";
        DockInterface = "org.freedesktop.DBus.Properties";

        NotificationService = "com.lingmo.ocean.Notification";
        NotificationPath = "/com/lingmo/ocean/Notification";
        NotificationInterface = "com.lingmo.ocean.Notification";

        MONITOR_SERVICE = "com.lingmo.api.XEventMonitor";

        DISPLAY_SERVICE = "com.lingmo.daemon.Display";
        DISPLAY_PATH = "/com/lingmo/daemon/Display";
        DISPLAY_INTERFACE = "com.lingmo.daemon.Display";
        DISPLAYMONITOR_INTERFACE = "com.lingmo.daemon.Display.Monitor";

        DOCK_SERVICE = "com.lingmo.ocean.daemon.Dock";
        DOCK_PATH = "/com/lingmo/ocean/daemon/Dock";
        DOCK_INTERFACE = "com.lingmo.ocean.daemon.Dock";
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