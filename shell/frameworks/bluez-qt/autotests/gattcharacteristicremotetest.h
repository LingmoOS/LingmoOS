/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTCHARACTERISTICREMOTETEST_H
#define GATTCHARACTERISTICREMOTETEST_H

#include <QList>
#include <QObject>

#include "bluezgattcharacteristic1_tst.h"
#include "dbusproperties_tst.h"

#include "gattserviceremote.h"
#include "gattcharacteristicremote.h"
#include "manager.h"

class GattCharacteristicRemoteTest : public QObject
{
    Q_OBJECT

public:
    explicit GattCharacteristicRemoteTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void getPropertiesTest();
    void setHandleTest();

    void readValueTest();
    void writeValueTest();
    void startNotifyTest();
    void stopNotifyTest();

    void characteristicRemovedTest();

private:
    struct GattCharacteristicRemoteUnit {
        BluezQt::GattCharacteristicRemotePtr characteristic;
        org::bluez::GattCharacteristic1 *dbusCharacteristic;
        org::freedesktop::DBus::Properties *dbusProperties;
    };

    BluezQt::Manager *m_manager;
    QList<GattCharacteristicRemoteUnit> m_units;
};

#endif // GATTCHARACTERISTICREMOTETEST_H
