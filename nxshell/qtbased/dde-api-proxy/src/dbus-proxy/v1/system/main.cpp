// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later


#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QStringList>

#include "system/org_deepin_dde_Accounts1.hpp"
#include "system/org_deepin_dde_Display1.hpp"
#include "system/org_deepin_dde_Gesture1.hpp"
#include "system/org_deepin_dde_Grub2.hpp"
#include "system/org_deepin_dde_Grub2_Theme.hpp"
#include "system/org_deepin_dde_Grub2_EditAuthentication.hpp"
#include "system/org_deepin_dde_Lastore1_Manager.hpp"
#include "system/org_deepin_dde_LockService1.hpp"
#include "system/org_deepin_dde_Network1.hpp"
#include "system/org_deepin_dde_PasswdConf1.hpp"
#include "system/org_deepin_dde_Power1.hpp"
#include "system/org_deepin_dde_SystemInfo1.hpp"
#include "system/org_deepin_dde_Timedate1.hpp"
#include "system/org_deepin_dde_AirplaneMode1.hpp"
#include "system/org_deepin_dde_PowerManager1.hpp"

#include "deepinruntime.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    if (!DeepinRuntime::CheckStartddeSession()) {
        qWarning() << "check startdde session error.";
        return 0;
    }
    QCommandLineParser parser;
    parser.setApplicationDescription("dde-api-proxy-system");
    parser.addHelpOption();
    QCommandLineOption serviceOption({"s", "service"}, "specific the service name", "service name");
    parser.addOption(serviceOption);
    parser.process(a);

    if (parser.value(serviceOption) == "com.deepin.daemon.Accounts") {
        // V0 -> V1
        new SystemAccounts1Proxy("org.deepin.dde.Accounts1", "/org/deepin/dde/Accounts1", "org.deepin.dde.Accounts1", 
            "com.deepin.daemon.Accounts", "/com/deepin/daemon/Accounts", "com.deepin.daemon.Accounts", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.system.Display") {
        new SystemDisplay1Proxy("org.deepin.dde.Display1", "/org/deepin/dde/Display1", "org.deepin.dde.Display1", 
            "com.deepin.system.Display", "/com/deepin/system/Display", "com.deepin.system.Display", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Gesture") {
        new SystemGesture1Proxy("org.deepin.dde.Gesture1", "/org/deepin/dde/Gesture1", "org.deepin.dde.Gesture1", 
            "com.deepin.daemon.Gesture", "/com/deepin/daemon/Gesture", "com.deepin.daemon.Gesture", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Grub2") {
        new SystemGrub2Proxy("org.deepin.dde.Grub2", "/org/deepin/dde/Grub2", "org.deepin.dde.Grub2", 
            "com.deepin.daemon.Grub2", "/com/deepin/daemon/Grub2", "com.deepin.daemon.Grub2", QDBusConnection::SystemBus);
        new SystemGrub2ThemeProxy("org.deepin.dde.Grub2", "/org/deepin/dde/Grub2/Theme", "org.deepin.dde.Grub2.Theme", 
            "com.deepin.daemon.Grub2", "/com/deepin/daemon/Grub2/Theme", "com.deepin.daemon.Grub2.Theme", QDBusConnection::SystemBus);
        new SystemGrub2EditAuthenticationProxy("org.deepin.dde.Grub2", "/org/deepin/dde/Grub2/EditAuthentication", "org.deepin.dde.Grub2.EditAuthentication", 
            "com.deepin.daemon.Grub2", "/com/deepin/daemon/Grub2/EditAuthentication", "com.deepin.daemon.Grub2.EditAuthentication", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.system.Network") {
        new SystemNetwork1Proxy("org.deepin.dde.Network1", "/org/deepin/dde/Network1", "org.deepin.dde.Network1", 
            "com.deepin.system.Network", "/com/deepin/system/Network", "com.deepin.system.Network", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.system.Power") {
        new SystemPower1Proxy("org.deepin.dde.Power1", "/org/deepin/dde/Power1", "org.deepin.dde.Power1", 
            "com.deepin.system.Power", "/com/deepin/system/Power", "com.deepin.system.Power", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.system.SystemInfo") {
        new SystemSystemInfo1Proxy("org.deepin.dde.SystemInfo1", "/org/deepin/dde/SystemInfo1", "org.deepin.dde.SystemInfo1", 
            "com.deepin.system.SystemInfo", "/com/deepin/system/SystemInfo", "com.deepin.system.SystemInfo", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.Timedated") {
        new SystemTimedate1Proxy("org.deepin.dde.Timedate1", "/org/deepin/dde/Timedate1", "org.deepin.dde.Timedate1", 
            "com.deepin.daemon.Timedated", "/com/deepin/daemon/Timedated", "com.deepin.daemon.Timedated", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.AirplaneMode") {
        new SystemAirplaneMode1Proxy("org.deepin.dde.AirplaneMode1", "/org/deepin/dde/AirplaneMode1", "org.deepin.dde.AirplaneMode1",
            "com.deepin.daemon.AirplaneMode", "/com/deepin/daemon/AirplaneMode", "com.deepin.daemon.AirplaneMode", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.Accounts1") {
        // V0.5 -> V1
        new SystemAccounts1Proxy("org.deepin.dde.Accounts1", "/org/deepin/dde/Accounts1", "org.deepin.dde.Accounts1", 
            "org.deepin.daemon.Accounts1", "/org/deepin/daemon/Accounts1", "org.deepin.daemon.Accounts1", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.Grub2") {
        new SystemGrub2Proxy("org.deepin.dde.Grub2", "/org/deepin/dde/Grub2", "org.deepin.dde.Grub2", 
            "org.deepin.daemon.Grub2", "/org/deepin/daemon/Grub2", "org.deepin.daemon.Grub2", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.Grub2") {
        new SystemGrub2ThemeProxy("org.deepin.dde.Grub2", "/org/deepin/dde/Grub2/Theme", "org.deepin.dde.Grub2.Theme", 
            "org.deepin.daemon.Grub2", "/org/deepin/daemon/Grub2/Theme", "org.deepin.daemon.Grub2.Theme", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.Grub2") {
        new SystemGrub2EditAuthenticationProxy("org.deepin.dde.Grub2", "/org/deepin/dde/Grub2/EditAuthentication", "org.deepin.dde.Grub2.EditAuthentication", 
            "org.deepin.daemon.Grub2", "/org/deepin/daemon/Grub2/EditAuthentication", "org.deepin.daemon.Grub2.EditAuthentication", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "org.deepin.system.Power1") {
        new SystemPower1Proxy("org.deepin.dde.Power1", "/org/deepin/dde/Power1", "org.deepin.dde.Power1", 
            "org.deepin.system.Power1", "/org/deepin/system/Power1", "org.deepin.system.Power1", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "org.deepin.system.SystemInfo1") {
        new SystemSystemInfo1Proxy("org.deepin.dde.SystemInfo1", "/org/deepin/dde/SystemInfo1", "org.deepin.dde.SystemInfo1", 
            "org.deepin.system.SystemInfo1", "/org/deepin/system/SystemInfo1", "org.deepin.system.SystemInfo1", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "org.deepin.daemon.Timedated1") {
        new SystemTimedate1Proxy("org.deepin.dde.Timedate1", "/org/deepin/dde/Timedate1", "org.deepin.dde.Timedate1", 
            "org.deepin.daemon.Timedated1", "/org/deepin/daemon/Timedated1", "org.deepin.daemon.Timedated1", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.lastore") {
        // SystemLastore1ManagerProxy: 包含Manager和Updater
        // V0 -> V1
        new SystemLastore1ManagerProxy("org.deepin.dde.Lastore1", "/org/deepin/dde/Lastore1", "org.deepin.dde.Lastore1.Manager", 
            "com.deepin.lastore", "/com/deepin/lastore", "com.deepin.lastore.Manager", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "org.deepin.lastore1") {
        // V0.5 -> V1
        new SystemLastore1ManagerProxy("org.deepin.dde.Lastore1", "/org/deepin/dde/Lastore1", "org.deepin.dde.Lastore1.Manager", 
            "org.deepin.lastore1", "/org/deepin/lastore1", "org.deepin.lastore1.Manager", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.dde.LockService") {
        // V0 -> V1
        new SystemLockService1Proxy("org.deepin.dde.LockService1", "/org/deepin/dde/LockService1", "org.deepin.dde.LockService1", 
            "com.deepin.dde.LockService", "/com/deepin/dde/LockService", "com.deepin.dde.LockService", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.PasswdConf") {
        // V0 -> V1
        new SystemPasswdConf1Proxy("org.deepin.dde.PasswdConf1", "/org/deepin/dde/PasswdConf1", "org.deepin.dde.PasswdConf1", 
            "com.deepin.daemon.PasswdConf", "/com/deepin/daemon/PasswdConf", "com.deepin.daemon.PasswdConf", QDBusConnection::SystemBus);
    } else if (parser.value(serviceOption) == "com.deepin.daemon.PowerManager") {
        new SystemPowerManager1Proxy("org.deepin.dde.PowerManager1", "/org/deepin/dde/PowerManager1", "org.deepin.dde.PowerManager1",
                                     "com.deepin.daemon.PowerManager", "/com/deepin/daemon/PowerManager", "com.deepin.daemon.PowerManager", QDBusConnection::SystemBus);
    }

    qInfo() << "proxy modules finish to start.";

    return a.exec();
}
