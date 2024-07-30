/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNETWORKMOUNTSTESTCANONICAL_H
#define KNETWORKMOUNTSTESTCANONICAL_H

#include <QObject>
#include <QTemporaryDir>

class KNetworkMountsTestCanonical : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testCanonicalSymlinkPath_data();
    void testCanonicalSymlinkPath();

private:
    QString m_configFileName;
    QTemporaryDir m_tmpDir;
};

#endif
