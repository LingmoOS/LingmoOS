/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTSERVICEREMOTETEST_H
#define GATTSERVICEREMOTETEST_H

#include <QList>
#include <QObject>

#include "bluezgattservice1_tst.h"
#include "dbusproperties_tst.h"

#include "gattserviceremote.h"
#include "manager.h"

class GattServiceRemoteTest : public QObject
{
    Q_OBJECT

public:
    explicit GattServiceRemoteTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void getPropertiesTest();
    void setHandleTest();

    void serviceRemovedTest();

private:
    struct GattServiceRemoteUnit {
        BluezQt::GattServiceRemotePtr service;
        org::bluez::GattService1 *dbusService;
        org::freedesktop::DBus::Properties *dbusProperties;
    };

    BluezQt::Manager *m_manager;
    QList<GattServiceRemoteUnit> m_units;
};

#endif // GATTSERVICEREMOTETEST_H
