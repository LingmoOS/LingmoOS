/*
    SPDX-FileCopyrightText: 2014-2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include "kosrelease.h"

class KOSReleaseTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParse()
    {
        KOSRelease r(QFINDTESTDATA("data/os-release"));
        QCOMPARE(r.name(), QStringLiteral("Name"));
        QCOMPARE(r.version(), QStringLiteral("100.5"));
        QCOMPARE(r.id(), QStringLiteral("theid"));
        QCOMPARE(r.idLike(), QStringList({QStringLiteral("otherid"), QStringLiteral("otherotherid")}));
        QCOMPARE(r.versionCodename(), QStringLiteral("versioncodename"));
        QCOMPARE(r.versionId(), QStringLiteral("500.1"));
        QCOMPARE(r.prettyName(), QStringLiteral("Vicuña — Pretty Name #1"));
        QCOMPARE(r.ansiColor(), QStringLiteral("1;34"));
        QCOMPARE(r.cpeName(), QStringLiteral("cpe:/o:foo:bar:100"));
        QCOMPARE(r.homeUrl(), QStringLiteral("https://url.home"));
        QCOMPARE(r.documentationUrl(), QStringLiteral("https://url.docs"));
        QCOMPARE(r.supportUrl(), QStringLiteral("https://url.support"));
        QCOMPARE(r.bugReportUrl(), QStringLiteral("https://url.bugs"));
        QCOMPARE(r.privacyPolicyUrl(), QStringLiteral("https://url.privacy"));
        QCOMPARE(r.buildId(), QStringLiteral("105.5"));
        QCOMPARE(r.variant(), QStringLiteral("Test = Edition"));
        QCOMPARE(r.variantId(), QStringLiteral("test"));
        QCOMPARE(r.logo(), QStringLiteral("start-here-test"));
        QCOMPARE(r.extraKeys(), QStringList({QStringLiteral("DEBIAN_BTS")}));
        QCOMPARE(r.extraValue(QStringLiteral("DEBIAN_BTS")), QStringLiteral("debbugs://bugs.debian.org/"));
    }
};

QTEST_MAIN(KOSReleaseTest)

#include "kosreleasetest.moc"
