// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QStringList>

// dde-appearance
#include "session/org_deepin_dde_Appearance1.hpp"
// dde-daemon
#include "session/org_deepin_dde_Audio1.hpp"
#include "session/org_deepin_dde_Bluetooth1.hpp"
#include "session/org_deepin_dde_InputDevices1.hpp"
#include "session/org_deepin_dde_InputDevice1_Wacom.hpp"
#include "session/org_deepin_dde_InputDevice1_TouchPad.hpp"
#include "session/org_deepin_dde_InputDevice1_Keyboard.hpp"
#include "session/org_deepin_dde_InputDevice1_Mouse.hpp"
#include "session/org_deepin_dde_InputDevice1_TrackPoint.hpp"
#include "session/org_deepin_dde_Keybinding1.hpp"
#include "session/org_deepin_dde_LangSelector1.hpp"
#include "session/org_deepin_dde_Network1.hpp"
#include "session/org_deepin_dde_Power1.hpp"
#include "session/org_deepin_dde_SoundEffect1.hpp"
#include "session/org_deepin_dde_SystemInfo1.hpp"
#include "session/org_deepin_dde_Timedate1.hpp"
#include "session/org_deepin_dde_TrayManager1.hpp"
#include "session/org_deepin_dde_XEventMonitor1.hpp"
// startdde
#include "session/org_deepin_dde_Display1.hpp"
// dde-session
#include "session/org_deepin_dde_SessionManager1.hpp"
#include "session/org_deepin_dde_WMSwitcher1.hpp"
// dde-permission-manager
#include "session/org_deepin_dde_Permission1.hpp"
// dde-application-manager
#include "session/org_deepin_dde_daemon_Dock1.hpp"
#include "session/org_deepin_dde_daemon_Launcher1.hpp"
#include "session/org_deepin_dde_StartManager1.hpp"
#include "session/org_desktopspec_MimeManager1.hpp"
// dde-control-center
#include "session/org_deepin_dde_ControlCenter1.hpp"
// dde-dock
#include "session/org_deepin_dde_Dock1.hpp"
// dde-launcher
#include "session/org_deepin_dde_Launcher1.hpp"
// dde-lock
#include "session/org_deepin_dde_LockFront1.hpp"
#include "session/org_deepin_dde_ShutdownFront1.hpp"
// dde-osd
#include "session/org_deepin_dde_Notification1.hpp"
#include "session/org_deepin_dde_Osd1.hpp"

