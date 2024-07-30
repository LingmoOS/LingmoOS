/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMLOCATION_PROPERTIES_TEST_H
#define MODEMMANAGERQT_MODEMLOCATION_PROPERTIES_TEST_H

#include <QObject>

#include "fakemodem/fakemodem.h"
#include "fakemodem/modemlocation.h"

class ModemLocationPropertiesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testModemLocationProperties();

private:
    FakeModem *fakeModem;
    Modem *modem;
    ModemLocation *modemLocation;
};

#endif // MODEMMANAGERQT_MODEMLOCATION_PROPERTIES_TEST_H
