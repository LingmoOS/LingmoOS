/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNETWORKMOUNTSTESTSTATIC_H
#define KNETWORKMOUNTSTESTSTATIC_H

#include <QObject>

class KNetworkMountsTestStatic : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testStaticFunctions_data();
    void testStaticFunctions();
    void testStaticKNetworkMountOptionToString_data();
    void testStaticKNetworkMountOptionToString();
    void testStaticKNetworkMountsTypeToString_data();
    void testStaticKNetworkMountsTypeToString();

private:
    QString m_configFileName;
};

#endif
