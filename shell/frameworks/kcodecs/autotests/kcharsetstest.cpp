/*
    SPDX-FileCopyrightText: 2011 Romain Perier <bambi@kubuntu.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcharsetstest.h"

#include "kcharsets_p.h"
#include <QDebug>
#include <QString>
#include <QTest>
#include <kcharsets.h>

static bool encodingNameHasADescription(const QString &encodingName, const QStringList &descriptions)
{
    return std::any_of(descriptions.cbegin(), descriptions.cend(), [&encodingName](const QString &description) {
        return description.contains(encodingName);
    });
}

void KCharsetsTest::testSingleton()
{
    QVERIFY(KCharsets::charsets() != nullptr);
    QCOMPARE(KCharsets::charsets(), KCharsets::charsets());
}

void KCharsetsTest::testFromEntity()
{
    KCharsets *singleton = KCharsets::charsets();

    QCOMPARE(singleton->fromEntity(QString::fromLatin1("&#1234")), QChar(1234));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("&#x1234")), QChar(0x1234));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("lt")), QChar::fromLatin1('<'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("gt")), QChar::fromLatin1('>'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("quot")), QChar::fromLatin1('"'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("amp")), QChar::fromLatin1('&'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("apos")), QChar::fromLatin1('\''));
}

void KCharsetsTest::testToEntity()
{
    QSKIP("KCharsets::toEntity test not implemented.");
}

void KCharsetsTest::testResolveEntities()
{
    KCharsets *singleton = KCharsets::charsets();

    QCOMPARE(singleton->resolveEntities(QString::fromLatin1("&quot;&apos;&lt;Hello &amp;World&gt;&apos;&quot;")),
             QString::fromLatin1("\"\'<Hello &World>\'\""));
}

void KCharsetsTest::testEncodingNames()
{
    KCharsets *singleton = KCharsets::charsets();

    QCOMPARE(singleton->availableEncodingNames().count(), singleton->descriptiveEncodingNames().count());

    for (const QString &encodingName : singleton->availableEncodingNames()) {
        QVERIFY(encodingNameHasADescription(encodingName, singleton->descriptiveEncodingNames()));
        QVERIFY(!singleton->descriptionForEncoding(encodingName).isEmpty());
        QCOMPARE(singleton->encodingForName(singleton->descriptionForEncoding(encodingName)), encodingName);
    }
}

QTEST_MAIN(KCharsetsTest)

#include "moc_kcharsetstest.cpp"
