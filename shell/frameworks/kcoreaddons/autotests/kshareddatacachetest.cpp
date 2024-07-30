/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <kshareddatacache.h>

#include <QStandardPaths>
#include <QTest>

#include <QObject>
#include <QStandardPaths>
#include <QString>
#include <string.h> // strcpy

class KSharedDataCacheTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void simpleInsert();
};

void KSharedDataCacheTest::initTestCase()
{
}

void KSharedDataCacheTest::simpleInsert()
{
    const QLatin1String cacheName("myTestCache");
    const QLatin1String key("mypic");
    // clear the cache
    QString cacheFile = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/") + cacheName + QLatin1String(".kcache");
    QFile file(cacheFile);
    if (file.exists()) {
        QVERIFY(file.remove());
    }
    // insert something into it
    KSharedDataCache cache(cacheName, 5 * 1024 * 1024);
#ifndef Q_OS_WIN // the windows implementation is currently only memory based and not really shared
    QVERIFY(file.exists()); // make sure we got the cache filename right
#endif
    QByteArray data;
    data.resize(9228);
    strcpy(data.data(), "Hello world");
    QVERIFY(cache.insert(key, data));
    // read it out again
    QByteArray result;
    QVERIFY(cache.find(key, &result));
    QCOMPARE(result, data);
    // another insert
    strcpy(data.data(), "Hello KDE");
    QVERIFY(cache.insert(key, data));
    // and another read
    QVERIFY(cache.find(key, &result));
    QCOMPARE(result, data);
}

QTEST_MAIN(KSharedDataCacheTest)

#include "kshareddatacachetest.moc"
