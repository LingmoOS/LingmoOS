/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMTIME_PROPERTIES_TEST_H
#define MODEMMANAGERQT_MODEMTIME_PROPERTIES_TEST_H

#include <QObject>

#include "fakemodem/fakemodem.h"
#include "fakemodem/modemtime.h"

class ModemTimePropertiesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testModemTimeProperties();

private:
    FakeModem *fakeModem;
    Modem *modem;
    ModemTime *modemTime;
};

#endif // MODEMMANAGERQT_MODEMTIME_PROPERTIES_TEST_H
