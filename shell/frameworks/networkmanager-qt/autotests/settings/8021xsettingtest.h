/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_SECURITY8021XSETTING_TEST_H
#define NETWORKMANAGERQT_SECURITY8021XSETTING_TEST_H

#include <QObject>

class Security8021xSettingTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSetting_data();
    void testSetting();
};

#endif // NETWORKMANAGERQT_SECURITY8021XSETTING_TEST_H
