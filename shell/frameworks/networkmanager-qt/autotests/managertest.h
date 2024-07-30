/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_MANAGER_TEST_H
#define NETWORKMANAGERQT_MANAGER_TEST_H

#include <QObject>

#include "fakenetwork/fakenetwork.h"

class ManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testDevices();
    void testDeviceAdded(const QString &dev);
    void testManager();

private:
    FakeNetwork *fakeNetwork;
};

#endif // NETWORKMANAGERQT_MANAGER_TEST_H
