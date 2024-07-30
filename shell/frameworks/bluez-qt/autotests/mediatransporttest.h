/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef MEDIATRANSPORTTEST_H
#define MEDIATRANSPORTTEST_H

#include <QList>
#include <QObject>

#include "bluezdevice1_tst.h"
#include "bluezmediatransport1_tst.h"
#include "dbusproperties_tst.h"

#include "device.h"
#include "manager.h"
#include "mediatransport.h"

class MediaTransportTest : public QObject
{
    Q_OBJECT

public:
    explicit MediaTransportTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void connectTest();
    void disconnectTest();
    void connectProfileTest();

    void getPropertiesTest();

    void disconnectProfileTest();

private:
    struct MediaTransportUnit {
        BluezQt::DevicePtr device;
        org::bluez::MediaTransport1 *dbusMediaTransport;
        org::freedesktop::DBus::Properties *dbusProperties;
    };

    BluezQt::Manager *m_manager;
    QList<MediaTransportUnit> m_units;
};

#endif
