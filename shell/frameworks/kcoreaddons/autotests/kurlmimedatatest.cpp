/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kurlmimedatatest.h"
#include <QMimeData>
#include <QTest>
#include <kurlmimedata.h>
#include <kurlmimedata_p.h>
#include <memory>

using namespace Qt::StringLiterals;

QTEST_MAIN(KUrlMimeDataTest)

void KUrlMimeDataTest::testURLList()
{
    QMimeData *mimeData = new QMimeData();
    QVERIFY(!mimeData->hasUrls());

    QList<QUrl> urls;
    urls.append(QUrl(QLatin1String("https://www.kde.org")));
    urls.append(QUrl(QLatin1String("http://wstephenson:secret@example.com/path")));
    urls.append(QUrl(QLatin1String("file:///home/dfaure/konqtests/Mat%C3%A9riel")));
    QMap<QString, QString> metaData;
    metaData[QLatin1String("key")] = QLatin1String("value");
    metaData[QLatin1String("key2")] = QLatin1String("value2");

    KUrlMimeData::setUrls(QList<QUrl>(), urls, mimeData);
    KUrlMimeData::setMetaData(metaData, mimeData);

    QVERIFY(mimeData->hasUrls());
    QVERIFY(mimeData->hasText());

    QMap<QString, QString> decodedMetaData;
    QList<QUrl> decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData, KUrlMimeData::PreferKdeUrls, &decodedMetaData);
    QVERIFY(!decodedURLs.isEmpty());
    QList<QUrl> expectedUrls = urls;
    expectedUrls[1] = QUrl(QLatin1String("http://wstephenson:secret@example.com/path")); // password kept, unlike in KDE4, but that's okay, it's not displayed
    QCOMPARE(expectedUrls, decodedURLs);

    const QList<QUrl> qurls = mimeData->urls();
    QCOMPARE(qurls.count(), urls.count());
    for (int i = 0; i < qurls.count(); ++i) {
        QCOMPARE(qurls[i], decodedURLs[i]);
    }

    QVERIFY(!decodedMetaData.isEmpty());
    QCOMPARE(decodedMetaData[QLatin1String("key")], QString::fromLatin1("value"));
    QCOMPARE(decodedMetaData[QLatin1String("key2")], QString::fromLatin1("value2"));

    delete mimeData;
}

void KUrlMimeDataTest::testOneURL()
{
    QUrl oneURL(QLatin1String("file:///tmp"));
    QList<QUrl> oneEltList;
    oneEltList.append(oneURL);
    QMimeData *mimeData = new QMimeData();

    KUrlMimeData::setUrls(QList<QUrl>(), oneEltList, mimeData);
    QVERIFY(mimeData->hasUrls());

    QMap<QString, QString> decodedMetaData;
    QList<QUrl> decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData, KUrlMimeData::PreferKdeUrls, &decodedMetaData);
    QVERIFY(!decodedURLs.isEmpty());
    QCOMPARE(decodedURLs.count(), 1);
    QCOMPARE(decodedURLs[0], oneURL);
    QVERIFY(decodedMetaData.isEmpty());
    delete mimeData;
}

void KUrlMimeDataTest::testFromQUrl()
{
    QList<QUrl> qurls;
    qurls.append(QUrl(QLatin1String("https://www.kde.org")));
    qurls.append(QUrl(QLatin1String("file:///home/dfaure/konqtests/Mat%C3%A9riel")));
    QMimeData *mimeData = new QMimeData();
    KUrlMimeData::setUrls(QList<QUrl>(), qurls, mimeData);
    QVERIFY(mimeData->hasUrls());

    QMap<QString, QString> decodedMetaData;
    QList<QUrl> decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData, KUrlMimeData::PreferKdeUrls, &decodedMetaData);
    QVERIFY(!decodedURLs.isEmpty());
    QCOMPARE(decodedURLs.count(), 2);
    QCOMPARE(decodedURLs[0], qurls[0]);
    QCOMPARE(decodedURLs[1], qurls[1]);
    QVERIFY(decodedMetaData.isEmpty());
    delete mimeData;
}

