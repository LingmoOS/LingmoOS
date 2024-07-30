/*
    SPDX-FileCopyrightText: 2014 Kevin Krammer <krammer@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <ktranscriptcleantest.h>

#include <QTest>

#include <QDebug>

#include <ktranscript_p.h>

QTEST_MAIN(KTranscriptCleanTest)

KTranscriptCleanTest::KTranscriptCleanTest()
    : m_transcript(nullptr)
{
}

void KTranscriptCleanTest::init()
{
    m_transcript = autotestCreateKTranscriptImp();
}

void KTranscriptCleanTest::cleanup()
{
    autotestDestroyKTranscriptImp();
    m_transcript = nullptr;
}

void KTranscriptCleanTest::test_data()
{
    QTest::addColumn<QVariantList>("argv");
    QTest::addColumn<bool>("fallsBack");
    QTest::addColumn<QString>("expected");

    // Example test case, replace with first clean-slate test
    QTest::newRow("test_basic") << QVariantList{"test_basic", "foo"} << false << "foo bar";
}

void KTranscriptCleanTest::test()
{
    QFETCH(QVariantList, argv);
    QFETCH(bool, fallsBack);
    QFETCH(QString, expected);

    QString language = "fr";
    QString country = "fr";
    QString msgctxt = "a-context";
    QHash<QString, QString> dynamicContext;
    dynamicContext.insert("origin", "neverwhere");
    QString msgid = "source-text";
    QStringList subs;
    subs << "10"
         << "qwyx";
    QList<QVariant> values;
    values << 10 << "qwyx";
    QString ordinaryTranslation = "translated-text";

    QString testJs = QFINDTESTDATA("test.js");
    QList<QStringList> modules;
    modules << (QStringList() << testJs << language);

    QString error;
    bool fallback;
    QString result = m_transcript->eval(argv, language, country, msgctxt, dynamicContext, msgid, subs, values, ordinaryTranslation, modules, error, fallback);

    if (!error.isEmpty()) {
        QFAIL(qPrintable(error));
    }
    if (!fallsBack) {
        QVERIFY(!fallback);
        QCOMPARE(result, expected);
    } else {
        QVERIFY(fallback);
    }
}

#include "moc_ktranscriptcleantest.cpp"
