/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMOMA_PROPERTIES_TEST_H
#define MODEMMANAGERQT_MODEMOMA_PROPERTIES_TEST_H

#include <QObject>

#include "fakemodem/fakemodem.h"
#include "fakemodem/modemoma.h"

class ModemOmaPropertiesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testModemOmaProperties();

private:
    FakeModem *fakeModem;
    Modem *modem;
    ModemOma *modemOma;
};

#endif // MODEMMANAGERQT_MODEMOMA_PROPERTIES_TEST_H
