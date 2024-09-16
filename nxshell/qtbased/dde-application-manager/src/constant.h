// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONSTANT_H
#define CONSTANT_H

constexpr auto SystemdService = u8"org.freedesktop.systemd1";
constexpr auto SystemdObjectPath = u8"/org/freedesktop/systemd1";
constexpr auto SystemdPropInterfaceName = u8"org.freedesktop.DBus.Properties";
constexpr auto SystemdInterfaceName = u8"org.freedesktop.systemd1.Manager";
constexpr auto SystemdUnitInterfaceName = u8"org.freedesktop.systemd1.Unit";
constexpr auto DDEApplicationManager1ServiceName =
#ifdef DDE_AM_USE_DEBUG_DBUS_NAME
    u8"org.desktopspec.debug.ApplicationManager1";
#else
    u8"org.desktopspec.ApplicationManager1";
#endif

constexpr auto DDEApplicationManager1ObjectPath = u8"/org/desktopspec/ApplicationManager1";
constexpr auto DDEAutoStartManager1ObjectPath = u8"/org/desktopspec/AutoStartManager1";
constexpr auto DDEApplicationManager1JobManager1ObjectPath = u8"/org/desktopspec/ApplicationManager1/JobManager1";
constexpr auto DDEApplicationManager1MimeManager1ObjectPath = u8"/org/desktopspec/ApplicationManager1/MimeManager1";
constexpr auto DesktopFileEntryKey = u8"Desktop Entry";
constexpr auto DesktopFileActionKey = u8"Desktop Action ";
constexpr auto DesktopFileDefaultKeyLocale = "default";
constexpr auto X_Deepin_GenerateSource = u8"X-Deepin-GenerateSource";
constexpr auto DesktopEntryHidden = u8"Hidden";

constexpr auto ApplicationManagerServerDBusName =
#ifdef DDE_AM_USE_DEBUG_DBUS_NAME
    u8"deepin_application_manager_debug_server_bus";
#else
    u8"deepin_application_manager_server_bus";
#endif

constexpr auto ApplicationManagerDestDBusName =
#ifdef DDE_AM_USE_DEBUG_DBUS_NAME
    u8"deepin_application_manager_debug_dest_bus";
#else
    u8"deepin_application_manager_dest_bus";
#endif

constexpr auto ObjectManagerInterface = "org.desktopspec.DBus.ObjectManager";
constexpr auto JobManager1Interface = "org.desktopspec.JobManager1";
constexpr auto JobInterface = "org.desktopspec.JobManager1.Job";
constexpr auto ApplicationManager1Interface = "org.desktopspec.ApplicationManager1";
constexpr auto InstanceInterface = "org.desktopspec.ApplicationManager1.Instance";
constexpr auto ApplicationInterface = "org.desktopspec.ApplicationManager1.Application";
constexpr auto PropertiesInterface = u8"org.freedesktop.DBus.Properties";
constexpr auto MimeManager1Interface = u8"org.desktopspec.MimeManager1";

constexpr auto systemdOption = u8"systemd";
constexpr auto splitOption = u8"split";
constexpr auto AppExecOption = u8"appExec";

constexpr auto STORAGE_VERSION = 0;
constexpr auto ApplicationPropertiesGroup = u8"Application Properties";
constexpr auto LastLaunchedTime = u8"LastLaunchedTime";
constexpr auto Environ = u8"Environ";
constexpr auto LaunchedTimes = u8"LaunchedTimes";
constexpr auto InstalledTime = u8"InstalledTime";

constexpr auto ApplicationManagerHookDir = u8"/deepin/dde-application-manager/hooks.d";

constexpr auto ApplicationManagerToolsConfig = u8"org.deepin.dde.am";

constexpr auto ApplicationManagerConfig = u8"org.deepin.dde.application-manager";
constexpr auto AppExtraEnvironments = u8"appExtraEnvironments";
constexpr auto AppEnvironmentsBlacklist = u8"appEnvironmentsBlacklist";

#endif
