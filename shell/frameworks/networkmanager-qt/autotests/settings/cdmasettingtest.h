/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_CDMASETTING_TEST_H
#define NETWORKMANAGERQT_CDMASETTING_TEST_H

#include <QObject>

class CdmaSettingTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSetting_data();
    void testSetting();
};

#endif // NETWORKMANAGERQT_CDMASETTING_TEST_H
