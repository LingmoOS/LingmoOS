/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_OVS_PATCH_SETTING_TEST_H
#define NETWORKMANAGERQT_OVS_PATCH_SETTING_TEST_H

#include <QObject>

class OvsPatchSettingTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSetting_data();
    void testSetting();
};

#endif // NETWORKMANAGERQT_OVS_PATCH_SETTING_TEST_H
