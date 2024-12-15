// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "systemdsignaldispatcher.h"

bool SystemdSignalDispatcher::connectToSignals() noexcept
{
    auto &con = ApplicationManager1DBus::instance().globalDestBus();

    if (!con.connect(SystemdService,
                     SystemdObjectPath,
                     SystemdInterfaceName,
                     "UnitNew",
                     this,
                     SLOT(onUnitNew(QString, QDBusObjectPath)))) {
        qCritical() << "can't connect to UnitNew signal of systemd service.";
        return false;
    }

    if (!con.connect(SystemdService,
                     SystemdObjectPath,
                     SystemdInterfaceName,
                     "UnitRemoved",
                     this,
                     SLOT(onUnitRemoved(QString, QDBusObjectPath)))) {
        qCritical() << "can't connect to UnitRemoved signal of systemd service.";
        return false;
    }

    if (!con.connect(SystemdService,
                     SystemdObjectPath,
                     SystemdPropInterfaceName,
                     "PropertiesChanged",
                     this,
                     SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)))) {
        qCritical() << "can't connect to PropertiesChanged signal of systemd service.";
        return false;
    }

    if (!con.connect(SystemdService,
                     SystemdObjectPath,
                     SystemdInterfaceName,
                     "JobNew",
                     this,
                     SLOT(onJobNew(uint32_t, QDBusObjectPath, QString)))) {
        qCritical() << "can't connect to JobNew signal of systemd service.";
        return false;
    }

    return true;
}

void SystemdSignalDispatcher::onPropertiesChanged(QString interface, QVariantMap props, [[maybe_unused]] QStringList invalid)
{
    if (interface != SystemdPropInterfaceName) {
        return;
    }

    if (auto it = props.find("Environment"); it != props.end()) {
        emit SystemdEnvironmentChanged(it->toStringList());
    }
}

void SystemdSignalDispatcher::onUnitNew(QString unitName, QDBusObjectPath systemdUnitPath)
{
    emit SystemdUnitNew(unitName, systemdUnitPath);
}

void SystemdSignalDispatcher::onJobNew(uint32_t, QDBusObjectPath systemdUnitPath, QString unitName)
{
    emit SystemdJobNew(unitName, systemdUnitPath);
}

void SystemdSignalDispatcher::onUnitRemoved(QString unitName, QDBusObjectPath systemdUnitPath)
{
    emit SystemdUnitRemoved(unitName, systemdUnitPath);
}
