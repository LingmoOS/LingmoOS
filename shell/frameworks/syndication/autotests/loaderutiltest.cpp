/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "loaderutiltest.h"
#include "loaderutil_p.h"
#include <QTest>
QTEST_GUILESS_MAIN(LoaderUtilTest)

LoaderUtilTest::LoaderUtilTest(QObject *parent)
    : QObject(parent)
{
}

void LoaderUtilTest::testParsing_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QUrl>("urlfound");
    QTest::addRow("bug-383381") << QString(QStringLiteral(SYNDICATION_DATA_FEEDPARSING_DIR) + QStringLiteral("/bug-383381.txt"))
                                << QUrl(QStringLiteral("https://www.youtube.com/user/bigclivedotcom/videos"))
                                << QUrl(QStringLiteral("https://www.youtube.com/feeds/videos.xml?channel_id=UCtM5z2gkrGRuWd0JQMx76qA"));

    QTest::addRow("test-gentoo-url") << QString(QStringLiteral(SYNDICATION_DATA_FEEDPARSING_DIR) + QStringLiteral("/bug-test-gentoo.txt"))
                                     << QUrl(QStringLiteral("https://planet.gentoo.org")) << QUrl(QStringLiteral("https://planet.gentoo.org/rss20.xml"));

    QTest::addRow("bug-484615") << QString(QStringLiteral(SYNDICATION_DATA_FEEDPARSING_DIR) + QStringLiteral("/bug-484615.txt"))
                                << QUrl(QStringLiteral("https://www.qt.io/blog")) << QUrl(QStringLiteral("https://www.qt.io/blog/rss.xml"));
}

void LoaderUtilTest::testParsing()
{
    QFETCH(QString, fileName);
    QFETCH(QUrl, url);
    QFETCH(QUrl, urlfound);
    QFile f(fileName);
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
    const QByteArray ba = f.readAll();
    QCOMPARE(Syndication::LoaderUtil::parseFeed(ba, url), urlfound);
}

#include "moc_loaderutiltest.cpp"
