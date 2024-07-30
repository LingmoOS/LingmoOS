/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEM3GPP_PROPERTIES_TEST_H
#define MODEMMANAGERQT_MODEM3GPP_PROPERTIES_TEST_H

#include <QObject>

#include "fakemodem/fakemodem.h"
#include "fakemodem/modem3gpp.h"

class Modem3gppPropertiesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testModem3gppProperties();

private:
    FakeModem *fakeModem;
    Modem *modem;
    Modem3gpp *modem3gpp;
};

#endif // MODEMMANAGERQT_MODEM3GPP_PROPERTIES_TEST_H
