/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014 Chusslove Illich <caslav.ilic@gmx.net>
    SPDX-FileCopyrightText: 2014 Kevin Krammer <krammer@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "ktranscripttest.h"

#include "testhelpers.h"

#include <QTest>

#include <QDebug>

#include <ktranscript_p.h>

QTEST_MAIN(KTranscriptTest)

extern "C" {
typedef KTranscript *(*InitFunc)();
}

KTranscriptTest::KTranscriptTest()
    : m_transcript(nullptr)
{
}

void KTranscriptTest::initTestCase()
{
    QVERIFY2(deployTestConfig(), "Could not deploy test ktranscript.ini");

    QString pluginPath = QStringLiteral(KTRANSCRIPT_PATH);
    QVERIFY2(QFile::exists(pluginPath), "Could not find ktranscript plugin");

    m_library.setFileName(pluginPath);

    QVERIFY(m_library.load());
    InitFunc initf = (InitFunc)m_library.resolve("load_transcript");
    QVERIFY(initf);
    m_transcript = initf();
    QVERIFY(m_transcript);
}

void KTranscriptTest::cleanupTestCase()
{
    QVERIFY2(removeTestConfig(), "Could not remove test ktranscript.ini");
}

void KTranscriptTest::test_data()
{
    QTest::addColumn<QVariantList>("argv");
    QTest::addColumn<bool>("fallsBack");
    QTest::addColumn<QString>("expected");

    QTest::newRow("test_basic") << QVariantList{"test_basic", "foo"} << false << "foo bar";
    QTest::newRow("test_unicode") << QVariantList{"test_unicode", "čгσィ九"} << false << "čгσィ九 фу";
    QTest::newRow("test_hascall") << QVariantList{"test_hascall", "test_basic"} << false << "yes";
    QTest::newRow("test_acall") << QVariantList{"test_acall", "test_basic", "qwyx"} << false << "qwyx bar";
    QTest::newRow("test_load") << QVariantList{"test_load"} << false << "foo blurb";
    QTest::newRow("test_fallback") << QVariantList{"test_fallback"} << true << "";
    QTest::newRow("test_msgid") << QVariantList{"test_msgid"} << false << "source-text";
    QTest::newRow("test_msgtrf") << QVariantList{"test_msgtrf"} << false << "translated-text";
    QTest::newRow("test_msgctxt") << QVariantList{"test_msgctxt"} << false << "a-context";
    QTest::newRow("test_msgkey") << QVariantList{"test_msgkey"} << false << "a-context|source-text";
    QTest::newRow("test_nsubs") << QVariantList{"test_nsubs"} << false << "2";
    QTest::newRow("test_subs") << QVariantList{"test_subs", 1} << false << "qwyx";
    QTest::newRow("test_vals") << QVariantList{"test_vals", 0, 5} << false << "50";
    QTest::newRow("test_dynctxt") << QVariantList{"test_dynctxt", "origin"} << false << "neverwhere";
    QTest::newRow("test_dbgputs") << QVariantList{"test_dbgputs"} << false << "debugged";
    QTest::newRow("test_warnputs") << QVariantList{"test_warnputs"} << false << "warned";
    QTest::newRow("test_setcallForall") << QVariantList{"test_setcallForall"} << false << "done";
    QTest::newRow("test_toUpperFirst") << QVariantList{"test_toUpperFirst", "...123 foo"} << false << "...123 Foo";
    QTest::newRow("test_toUpperFirst_unicode") << QVariantList{"test_toUpperFirst", "...123 фу"} << false << "...123 Фу";
    QTest::newRow("test_toLowerFirst") << QVariantList{"test_toLowerFirst", "...123 FOO"} << false << "...123 fOO";
    QTest::newRow("test_loadProps") << QVariantList{"test_loadProps", "cities"} << false << "loaded";
    QTest::newRow("test_getProp") << QVariantList{"test_getProp", "cities", "Athens", "gen"} << false << "Atine";
    QTest::newRow("test_setProp") << QVariantList{"test_setProp", "Oslo", "dat", "Oslou"} << false << "Oslou";
    QTest::newRow("test_normKey") << QVariantList{"test_normKey", "Some &Thing"} << false << "something";
    QTest::newRow("test_getConfString") << QVariantList{"test_getConfString", "StringKey"} << false << "StringValue";
    QTest::newRow("test_getConfStringWithDefault") << QVariantList{"test_getConfStringWithDefault", "NoSuchKey", "DefaultValue"} << false << "DefaultValue";
    QTest::newRow("test_getConfBool") << QVariantList{"test_getConfBool", "BoolKey"} << false << "true";
    QTest::newRow("test_getConfBoolWithDefault") << QVariantList{"test_getConfBoolWithDefault", "NoSuchKey", true} << false << "true";
    QTest::newRow("test_getConfNumber") << QVariantList{"test_getConfNumber", "NumberKey"} << false << "12345";
    QTest::newRow("test_getConfNumberWithDefault") << QVariantList{"test_getConfNumberWithDefault", "NoSuchKey", 54321} << false << "54321";
}

void KTranscriptTest::test()
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

#include "moc_ktranscripttest.cpp"
