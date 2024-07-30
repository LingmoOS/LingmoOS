/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_BEARER_PROPERTIES_TEST_H
#define MODEMMANAGERQT_BEARER_PROPERTIES_TEST_H

#include <QObject>

#include "fakemodem/fakemodem.h"

class BearerPropertiesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
#if MM_CHECK_VERSION(1, 2, 0)
    void testBearerProperties();
#endif

private:
    Bearer *bearer;
    FakeModem *fakeModem;
    Modem *modem;
};

#endif // MODEMMANAGERQT_BEARER_PROPERTIES_TEST_H
