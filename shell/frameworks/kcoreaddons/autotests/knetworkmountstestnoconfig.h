/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNETWORKMOUNTSTESTNOCONFIG_H
#define KNETWORKMOUNTSTESTNOCONFIG_H

#include <QObject>

class KNetworkMountsTestNoConfig : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testNoConfigPathTypes_data();
    void testNoConfigPathTypes();

    void testNoConfigPathOptions_data();
    void testNoConfigPathOptions();

    void testNoConfigOptions_data();
    void testNoConfigOptions();

private:
    QString m_configFileName;
};

#endif
