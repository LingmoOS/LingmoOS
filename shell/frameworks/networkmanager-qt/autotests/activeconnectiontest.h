/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_ACTIVE_CONNECTION_TEST_H
#define NETWORKMANAGERQT_ACTIVE_CONNECTION_TEST_H

#include <QObject>

#include "fakenetwork/fakenetwork.h"

class ActiveConnectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testActiveConnection();

private:
    FakeNetwork *fakeNetwork;
};

#endif // NETWORKMANAGERQT_ACTIVE_CONNECTION_TEST_H
