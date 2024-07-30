/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef MANAGERTEST_H
#define MANAGERTEST_H

#include <QObject>

class ManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanup();

    void bluezNotRunningTest();
    void bluezNotExportingInterfacesTest();
    void bluezEmptyManagedObjectsTest();
    void bluezNoAdaptersTest();
    void bluezShutdownTest();

    void usableAdapterTest();
    void deviceForAddressTest();
    void adapterWithDevicesRemovedTest();
    void bug364416();
    void bug377405();
};

#endif // MANAGERTEST_H
