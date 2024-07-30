/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTDESCRIPTORREMOTETEST_H
#define GATTDESCRIPTORREMOTETEST_H

#include <QList>
#include <QObject>

#include "bluezgattdescriptor1_tst.h"
#include "dbusproperties_tst.h"

#include "gattdescriptorremote.h"
#include "manager.h"

class GattDescriptorRemoteTest : public QObject
{
    Q_OBJECT

public:
    explicit GattDescriptorRemoteTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void getPropertiesTest();
    void setHandleTest();

    void readValueTest();
    void writeValueTest();

    void descriptorRemovedTest();

private:
    struct GattDescriptorRemoteUnit {
        BluezQt::GattDescriptorRemotePtr descriptor;
        org::bluez::GattDescriptor1 *dbusDescriptor;
        org::freedesktop::DBus::Properties *dbusProperties;
    };

    BluezQt::Manager *m_manager;
    QList<GattDescriptorRemoteUnit> m_units;
};

#endif // GATTDESCRIPTORREMOTETEST_H
