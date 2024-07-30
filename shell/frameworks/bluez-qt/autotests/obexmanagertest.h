/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef OBEXMANAGERTEST_H
#define OBEXMANAGERTEST_H

#include <QObject>

class ObexManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanup();

    void obexNotRunningTest();
    void obexNotExportingInterfacesTest();
    void obexEmptyManagedObjectsTest();
    void obexNoClientTest();
    void obexNoAgentManagerTest();
    void obexServiceOkTest();
};

#endif // OBEXMANAGERTEST_H
