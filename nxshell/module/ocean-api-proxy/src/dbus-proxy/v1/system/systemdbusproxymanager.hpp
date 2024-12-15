// SPDX-FileCopyrightText: 2018 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dbusproxybasemanager.hpp"
#include "org_lingmo_ocean_Accounts1.hpp"
#include "org_lingmo_ocean_Display1.hpp"
#include "org_lingmo_ocean_Gesture1.hpp"
#include "org_lingmo_ocean_Grub2.hpp"
#include "org_lingmo_ocean_Grub2_Theme.hpp"
#include "org_lingmo_ocean_Grub2_EditAuthentication.hpp"
#include "org_lingmo_ocean_Network1.hpp"
#include "org_lingmo_ocean_Power1.hpp"
#include "org_lingmo_ocean_SystemInfo1.hpp"
#include "org_lingmo_ocean_Timedate1.hpp"
#include "org_lingmo_ocean_AirplaneMode1.hpp"
#include "org_lingmo_ocean_Lastore1_Manager.hpp"
#include "org_lingmo_ocean_LockService1.hpp"
#include "org_lingmo_ocean_PasswdConf1.hpp"
#include "org_lingmo_ocean_PowerManager1.hpp"

class SystemDBusProxyManager : public DBusProxyManager {

public:
    SystemDBusProxyManager(QDBusConnection::BusType busType)
    :DBusProxyManager(busType)
    {

    }

    void init() override
    {
        m_dbusProxyFactory.push_back([this]{
            return  new SystemAccounts1Proxy("org.lingmo.ocean.Accounts1", "/org/lingmo/ocean/Accounts1", "org.lingmo.ocean.Accounts1",
                                        "com.lingmo.daemon.Accounts", "/com/lingmo/daemon/Accounts", "com.lingmo.daemon.Accounts", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemDisplay1Proxy("org.lingmo.ocean.Display1", "/org/lingmo/ocean/Display1", "org.lingmo.ocean.Display1",
                                        "com.lingmo.system.Display", "/com/lingmo/system/Display", "com.lingmo.system.Display", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemGesture1Proxy("org.lingmo.ocean.Gesture1", "/org/lingmo/ocean/Gesture1", "org.lingmo.ocean.Gesture1",
                                        "com.lingmo.daemon.Gesture", "/com/lingmo/daemon/Gesture", "com.lingmo.daemon.Gesture", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemGrub2Proxy("org.lingmo.ocean.Grub2", "/org/lingmo/ocean/Grub2", "org.lingmo.ocean.Grub2",
                                        "com.lingmo.daemon.Grub2", "/com/lingmo/daemon/Grub2", "com.lingmo.daemon.Grub2", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemGrub2ThemeProxy("org.lingmo.ocean.Grub2", "/org/lingmo/ocean/Grub2/Theme", "org.lingmo.ocean.Grub2.Theme",
                                        "com.lingmo.daemon.Grub2", "/com/lingmo/daemon/Grub2/Theme", "com.lingmo.daemon.Grub2.Theme", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemGrub2EditAuthenticationProxy("org.lingmo.ocean.Grub2", "/org/lingmo/ocean/Grub2/EditAuthentication", "org.lingmo.ocean.Grub2.EditAuthentication",
                                        "com.lingmo.daemon.Grub2", "/com/lingmo/daemon/Grub2/EditAuthentication", "com.lingmo.daemon.Grub2.EditAuthentication", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemNetwork1Proxy("org.lingmo.ocean.Network1", "/org/lingmo/ocean/Network1", "org.lingmo.ocean.Network1",
                                        "com.lingmo.system.Network", "/com/lingmo/system/Network", "com.lingmo.system.Network", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemPower1Proxy("org.lingmo.ocean.Power1", "/org/lingmo/ocean/Power1", "org.lingmo.ocean.Power1",
                                        "com.lingmo.system.Power", "/com/lingmo/system/Power", "com.lingmo.system.Power", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemSystemInfo1Proxy("org.lingmo.ocean.SystemInfo1", "/org/lingmo/ocean/SystemInfo1", "org.lingmo.ocean.SystemInfo1",
                                        "com.lingmo.system.SystemInfo", "/com/lingmo/system/SystemInfo", "com.lingmo.system.SystemInfo", m_busType);

        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemTimedate1Proxy("org.lingmo.ocean.Timedate1", "/org/lingmo/ocean/Timedate1", "org.lingmo.ocean.Timedate1",
                                        "com.lingmo.daemon.Timedated", "/com/lingmo/daemon/Timedated", "com.lingmo.daemon.Timedated", m_busType);

        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemAirplaneMode1Proxy("org.lingmo.ocean.AirplaneMode1", "/org/lingmo/ocean/AirplaneMode1", "org.lingmo.ocean.AirplaneMode1",
                                        "com.lingmo.daemon.AirplaneMode", "/com/lingmo/daemon/AirplaneMode", "com.lingmo.daemon.AirplaneMode", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return new SystemLastore1ManagerProxy("org.lingmo.ocean.Lastore1", "/org/lingmo/ocean/Lastore1", "org.lingmo.ocean.Lastore1.Manager",
                                        "com.lingmo.lastore", "/com/lingmo/lastore", "com.lingmo.lastore.Manager", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return new SystemLockService1Proxy("org.lingmo.ocean.LockService1", "/org/lingmo/ocean/LockService1", "org.lingmo.ocean.LockService1",
                                        "com.lingmo.ocean.LockService", "/com/lingmo/ocean/LockService", "com.lingmo.ocean.LockService", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return new SystemPasswdConf1Proxy("org.lingmo.ocean.PasswdConf1", "/org/lingmo/ocean/PasswdConf1", "org.lingmo.ocean.PasswdConf1",
                                        "com.lingmo.daemon.PasswdConf", "/com/lingmo/daemon/PasswdConf", "com.lingmo.daemon.PasswdConf", m_busType);
        });

        m_dbusProxyFactory.push_back([this]{
            return  new SystemPowerManager1Proxy("org.lingmo.ocean.PowerManager1", "/org/lingmo/ocean/PowerManager1", "org.lingmo.ocean.PowerManager1",
                                        "com.lingmo.daemon.PowerManager", "/com/lingmo/daemon/PowerManager", "com.lingmo.daemon.PowerManager", m_busType);
        });
    }
};
