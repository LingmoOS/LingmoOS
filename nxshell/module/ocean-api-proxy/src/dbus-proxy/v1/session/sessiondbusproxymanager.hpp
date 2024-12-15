// SPDX-FileCopyrightText: 2018 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dbusproxybasemanager.hpp"
#include "org_lingmo_ocean_Appearance1.hpp"
#include "org_lingmo_ocean_Audio1.hpp"
#include "org_lingmo_ocean_Bluetooth1.hpp"
#include "org_lingmo_ocean_InputDevices1.hpp"
#include "org_lingmo_ocean_InputDevice1_Wacom.hpp"
#include "org_lingmo_ocean_InputDevice1_TouchPad.hpp"
#include "org_lingmo_ocean_InputDevice1_Keyboard.hpp"
#include "org_lingmo_ocean_InputDevice1_Mouse.hpp"
#include "org_lingmo_ocean_InputDevice1_TrackPoint.hpp"
#include "org_lingmo_ocean_Keybinding1.hpp"
#include "org_lingmo_ocean_LangSelector1.hpp"
#include "org_lingmo_ocean_Network1.hpp"
#include "org_lingmo_ocean_Power1.hpp"
#include "org_lingmo_ocean_SoundEffect1.hpp"
#include "org_lingmo_ocean_SystemInfo1.hpp"
#include "org_lingmo_ocean_Timedate1.hpp"
#include "org_lingmo_ocean_TrayManager1.hpp"
#include "org_lingmo_ocean_XEventMonitor1.hpp"
#include "org_lingmo_ocean_Display1.hpp"
#include "org_lingmo_ocean_SessionManager1.hpp"
#include "org_lingmo_ocean_WMSwitcher1.hpp"
#include "org_lingmo_ocean_Permission1.hpp"
#include "org_lingmo_ocean_daemon_Dock1.hpp"
#include "org_lingmo_ocean_daemon_Launcher1.hpp"
#include "org_lingmo_ocean_StartManager1.hpp"
#include "org_desktopspec_MimeManager1.hpp"
#include "org_lingmo_ocean_ControlCenter1.hpp"
#include "org_lingmo_ocean_Dock1.hpp"
#include "org_lingmo_ocean_Launcher1.hpp"
#include "org_lingmo_ocean_LockFront1.hpp"
#include "org_lingmo_ocean_ShutdownFront1.hpp"
#include "org_lingmo_ocean_Notification1.hpp"
#include "org_lingmo_ocean_Osd1.hpp"

const QString &AppearanceOldDbusName = "com.lingmo.daemon.Appearance";
const QString &AudioOldDbusName = "com.lingmo.daemon.Audio";
const QString &BluetoothOldDbusName = "com.lingmo.daemon.Bluetooth";
const QString &InputDevicesOldDbusName = "com.lingmo.daemon.InputDevices";
const QString &KeybindingOldDbusName = "com.lingmo.daemon.Keybinding";
const QString &LangSelectorOldDbusName = "com.lingmo.daemon.LangSelector";
const QString &NetworkOldDbusName = "com.lingmo.daemon.Network";
const QString &PowerOldDbusName = "com.lingmo.daemon.Power";
const QString &SoundEffectOldDbusName = "com.lingmo.daemon.SoundEffect";
const QString &SystemInfoOldDbusName = "com.lingmo.daemon.SystemInfo";
const QString &TimedateOldDbusName = "com.lingmo.daemon.Timedate";
const QString &TrayManagerOldDbusName = "com.lingmo.ocean.TrayManager";

class SessionDBusProxyManager : public DBusProxyManager {

public:
    SessionDBusProxyManager(QDBusConnection::BusType busType)
    :DBusProxyManager(busType)
    {

    }

