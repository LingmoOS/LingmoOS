/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEM_PROPERTIES_TEST_H
#define MODEMMANAGERQT_MODEM_PROPERTIES_TEST_H

#include <QObject>

#include "fakemodem/fakemodem.h"

class ModemPropertiesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testModemProperties();

private:
    FakeModem *fakeModem;
    Modem *modem;
};

#endif // MODEMMANAGERQT_MODEM_PROPERTIES_TEST_H
