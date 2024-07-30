/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNETWORKMOUNTSTESTPATHS_H
#define KNETWORKMOUNTSTESTPATHS_H

#include <QObject>

class KNetworkMountsTestPaths : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testPaths_data();
    void testPaths();

private:
    QString m_configFileName;
};

#endif
