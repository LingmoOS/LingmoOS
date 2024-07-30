/*
    SPDX-FileCopyrightText: 2018 Billy Laws <blaws05@gmai.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VXLANSETTING_TEST_H
#define NETWORKMANAGERQT_VXLANSETTING_TEST_H

#include <QObject>

class VxlanSettingTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSetting_data();
    void testSetting();
};

#endif // NETWORKMANAGERQT_VXLANSETTING_TEST_H
