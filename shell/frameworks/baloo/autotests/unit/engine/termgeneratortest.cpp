/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "termgenerator.h"
#include "document.h"

#include <QTest>

using namespace Baloo;

#include <QObject>

class Baloo::TermGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testWordBoundaries();
    void testWordBoundariesCJK();
    void testWordBoundariesCJKMixed();
    void testUnderscoreWord();
    void testUnderscore_splitting();
    void testAccentCharacters();
    void testUnicodeCompatibleComposition();
    void testUnicodeLowering();
    void testEmails();
    void testWordPositions();
    void testWordPositionsCJK();
    void testNumbers();
    void testControlCharacter();
    void testFilePaths();
    void testFilePaths_data();
    void testApostroph();
    void testApostroph_data();

    QList<QByteArray> allWords(const QString& str)
    {
        Document doc;
        TermGenerator termGen(doc);
        termGen.indexText(str);

        return doc.m_terms.keys();
    }
};

void TermGeneratorTest::testWordBoundaries()
{
    QString str = QString::fromLatin1("The quick (\"brown\") 'fox' can't jump 32.3 feet, right? No-Wrong;xx.txt");

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("32.3") << QByteArray("brown") << QByteArray("can't") << QByteArray("feet") << QByteArray("fox") << QByteArray("jump")
                  << QByteArray("no") << QByteArray("quick") << QByteArray("right") << QByteArray("the") << QByteArray("txt") << QByteArray("wrong")
                  << QByteArray("xx");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testWordBoundariesCJK()
{
    QString str = QString::fromUtf8("你");

    QList<QByteArray> words = allWords(str);
    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("你");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testWordBoundariesCJKMixed()
{
    // This is a English and CJK mixed string.
    QString str = QString::fromUtf8("hello world!你好世界貴方元気켐ㅇㄹ？☺");

    QList<QByteArray> words = allWords(str);
    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("hello") << QByteArray("world") << QByteArray("☺") << QByteArray("你好世界貴方元気") << QByteArray("켐ᄋᄅ");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnderscoreWord()
{
    QString str = QString::fromLatin1("_plant");

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("plant");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnderscore_splitting()
{
    QString str = QString::fromLatin1("Hello_Howdy");

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("hello") << QByteArray("howdy");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testAccentCharacters()
{
    QString str = QString::fromUtf8("Como est\xC3\xA1 K\xC3\xBBg"); // "Como está Kûg"

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("como") << QByteArray("esta") << QByteArray("kug");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testUnicodeCompatibleComposition()
{
    // The 0xfb00 corresponds to U+FB00 which is a 'ff' ligature
    QString expected = QLatin1String("maffab");
    QString str = QLatin1String("ma") + QChar(0xfb00) + QStringLiteral("ab");

    QList<QByteArray> words = allWords(str);
    QCOMPARE(words.size(), 1);

    QByteArray output = words.first();
    QCOMPARE(expected.toUtf8(), output);
}

void TermGeneratorTest::testUnicodeLowering()
{
    // This string is unicode mathematical italic "Hedge"
    QString str = QString::fromUtf8("\xF0\x9D\x90\xBB\xF0\x9D\x91\x92\xF0\x9D\x91\x91\xF0\x9D\x91\x94\xF0\x9D\x91\x92");

    QList<QByteArray> words = allWords(str);

    QCOMPARE(words, {QByteArray("hedge")});
}

void TermGeneratorTest::testEmails()
{
    QString str = QString::fromLatin1("me@vhanda.in");

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("in") << QByteArray("me") << QByteArray("vhanda");

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testWordPositions()
{
    Document doc;
    TermGenerator termGen(doc);

    QString str = QString::fromLatin1("Hello hi how hi");
    termGen.indexText(str);

    QList<QByteArray> words = doc.m_terms.keys();

    QList<QByteArray> expectedWords;
    expectedWords << QByteArray("hello") << QByteArray("hi") << QByteArray("how");
    QCOMPARE(words, expectedWords);

    QVector<uint> posInfo1 = doc.m_terms.value("hello").positions;
    QCOMPARE(posInfo1, QVector<uint>() << 1);

    QVector<uint> posInfo2 = doc.m_terms.value("hi").positions;
    QCOMPARE(posInfo2, QVector<uint>() << 2 << 4);

    QVector<uint> posInfo3 = doc.m_terms.value("how").positions;
    QCOMPARE(posInfo3, QVector<uint>() << 3);
}

void TermGeneratorTest::testWordPositionsCJK()
{
    Document doc;
    TermGenerator termGen(doc);

    // This is a Chinese sentence: Hello! I know about you.
    QString str = QString::fromUtf8("你好你好！我认识你。");
    termGen.indexText(str);

    QList<QByteArray> words = doc.m_terms.keys();
    QList<QByteArray> expectedWords;
    // Full width question mark is split point, and the fullwidth period is trimmed.
    expectedWords << QByteArray("你好你好") << QByteArray("我认识你");
    QCOMPARE(words, expectedWords);

    QVector<uint> posInfo1 = doc.m_terms.value("你好你好").positions;
    QCOMPARE(posInfo1, QVector<uint>() << 1);

    QVector<uint> posInfo2 = doc.m_terms.value("我认识你").positions;
    QCOMPARE(posInfo2, QVector<uint>() << 2);
}

void TermGeneratorTest::testNumbers()
{
    QString str = QString::fromLatin1("1 5 10 -3 -12, 5.6, -13.4 -7e3");

    QList<QByteArray> words = allWords(str);

    QList<QByteArray> expectedWords;
    // TODO: Signs are dropped by the TermGenerator
    expectedWords = { "1", "10", "12", "13.4", "3", "5", "5.6", "7e3"};
    QCOMPARE(words, expectedWords);

    expectedWords = { "1", "10", "12", "-13.4", "-3", "5", "5.6", "-7e3"};
    QEXPECT_FAIL("", "signs not handled correctly", Continue);
    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testControlCharacter()
{
    QString str = QString::fromUtf8("word1\u0001word2");

    QList<QByteArray> words = allWords(str);
    QList<QByteArray> expectedWords = { "word1", "word2" };

    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testFilePaths()
{
    QFETCH(QString, input);
    QFETCH(QList<QByteArray>, expectedWords);

    auto words = allWords(input);
    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testFilePaths_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QList<QByteArray>>("expectedWords");

    QTest::addRow("filename with suffix") << QStringLiteral("file.png")
        << QList<QByteArray>({"file", "png"});
    QTest::addRow("filename") << QStringLiteral("foo_bar.png")
        << QList<QByteArray>({"bar", "foo", "png"});
    QTest::addRow("filepath") << QStringLiteral("/foo/bar")
        << QList<QByteArray>({"bar", "foo"});
}

void TermGeneratorTest::testApostroph()
{
    QFETCH(QString, input);
    QFETCH(QList<QByteArray>, expectedWords);

    auto words = allWords(input);
    QCOMPARE(words, expectedWords);
}

void TermGeneratorTest::testApostroph_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QList<QByteArray>>("expectedWords");

    QTest::addRow("Leading") << QStringLiteral("'one two")
        << QList<QByteArray>({"one", "two"});
    QTest::addRow("Middle") << QStringLiteral("one'two three")
        << QList<QByteArray>({"one'two", "three"});
    QTest::addRow("End") << QStringLiteral("one' two")
        << QList<QByteArray>({"one", "two"});
}

QTEST_MAIN(TermGeneratorTest)

#include "termgeneratortest.moc"
