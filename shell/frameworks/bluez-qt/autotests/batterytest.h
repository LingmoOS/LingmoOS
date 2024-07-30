/*
 * SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BATTERYTEST_H
#define BATTERYTEST_H

#include <QObject>

#include "bluezbattery1_tst.h"

#include "battery.h"
#include "device.h"
#include "manager.h"

class BatteryTest : public QObject
{
    Q_OBJECT

public:
    explicit BatteryTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void getPropertiesTest();

private:
    struct BatteryUnit {
        BluezQt::DevicePtr device;
        org::bluez::Battery1 *dbusBattery;
    };

    BluezQt::Manager *m_manager;
    QList<BatteryUnit> m_units;
};

#endif // BATTERYTEST_H
