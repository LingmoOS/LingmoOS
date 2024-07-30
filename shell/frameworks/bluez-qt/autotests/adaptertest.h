/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ADAPTERTEST_H
#define ADAPTERTEST_H

#include <QList>
#include <QObject>

#include "bluezadapter1_tst.h"
#include "dbusproperties_tst.h"

#include "adapter.h"
#include "manager.h"

class AdapterTest : public QObject
{
    Q_OBJECT

public:
    explicit AdapterTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void getPropertiesTest();
    void setAliasTest();
    void setPoweredTest();
    void setDiscoverableTest();
    void setDiscoverableTimeoutTest();
    void setPairableTest();
    void setPairableTimeoutTest();

    void discoveryTest();
    void removeDeviceTest();
    void discoveryFilterTest_data();
    void discoveryFilterTest();
    void adapterRemovedTest();

private:
    struct AdapterUnit {
        BluezQt::AdapterPtr adapter;
        org::bluez::Adapter1 *dbusAdapter;
        org::freedesktop::DBus::Properties *dbusProperties;
    };

    BluezQt::Manager *m_manager;
    QList<AdapterUnit> m_units;
};

#endif // ADAPTERTEST_H
