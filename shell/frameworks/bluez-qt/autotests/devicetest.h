/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef DEVICETEST_H
#define DEVICETEST_H

#include <QList>
#include <QObject>

#include "bluezdevice1_tst.h"
#include "dbusproperties_tst.h"

#include "adapter.h"
#include "device.h"
#include "manager.h"

class DeviceTest : public QObject
{
    Q_OBJECT

public:
    explicit DeviceTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void getPropertiesTest();
    void setAliasTest();
    void setTrustedTest();
    void setBlockedTest();

    void deviceRemovedTest();

private:
    struct DeviceUnit {
        BluezQt::DevicePtr device;
        org::bluez::Device1 *dbusDevice;
        org::freedesktop::DBus::Properties *dbusProperties;
    };

    QString deviceIcon(org::bluez::Device1 *device) const;
    qint16 deviceRssi(org::bluez::Device1 *device) const;

    BluezQt::Manager *m_manager;
    QList<DeviceUnit> m_units;
};

#endif // DEVICETEST_H