    void init() override
    {
        m_dbusProxyFactory.push_back([this]{
            return   new SessionAppearance1Proxy("org.lingmo.ocean.Appearance1", "/org/lingmo/ocean/Appearance1", "org.lingmo.ocean.Appearance1",
                                        "com.lingmo.daemon.Appearance", "/com/lingmo/daemon/Appearance", "com.lingmo.daemon.Appearance", m_busType);
       });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionAudio1Proxy("org.lingmo.ocean.Audio1", "/org/lingmo/ocean/Audio1", "org.lingmo.ocean.Audio1",
                                         "com.lingmo.daemon.Audio", "/com/lingmo/daemon/Audio", "com.lingmo.daemon.Audio", m_busType);
       });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionBluetooth1Proxy("org.lingmo.ocean.Bluetooth1", "/org/lingmo/ocean/Bluetooth1", "org.lingmo.ocean.Bluetooth1",
                                        "com.lingmo.daemon.Bluetooth", "/com/lingmo/daemon/Bluetooth", "com.lingmo.daemon.Bluetooth", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionInputDevices1Proxy("org.lingmo.ocean.InputDevices1", "/org/lingmo/ocean/InputDevices1", "org.lingmo.ocean.InputDevices1",
                                        "com.lingmo.daemon.InputDevices", "/com/lingmo/daemon/InputDevices", "com.lingmo.daemon.InputDevices", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionInputDevice1WacomProxy("org.lingmo.ocean.InputDevices1", "/org/lingmo/ocean/InputDevice1/Wacom", "org.lingmo.ocean.InputDevice1.Wacom",
                                        "com.lingmo.daemon.InputDevices", "/com/lingmo/daemon/InputDevice/Wacom", "com.lingmo.daemon.InputDevice.Wacom", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionInputDevice1TouchPadProxy("org.lingmo.ocean.InputDevices1", "/org/lingmo/ocean/InputDevice1/TouchPad", "org.lingmo.ocean.InputDevice1.TouchPad",
                                        "com.lingmo.daemon.InputDevices", "/com/lingmo/daemon/InputDevice/TouchPad", "com.lingmo.daemon.InputDevice.TouchPad", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionInputDevice1KeyboardProxy("org.lingmo.ocean.InputDevices1", "/org/lingmo/ocean/InputDevice1/Keyboard", "org.lingmo.ocean.InputDevice1.Keyboard",
                                        "com.lingmo.daemon.InputDevices", "/com/lingmo/daemon/InputDevice/Keyboard", "com.lingmo.daemon.InputDevice.Keyboard", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionInputDevice1MouseProxy("org.lingmo.ocean.InputDevices1", "/org/lingmo/ocean/InputDevice1/Mouse", "org.lingmo.ocean.InputDevice1.Mouse",
                                        "com.lingmo.daemon.InputDevices", "/com/lingmo/daemon/InputDevice/Mouse", "com.lingmo.daemon.InputDevice.Mouse", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionInputDevice1TrackPointProxy("org.lingmo.ocean.InputDevices1", "/org/lingmo/ocean/InputDevice1/Mouse", "org.lingmo.ocean.InputDevice1.TrackPoint",
                                        "com.lingmo.daemon.InputDevices", "/com/lingmo/daemon/InputDevice/Mouse", "com.lingmo.daemon.InputDevice.TrackPoint", m_busType);

        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionKeybinding1Proxy("org.lingmo.ocean.Keybinding1", "/org/lingmo/ocean/Keybinding1", "org.lingmo.ocean.Keybinding1",
                                        "com.lingmo.daemon.Keybinding", "/com/lingmo/daemon/Keybinding", "com.lingmo.daemon.Keybinding", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionLangSelector1Proxy("org.lingmo.ocean.LangSelector1", "/org/lingmo/ocean/LangSelector1", "org.lingmo.ocean.LangSelector1",
                                        "com.lingmo.daemon.LangSelector", "/com/lingmo/daemon/LangSelector", "com.lingmo.daemon.LangSelector", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionNetwork1Proxy("org.lingmo.ocean.Network1", "/org/lingmo/ocean/Network1", "org.lingmo.ocean.Network1",
                                        "com.lingmo.daemon.Network", "/com/lingmo/daemon/Network", "com.lingmo.daemon.Network", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionPower1Proxy("org.lingmo.ocean.Power1", "/org/lingmo/ocean/Power1", "org.lingmo.ocean.Power1",
                                        "com.lingmo.daemon.Power", "/com/lingmo/daemon/Power", "com.lingmo.daemon.Power", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionSoundEffect1Proxy("org.lingmo.ocean.SoundEffect1", "/org/lingmo/ocean/SoundEffect1", "org.lingmo.ocean.SoundEffect1",
                                        "com.lingmo.daemon.SoundEffect", "/com/lingmo/daemon/SoundEffect", "com.lingmo.daemon.SoundEffect", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionSystemInfo1Proxy("org.lingmo.ocean.SystemInfo1", "/org/lingmo/ocean/SystemInfo1", "org.lingmo.ocean.SystemInfo1",
                                        "com.lingmo.daemon.SystemInfo", "/com/lingmo/daemon/SystemInfo", "com.lingmo.daemon.SystemInfo", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionTimedate1Proxy("org.lingmo.ocean.Timedate1", "/org/lingmo/ocean/Timedate1", "org.lingmo.ocean.Timedate1",
                                        "com.lingmo.daemon.Timedate", "/com/lingmo/daemon/Timedate", "com.lingmo.daemon.Timedate", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionTrayManager1Proxy("org.lingmo.ocean.TrayManager1", "/org/lingmo/ocean/TrayManager1", "org.lingmo.ocean.TrayManager1",
                                        "com.lingmo.ocean.TrayManager", "/com/lingmo/ocean/TrayManager", "com.lingmo.ocean.TrayManager", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionXEventMonitor1Proxy("org.lingmo.ocean.XEventMonitor1", "/org/lingmo/ocean/XEventMonitor1", "org.lingmo.ocean.XEventMonitor1",
                                        "com.lingmo.api.XEventMonitor", "/com/lingmo/api/XEventMonitor", "com.lingmo.api.XEventMonitor", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionDisplay1Proxy("org.lingmo.ocean.Display1", "/org/lingmo/ocean/Display1", "org.lingmo.ocean.Display1",
                                        "com.lingmo.daemon.Display", "/com/lingmo/daemon/Display", "com.lingmo.daemon.Display", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionSessionManager1Proxy("org.lingmo.ocean.SessionManager1", "/org/lingmo/ocean/SessionManager1", "org.lingmo.ocean.SessionManager1",
                                        "com.lingmo.SessionManager", "/com/lingmo/SessionManager", "com.lingmo.SessionManager", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionWMSwitcher1Proxy("org.lingmo.ocean.WMSwitcher1", "/org/lingmo/ocean/WMSwitcher1", "org.lingmo.ocean.WMSwitcher1",
                                        "com.lingmo.WMSwitcher", "/com/lingmo/WMSwitcher", "com.lingmo.WMSwitcher", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionPermission1Proxy("org.lingmo.ocean.Permission1", "/org/lingmo/ocean/Permission1", "org.lingmo.ocean.Permission1",
                                        "org.desktopspec.permission", "/org/desktopspec/permission", "org.desktopspec.permission", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionDaemonDock1Proxy("org.lingmo.ocean.daemon.Dock1", "/org/lingmo/ocean/daemon/Dock1", "org.lingmo.ocean.daemon.Dock1",
                                        "com.lingmo.ocean.daemon.Dock", "/com/lingmo/ocean/daemon/Dock", "com.lingmo.ocean.daemon.Dock", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionDaemonLauncher1Proxy("org.lingmo.ocean.daemon.Launcher1", "/org/lingmo/ocean/daemon/Launcher1", "org.lingmo.ocean.daemon.Launcher1",
                                        "com.lingmo.ocean.daemon.Launcher", "/com/lingmo/ocean/daemon/Launcher", "com.lingmo.ocean.daemon.Launcher", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionStartManager1Proxy("org.lingmo.ocean.StartManager1", "/org/lingmo/ocean/StartManager1", "org.lingmo.ocean.StartManager1",
                                        "com.lingmo.SessionManager", "/com/lingmo/StartManager", "com.lingmo.StartManager", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionMimeManager1Proxy("org.desktopspec.ApplicationManager1", "/org/desktopspec/ApplicationManager1/MimeManager1", "org.desktopspec.MimeManager1",
                                        "com.lingmo.daemon.Mime", "/com/lingmo/daemon/Mime", "com.lingmo.daemon.Mime", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionControlCenter1Proxy("org.lingmo.ocean.ControlCenter1", "/org/lingmo/ocean/ControlCenter1", "org.lingmo.ocean.ControlCenter1",
                                        "com.lingmo.ocean.ControlCenter", "/com/lingmo/ocean/ControlCenter", "com.lingmo.ocean.ControlCenter", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionDock1Proxy("org.lingmo.ocean.Dock1", "/org/lingmo/ocean/Dock1", "org.lingmo.ocean.Dock1",
                                        "com.lingmo.ocean.Dock", "/com/lingmo/ocean/Dock", "com.lingmo.ocean.Dock", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionLauncher1Proxy("org.lingmo.ocean.Launcher1", "/org/lingmo/ocean/Launcher1", "org.lingmo.ocean.Launcher1",
                                        "com.lingmo.ocean.Launcher", "/com/lingmo/ocean/Launcher", "com.lingmo.ocean.Launcher", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionLockFront1Proxy("org.lingmo.ocean.LockFront1", "/org/lingmo/ocean/LockFront1", "org.lingmo.ocean.LockFront1",
                                        "com.lingmo.ocean.lockFront", "/com/lingmo/ocean/lockFront", "com.lingmo.ocean.lockFront", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionShutdownFront1Proxy("org.lingmo.ocean.ShutdownFront1", "/org/lingmo/ocean/ShutdownFront1", "org.lingmo.ocean.ShutdownFront1",
                                        "com.lingmo.ocean.shutdownFront", "/com/lingmo/ocean/shutdownFront", "com.lingmo.ocean.shutdownFront", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return   new SessionNotification1Proxy("org.lingmo.ocean.Notification1", "/org/lingmo/ocean/Notification1", "org.lingmo.ocean.Notification1",
                                        "com.lingmo.ocean.Notification", "/com/lingmo/ocean/Notification", "com.lingmo.ocean.Notification", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SessionOsd1Proxy("org.lingmo.ocean.Osd1", "/org/lingmo/ocean/Osd1", "org.lingmo.ocean.Osd1",
                                        "com.lingmo.ocean.osd", "/", "com.lingmo.ocean.osd", m_busType);
        });
    }
};
