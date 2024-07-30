/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>

class ObjectManager;

class DeviceManager : public QObject
{
    Q_OBJECT

public:
    explicit DeviceManager(ObjectManager *parent = nullptr);

    void runAction(const QString &actionName, const QVariantMap &properties);

private:
    void runCreateAdapterAction(const QVariantMap &properties);
    void runCreateDeviceAction(const QVariantMap &properties);
    void runCreateGattServiceAction(const QVariantMap &properties);
    void runCreateGattCharacteristicAction(const QVariantMap &properties);
    void runCreateGattDescriptorAction(const QVariantMap &properties);
    void runRemoveAdapterAction(const QVariantMap &properties);
    void runRemoveDeviceAction(const QVariantMap &properties);
    void runRemoveGattServiceAction(const QVariantMap &properties);
    void runRemoveGattCharacteristicAction(const QVariantMap &properties);
    void runRemoveGattDescriptorAction(const QVariantMap &properties);
    void runChangeAdapterProperty(const QVariantMap &properties);
    void runChangeDeviceProperty(const QVariantMap &properties);
    void runAdapterMediaAction(const QString action, const QVariantMap &properties);
    void runAdapterLeAdvertisingManagerAction(const QString action, const QVariantMap &properties);
    void runAdapterGattManagerAction(const QString action, const QVariantMap &properties);
    void runBug377405();
    void runBug403289(const QVariantMap &properties);

    ObjectManager *m_objectManager;
};

#endif // DEVICEMANAGER_H