void KUrlMimeDataTest::testMostLocalUrlList_data()
{
    QTest::addColumn<bool>("withKdeUrls");
    QTest::addColumn<bool>("withLocalUrls");
    QTest::addColumn<bool>("expectedLocalUrls");

    QTest::newRow("both") << true << true << false;
    QTest::newRow("local_only") << false << true << true;
    QTest::newRow("kde_only") << true << false << false;
}

void KUrlMimeDataTest::testMostLocalUrlList()
{
    QFETCH(bool, withKdeUrls);
    QFETCH(bool, withLocalUrls);
    QFETCH(bool, expectedLocalUrls);

    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;
    urls.append(QUrl(QLatin1String("desktop:/foo")));
    urls.append(QUrl(QLatin1String("desktop:/bar")));
    QList<QUrl> localUrls;
    localUrls.append(QUrl(QLatin1String("file:/home/dfaure/Desktop/foo")));
    localUrls.append(QUrl(QLatin1String("file:/home/dfaure/Desktop/bar")));

    if (withKdeUrls && withLocalUrls) {
        KUrlMimeData::setUrls(urls, localUrls, mimeData);
    } else if (withKdeUrls) {
        KUrlMimeData::setUrls(urls, {}, mimeData);
    } else if (withLocalUrls) {
        KUrlMimeData::setUrls({}, localUrls, mimeData);
    }

    QVERIFY(mimeData->hasUrls());
    QVERIFY(mimeData->hasText());
    // The support for urls is done in hasText, a direct call to hasFormat will say false.
    // QVERIFY(mimeData->hasFormat(QLatin1String("text/plain")));

    // urlsFromMimeData decodes the real "kde" urls by default, if any
    QList<QUrl> decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData);
    QVERIFY(!decodedURLs.isEmpty());
    if (expectedLocalUrls) {
        QCOMPARE(decodedURLs, localUrls);
    } else {
        QCOMPARE(decodedURLs, urls);
    }

    // urlsFromMimeData can also be told to decode the "most local" urls
    decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData, KUrlMimeData::PreferLocalUrls);
    QVERIFY(!decodedURLs.isEmpty());
    if (withLocalUrls) {
        QCOMPARE(decodedURLs, localUrls);
    } else {
        QCOMPARE(decodedURLs, urls);
    }

    // QMimeData decodes the "most local" urls
    const QList<QUrl> qurls = mimeData->urls();
    if (withLocalUrls) {
        QCOMPARE(qurls.count(), localUrls.count());
        for (int i = 0; i < qurls.count(); ++i) {
            QCOMPARE(qurls[i], static_cast<QUrl>(localUrls[i]));
        }
    } else {
        QCOMPARE(qurls.count(), 0);
    }

    delete mimeData;
}

#if HAVE_QTDBUS

void KUrlMimeDataTest::testSameProcessCopyPaste()
{
    if (!KUrlMimeData::isDocumentsPortalAvailable()) {
        QSKIP("Documents portal not available");
    }

    std::unique_ptr<QMimeData> mimeData(new QMimeData());
    QList<QUrl> urls{QUrl(QStringLiteral("desktop:/foo")), QUrl(QStringLiteral("desktop:/bar"))};

    KUrlMimeData::setUrls(urls, {}, mimeData.get());
    mimeData->setData(QStringLiteral("application/vnd.portal.filetransfer"), QFile::encodeName(QStringLiteral("bad-transfer-id")));

    // Should fail because the transfer id is invalid
    QList<QUrl> decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData.get());
    QVERIFY(decodedURLs.isEmpty());

    // Should succeed because we put in the source id, so the portal is skipped
    KUrlMimeData::setSourceId(mimeData.get());
    decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData.get());
    QVERIFY(!decodedURLs.isEmpty());
    QCOMPARE(decodedURLs, urls);

    // Should fail because we put in a bad source id, so the portal is not skipped
    mimeData->setData(QStringLiteral("application/x-kde-source-id"), QStringLiteral("bad-source-id").toUtf8());
    decodedURLs = KUrlMimeData::urlsFromMimeData(mimeData.get());
    QVERIFY(decodedURLs.isEmpty());
}

#endif // HAVE_QTDBUS

void KUrlMimeDataTest::initTestCase()
{
#if HAVE_QTDBUS
    qputenv("KCOREADDONS_FORCE_DOCUMENTS_PORTAL", "1"_ba);
#endif
}

#include "moc_kurlmimedatatest.cpp"
