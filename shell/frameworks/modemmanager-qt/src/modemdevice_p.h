/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMDEVICE_P_H
#define MODEMMANAGERQT_MODEMDEVICE_P_H

#include <QMap>
#include <QObject>
#include <QStringList>

#include "bearer.h"
#include "interface.h"
#include "modemdevice.h"
#include "sim.h"

namespace ModemManager
{
class ModemDevicePrivate : public QObject
{
    Q_OBJECT
public:
    ModemDevicePrivate(const QString &path, ModemManager::ModemDevice *q);
    ~ModemDevicePrivate() override;
    void init();
    void initInterfaces();
    QString introspect() const;
    QString uni;
    QMap<ModemManager::ModemDevice::InterfaceType, ModemManager::Interface::Ptr> interfaceList;
    // QMap<QString, ModemManager::Bearer::Ptr> bearerList;
    ModemManager::Sim::Ptr simCard;
    ModemManager::Interface::List interfaces();
    ModemManager::Interface::Ptr interface(ModemManager::ModemDevice::InterfaceType type);
    ModemManager::Interface::Ptr createInterface(ModemManager::ModemDevice::InterfaceType type);
    ModemManager::Bearer::Ptr findBearer(const QString &uni);
    ModemManager::Bearer::List bearers();
    ModemManager::Sim::Ptr sim();

    Q_DECLARE_PUBLIC(ModemManager::ModemDevice)
    ModemManager::ModemDevice *q_ptr;

private Q_SLOTS:
    void onInterfacesAdded(const QDBusObjectPath &object_path, const ModemManager::MMVariantMapMap &interfaces_and_properties);
    void onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void onSimPathChanged(const QString &oldPath, const QString &newPath);
};

} // namespace ModemManager

#endif