#include "deepinruntime.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("dde-api-proxy-session");
    parser.addHelpOption();
    QCommandLineOption serviceOption({"s", "service"}, "specific the service name", "service name");
    parser.addOption(serviceOption);
    parser.process(a);

    qInfo() << parser.value(serviceOption);
    if (parser.value(serviceOption) == "com.deepin.daemon.Appearance") {
        // V0 -> V1
        new SessionAppearance1Proxy("org.deepin.dde.Appearance1", "/org/deepin/dde/Appearance1", "org.deepin.dde.Appearance1",
            "com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", "com.deepin.daemon.Appearance", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.Appearance1") {
        // V0.5 -> V1
        new SessionAppearance1Proxy("org.deepin.dde.Appearance1", "/org/deepin/dde/Appearance1", "org.deepin.dde.Appearance1",
            "org.deepin.daemon.Appearance1", "/org/deepin/daemon/Appearance1", "org.deepin.daemon.Appearance1", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Audio") {
        // V0 -> V1
        new SessionAudio1Proxy("org.deepin.dde.Audio1", "/org/deepin/dde/Audio1", "org.deepin.dde.Audio1",
            "com.deepin.daemon.Audio", "/com/deepin/daemon/Audio", "com.deepin.daemon.Audio", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Bluetooth") {
        new SessionBluetooth1Proxy("org.deepin.dde.Bluetooth1", "/org/deepin/dde/Bluetooth1", "org.deepin.dde.Bluetooth1",
            "com.deepin.daemon.Bluetooth", "/com/deepin/daemon/Bluetooth", "com.deepin.daemon.Bluetooth", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.InputDevices") {
        new SessionInputDevices1Proxy("org.deepin.dde.InputDevices1", "/org/deepin/dde/InputDevices1", "org.deepin.dde.InputDevices1",
            "com.deepin.daemon.InputDevices", "/com/deepin/daemon/InputDevices", "com.deepin.daemon.InputDevices", QDBusConnection::SessionBus);
        new SessionInputDevice1WacomProxy("org.deepin.dde.InputDevices1", "/org/deepin/dde/InputDevice1/Wacom", "org.deepin.dde.InputDevice1.Wacom",
            "com.deepin.daemon.InputDevices", "/com/deepin/daemon/InputDevice/Wacom", "com.deepin.daemon.InputDevice.Wacom", QDBusConnection::SessionBus);
        new SessionInputDevice1TouchPadProxy("org.deepin.dde.InputDevices1", "/org/deepin/dde/InputDevice1/TouchPad", "org.deepin.dde.InputDevice1.TouchPad",
            "com.deepin.daemon.InputDevices", "/com/deepin/daemon/InputDevice/TouchPad", "com.deepin.daemon.InputDevice.TouchPad", QDBusConnection::SessionBus);
        new SessionInputDevice1KeyboardProxy("org.deepin.dde.InputDevices1", "/org/deepin/dde/InputDevice1/Keyboard", "org.deepin.dde.InputDevice1.Keyboard",
            "com.deepin.daemon.InputDevices", "/com/deepin/daemon/InputDevice/Keyboard", "com.deepin.daemon.InputDevice.Keyboard", QDBusConnection::SessionBus);
        new SessionInputDevice1MouseProxy("org.deepin.dde.InputDevices1", "/org/deepin/dde/InputDevice1/Mouse", "org.deepin.dde.InputDevice1.Mouse",
            "com.deepin.daemon.InputDevices", "/com/deepin/daemon/InputDevice/Mouse", "com.deepin.daemon.InputDevice.Mouse", QDBusConnection::SessionBus);
        new SessionInputDevice1TrackPointProxy("org.deepin.dde.InputDevices1", "/org/deepin/dde/InputDevice1/Mouse", "org.deepin.dde.InputDevice1.TrackPoint",
            "com.deepin.daemon.InputDevices", "/com/deepin/daemon/InputDevice/Mouse", "com.deepin.daemon.InputDevice.TrackPoint", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Keybinding") {
        new SessionKeybinding1Proxy("org.deepin.dde.Keybinding1", "/org/deepin/dde/Keybinding1", "org.deepin.dde.Keybinding1",
            "com.deepin.daemon.Keybinding", "/com/deepin/daemon/Keybinding", "com.deepin.daemon.Keybinding", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.LangSelector") {
        new SessionLangSelector1Proxy("org.deepin.dde.LangSelector1", "/org/deepin/dde/LangSelector1", "org.deepin.dde.LangSelector1",
            "com.deepin.daemon.LangSelector", "/com/deepin/daemon/LangSelector", "com.deepin.daemon.LangSelector", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Network") {
        new SessionNetwork1Proxy("org.deepin.dde.Network1", "/org/deepin/dde/Network1", "org.deepin.dde.Network1",
            "com.deepin.daemon.Network", "/com/deepin/daemon/Network", "com.deepin.daemon.Network", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Power") {
        new SessionPower1Proxy("org.deepin.dde.Power1", "/org/deepin/dde/Power1", "org.deepin.dde.Power1",
            "com.deepin.daemon.Power", "/com/deepin/daemon/Power", "com.deepin.daemon.Power", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.SoundEffect") {
        new SessionSoundEffect1Proxy("org.deepin.dde.SoundEffect1", "/org/deepin/dde/SoundEffect1", "org.deepin.dde.SoundEffect1",
            "com.deepin.daemon.SoundEffect", "/com/deepin/daemon/SoundEffect", "com.deepin.daemon.SoundEffect", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.SystemInfo") {
        new SessionSystemInfo1Proxy("org.deepin.dde.SystemInfo1", "/org/deepin/dde/SystemInfo1", "org.deepin.dde.SystemInfo1",
            "com.deepin.daemon.SystemInfo", "/com/deepin/daemon/SystemInfo", "com.deepin.daemon.SystemInfo", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Timedate") {
        new SessionTimedate1Proxy("org.deepin.dde.Timedate1", "/org/deepin/dde/Timedate1", "org.deepin.dde.Timedate1",
            "com.deepin.daemon.Timedate", "/com/deepin/daemon/Timedate", "com.deepin.daemon.Timedate", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.TrayManager") {
        new SessionTrayManager1Proxy("org.deepin.dde.TrayManager1", "/org/deepin/dde/TrayManager1", "org.deepin.dde.TrayManager1",
            "com.deepin.dde.TrayManager", "/com/deepin/dde/TrayManager", "com.deepin.dde.TrayManager", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.api.XEventMonitor") {
        new SessionXEventMonitor1Proxy("org.deepin.dde.XEventMonitor1", "/org/deepin/dde/XEventMonitor1", "org.deepin.dde.XEventMonitor1",
            "com.deepin.api.XEventMonitor", "/com/deepin/api/XEventMonitor", "com.deepin.api.XEventMonitor", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.Audio1") {
        // V0.5 -> V1
        new SessionAudio1Proxy("org.deepin.dde.Audio1", "/org/deepin/dde/Audio1", "org.deepin.dde.Audio1",
            "org.deepin.daemon.Audio1", "/org/deepin/daemon/Audio1", "org.deepin.daemon.Audio1", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.Bluetooth1") {
        new SessionBluetooth1Proxy("org.deepin.dde.Bluetooth1", "/org/deepin/dde/Bluetooth1", "org.deepin.dde.Bluetooth1",
            "org.deepin.daemon.Bluetooth1", "/org/deepin/daemon/Bluetooth1", "org.deepin.daemon.Bluetooth1", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.LangSelector1") {
        new SessionLangSelector1Proxy("org.deepin.dde.LangSelector1", "/org/deepin/dde/LangSelector1", "org.deepin.dde.LangSelector1",
            "org.deepin.daemon.LangSelector1", "/org/deepin/daemon/LangSelector1", "org.deepin.daemon.LangSelector1", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.Power1") {
        new SessionPower1Proxy("org.deepin.dde.Power1", "/org/deepin/dde/Power1", "org.deepin.dde.Power1",
            "org.deepin.daemon.Power1", "/org/deepin/daemon/Power1", "org.deepin.daemon.Power1", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.SoundEffect1") {
        new SessionSoundEffect1Proxy("org.deepin.dde.SoundEffect1", "/org/deepin/dde/SoundEffect1", "org.deepin.dde.SoundEffect1",
            "org.deepin.daemon.SoundEffect1", "/org/deepin/daemon/SoundEffect1", "org.deepin.daemon.SoundEffect1", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.Timedate1") {
        new SessionTimedate1Proxy("org.deepin.dde.Timedate1", "/org/deepin/dde/Timedate1", "org.deepin.dde.Timedate1",
            "org.deepin.daemon.Timedate1", "/org/deepin/daemon/Timedate1", "org.deepin.daemon.Timedate1", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.SystemInfo1") {
        new SessionSystemInfo1Proxy("org.deepin.dde.SystemInfo1", "/org/deepin/dde/SystemInfo1", "org.deepin.dde.SystemInfo1",
            "org.deepin.daemon.SystemInfo1", "/org/deepin/daemon/SystemInfo1", "org.deepin.daemon.SystemInfo1", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "org.deepin.api.XEventMonitor1") {
        new SessionXEventMonitor1Proxy("org.deepin.dde.XEventMonitor1", "/org/deepin/dde/XEventMonitor1", "org.deepin.dde.XEventMonitor1",
            "org.deepin.api.XEventMonitor1", "/org/deepin/api/XEventMonitor1", "org.deepin.api.XEventMonitor1", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Display") {
        // V0 -> V1
        new SessionDisplay1Proxy("org.deepin.dde.Display1", "/org/deepin/dde/Display1", "org.deepin.dde.Display1",
            "com.deepin.daemon.Display", "/com/deepin/daemon/Display", "com.deepin.daemon.Display", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.SessionManager") {
        // V0 -> V1
        new SessionSessionManager1Proxy("org.deepin.dde.SessionManager1", "/org/deepin/dde/SessionManager1", "org.deepin.dde.SessionManager1",
            "com.deepin.SessionManager", "/com/deepin/SessionManager", "com.deepin.SessionManager", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.WMSwitcher") {
        new SessionWMSwitcher1Proxy("org.deepin.dde.WMSwitcher1", "/org/deepin/dde/WMSwitcher1", "org.deepin.dde.WMSwitcher1",
            "com.deepin.WMSwitcher", "/com/deepin/WMSwitcher", "com.deepin.WMSwitcher", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "org.desktopspec.permission") {
        // V0 -> V1
        new SessionPermission1Proxy("org.deepin.dde.Permission1", "/org/deepin/dde/Permission1", "org.deepin.dde.Permission1",
            "org.desktopspec.permission", "/org/desktopspec/permission", "org.desktopspec.permission", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.daemon.Dock") {
        // V0 -> V1
        new SessionDaemonDock1Proxy("org.deepin.dde.daemon.Dock1", "/org/deepin/dde/daemon/Dock1", "org.deepin.dde.daemon.Dock1",
            "com.deepin.dde.daemon.Dock", "/com/deepin/dde/daemon/Dock", "com.deepin.dde.daemon.Dock", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.daemon.Launcher") {
        new SessionDaemonLauncher1Proxy("org.deepin.dde.daemon.Launcher1", "/org/deepin/dde/daemon/Launcher1", "org.deepin.dde.daemon.Launcher1",
            "com.deepin.dde.daemon.Launcher", "/com/deepin/dde/daemon/Launcher", "com.deepin.dde.daemon.Launcher", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.SessionManager") {
        // V0 StartManager的服务名为"com.deepin.SessionManager"
        new SessionStartManager1Proxy("org.deepin.dde.StartManager1", "/org/deepin/dde/StartManager1", "org.deepin.dde.StartManager1",
            "com.deepin.SessionManager", "/com/deepin/StartManager", "com.deepin.StartManager", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Mime") {
        new SessionMimeManager1Proxy("org.desktopspec.ApplicationManager1", "/org/desktopspec/ApplicationManager1/MimeManager1", "org.desktopspec.MimeManager1",
            "com.deepin.daemon.Mime", "/com/deepin/daemon/Mime", "com.deepin.daemon.Mime", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.ControlCenter") {
        // V0 -> V1
        new SessionControlCenter1Proxy("org.deepin.dde.ControlCenter1", "/org/deepin/dde/ControlCenter1", "org.deepin.dde.ControlCenter1",
            "com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter", "com.deepin.dde.ControlCenter", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.Dock") {
        // V0 -> V1
        new SessionDock1Proxy("org.deepin.dde.Dock1", "/org/deepin/dde/Dock1", "org.deepin.dde.Dock1",
            "com.deepin.dde.Dock", "/com/deepin/dde/Dock", "com.deepin.dde.Dock", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.Launcher") {
        // V0 -> V1
        new SessionLauncher1Proxy("org.deepin.dde.Launcher1", "/org/deepin/dde/Launcher1", "org.deepin.dde.Launcher1",
            "com.deepin.dde.Launcher", "/com/deepin/dde/Launcher", "com.deepin.dde.Launcher", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.lockFront") {
        // V0 -> V1
        new SessionLockFront1Proxy("org.deepin.dde.LockFront1", "/org/deepin/dde/LockFront1", "org.deepin.dde.LockFront1",
            "com.deepin.dde.lockFront", "/com/deepin/dde/lockFront", "com.deepin.dde.lockFront", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.shutdownFront") {
        new SessionShutdownFront1Proxy("org.deepin.dde.ShutdownFront1", "/org/deepin/dde/ShutdownFront1", "org.deepin.dde.ShutdownFront1",
            "com.deepin.dde.shutdownFront", "/com/deepin/dde/shutdownFront", "com.deepin.dde.shutdownFront", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.Notification") {
        // V0 -> V1
        new SessionNotification1Proxy("org.deepin.dde.Notification1", "/org/deepin/dde/Notification1", "org.deepin.dde.Notification1",
            "com.deepin.dde.Notification", "/com/deepin/dde/Notification", "com.deepin.dde.Notification", QDBusConnection::SessionBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.osd") {
        new SessionOsd1Proxy("org.deepin.dde.Osd1", "/org/deepin/dde/Osd1", "org.deepin.dde.Osd1",
            "com.deepin.dde.osd", "/", "com.deepin.dde.osd", QDBusConnection::SessionBus);
    }

    qInfo() << "proxy modules finish to start.";

    return a.exec();
}
