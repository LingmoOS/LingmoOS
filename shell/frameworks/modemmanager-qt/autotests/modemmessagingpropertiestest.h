/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMMESSAGING_PROPERTIES_TEST_H
#define MODEMMANAGERQT_MODEMMESSAGING_PROPERTIES_TEST_H

#include <QObject>

#include "fakemodem/fakemodem.h"
#include "fakemodem/modemmessaging.h"
#include "fakemodem/sms.h"

class ModemMessagingPropertiesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testModemMessagingProperties();

private:
    FakeModem *fakeModem;
    Modem *modem;
    ModemMessaging *modemMessaging;
    Sms *sms;
};

#endif // MODEMMANAGERQT_MODEMMESSAGING_PROPERTIES_TEST_H
