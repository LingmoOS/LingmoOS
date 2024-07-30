/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEM_TEST_H
#define MODEMMANAGERQT_MODEM_TEST_H

#include <QObject>

#include "fakemodem/fakemodem.h"

class ModemTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testModems();
    void testModemAdded(const QString &dev);

private:
    FakeModem *fakeModem;
};

#endif // MODEMMANAGERQT_MODEM_TEST_H
