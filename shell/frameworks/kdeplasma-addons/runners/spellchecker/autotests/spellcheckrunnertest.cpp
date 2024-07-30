/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QObject>
#include <QTest>

#include "../spellcheck.h"

class SpellCheckRunnerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testDefaultDictionary();
    void testSpecifiedDictionary();
    void testAutomaticDictionary();
    void testSuggestions();

private:
    bool m_isDefaultDictonaryUS;
    bool m_hasUSDictonary;
    bool m_hasRUDictionary;
};

void SpellCheckRunnerTest::initTestCase()
{
    Sonnet::Speller defaultDictonary = Sonnet::Speller(QString());

    m_isDefaultDictonaryUS = defaultDictonary.language() == QStringLiteral("en_US");
    m_hasUSDictonary = defaultDictonary.availableLanguages().contains(QStringLiteral("en_US"));
    m_hasRUDictionary = defaultDictonary.availableLanguages().contains(QStringLiteral("ru_RU"));
}

void SpellCheckRunnerTest::cleanupTestCase()
{
}

void SpellCheckRunnerTest::testDefaultDictionary()
{
    if (!m_isDefaultDictonaryUS) {
        QSKIP("The default dictionary is not en_US.");
    }

    SpellCheckRunner runner(this, KPluginMetaData());
    runner.loadData();

    KRunner::RunnerContext context;
    context.setQuery(QStringLiteral("hello"));
    runner.match(context);
    const auto matches = context.matches();
    QCOMPARE(matches.count(), 1);
    QCOMPARE(matches.constFirst().text(), QStringLiteral("hello"));
    QCOMPARE(matches.constFirst().iconName(), QStringLiteral("checkbox"));
}

void SpellCheckRunnerTest::testSpecifiedDictionary()
{
    if (!m_hasUSDictonary) {
        QSKIP("en_US dictionary is not available.");
    }

    SpellCheckRunner runner(this, KPluginMetaData());
    runner.loadData();

    KRunner::RunnerContext context;

    // Test exact match
    context.setQuery(QStringLiteral("en_US hello"));
    runner.match(context);
    auto matches = context.matches();
    QCOMPARE(matches.count(), 1);
    QCOMPARE(matches.constFirst().text(), QStringLiteral("hello"));
    QCOMPARE(matches.constFirst().iconName(), QStringLiteral("checkbox"));

    // Test case-insensitive match
    context.setQuery(QStringLiteral("en_us hello"));
    runner.match(context);
    matches = context.matches();
    QCOMPARE(matches.count(), 1);
    QCOMPARE(matches.constFirst().text(), QStringLiteral("hello"));
    QCOMPARE(matches.constFirst().iconName(), QStringLiteral("checkbox"));

    // Test startsWith
    context.setQuery(QStringLiteral("en hello"));
    runner.match(context);
    matches = context.matches();
    QCOMPARE(matches.count(), 1);
    QCOMPARE(matches.constFirst().text(), QStringLiteral("hello"));
    QCOMPARE(matches.constFirst().iconName(), QStringLiteral("checkbox"));

    if (m_hasRUDictionary) {
        context.setQuery(QStringLiteral("ru_RU мама"));
        runner.match(context);
        auto matches = context.matches();
        QCOMPARE(matches.count(), 1);
        QCOMPARE(matches.constFirst().text(), QStringLiteral("мама"));
        QCOMPARE(matches.constFirst().iconName(), QStringLiteral("checkbox"));
    }
}

void SpellCheckRunnerTest::testAutomaticDictionary()
{
    if (!m_isDefaultDictonaryUS) {
        QSKIP("The default dictionary is not en_US.");
    }

    if (!m_hasRUDictionary) {
        QSKIP("ru_RU dictionary is not available.");
    }

    SpellCheckRunner runner(this, KPluginMetaData());
    runner.loadData();

    KRunner::RunnerContext context;

    context.setQuery(QStringLiteral("мама"));
    runner.match(context);
    auto matches = context.matches();
    QCOMPARE(matches.count(), 1);
    QCOMPARE(matches.constFirst().text(), QStringLiteral("мама"));
    QCOMPARE(matches.constFirst().iconName(), QStringLiteral("checkbox"));

    // When a language code is specified but the search term is not in the language.
    context.setQuery(QStringLiteral("en_US мама"));
    runner.match(context);
    matches = context.matches();
    QCOMPARE(matches.count(), 0);
}

void SpellCheckRunnerTest::testSuggestions()
{
    if (!m_hasUSDictonary) {
        QSKIP("en_US dictionary is not available.");
    }

    SpellCheckRunner runner(this, KPluginMetaData());
    runner.loadData();

    KRunner::RunnerContext context;
    context.setQuery(QStringLiteral("hallo"));

    runner.match(context);
    const auto matches = context.matches();
    QVERIFY(std::any_of(matches.cbegin(), matches.cend(), [](const KRunner::QueryMatch &match) {
        return match.text() == QStringLiteral("hello") && match.iconName() == QStringLiteral("edit-rename");
    }));
}

QTEST_MAIN(SpellCheckRunnerTest)

#include "spellcheckrunnertest.moc"
