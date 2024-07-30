/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backtraceparsertest.h"
#include <QMetaEnum>
#include <QSharedPointer>

#define DATA_DIR QFINDTESTDATA("backtraceparsertest_data")

BacktraceParserTest::BacktraceParserTest(QObject *parent)
    : QObject(parent)
    , m_settings(DATA_DIR + QLatin1Char('/') + QStringLiteral("data.ini"), QSettings::IniFormat)
    , m_generator(new FakeBacktraceGenerator(this))
{
}

void BacktraceParserTest::fetchData(const QString &group)
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");
    QTest::addColumn<QString>("debugger");
    QTest::addColumn<bool>("compositorCrash");

    m_settings.beginGroup(group);
    const QStringList keys = m_settings.allKeys();
    m_settings.endGroup();

    for (const QString &key : keys) {
        QTest::newRow(qPrintable(key)) << QString(DATA_DIR + QLatin1Char('/') + key) << m_settings.value(group + QLatin1Char('/') + key).toString()
                                       << m_settings.value(QStringLiteral("debugger/") + key).toString()
                                       << m_settings.value(QStringLiteral("compositorCrash/") + key).toBool();
    }
}

void BacktraceParserTest::btParserUsefulnessTest_data()
{
    fetchData(QStringLiteral("usefulness"));
}

void BacktraceParserTest::btParserUsefulnessTest()
{
    QFETCH(QString, filename);
    QFETCH(QString, result);
    QFETCH(QString, debugger);

    // parse
    QSharedPointer<BacktraceParser> parser(BacktraceParser::newParser(debugger));
    parser->connectToGenerator(m_generator);
    m_generator->sendData(filename);

    // convert usefulness to string
    QMetaEnum metaUsefulness = BacktraceParser::staticMetaObject.enumerator(BacktraceParser::staticMetaObject.indexOfEnumerator("Usefulness"));
    QString btUsefulness = QString::fromLatin1(metaUsefulness.valueToKey(parser->backtraceUsefulness()));

    // compare
    QEXPECT_FAIL("test_e", "Working on it", Continue);
    QCOMPARE(btUsefulness, result);
}

void BacktraceParserTest::btParserBenchmark_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("debugger");

    m_settings.beginGroup(QStringLiteral("debugger"));
    const QStringList keys = m_settings.allKeys();
    for (const QString &key : keys) {
        QTest::newRow(qPrintable(key)) << QString(DATA_DIR + QLatin1Char('/') + key) << m_settings.value(key).toString();
    }
    m_settings.endGroup();
}

void BacktraceParserTest::btParserBenchmark()
{
    QFETCH(QString, filename);
    QFETCH(QString, debugger);

    QSharedPointer<BacktraceParser> parser(BacktraceParser::newParser(debugger));
    parser->connectToGenerator(m_generator);

    QBENCHMARK_ONCE {
        m_generator->sendData(filename);
    }
}

void BacktraceParserTest::btParserCompositorCrashTest_data()
{
    fetchData(QStringLiteral("compositorCrash"));
}

void BacktraceParserTest::btParserCompositorCrashTest()
{
    QFETCH(QString, filename);
    QFETCH(QString, debugger);
    QFETCH(bool, compositorCrash);
    // parse
    QSharedPointer<BacktraceParser> parser(BacktraceParser::newParser(debugger));
    parser->connectToGenerator(m_generator);
    m_generator->sendData(filename);

    // compare
    QCOMPARE(parser->hasCompositorCrashed(), compositorCrash);
}

QTEST_GUILESS_MAIN(BacktraceParserTest)

#include "moc_backtraceparsertest.cpp"
