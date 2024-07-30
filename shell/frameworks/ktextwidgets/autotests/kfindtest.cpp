/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2004 Arend van Beelen jr. <arend@auton.nl>
    SPDX-FileCopyrightText: 2010 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kfindtest.h"

#include <kfind.h>

#include <QRegularExpression>
#include <QTest>

#include <assert.h>

void KFindRecorder::changeText(int line, const QString &text)
{
    Q_ASSERT(line < m_text.count());
    Q_ASSERT(m_find != nullptr);

    m_line = line;
    m_text[line] = text;
    m_find->setData(line, text);
}

KFindRecorder::KFindRecorder(const QStringList &text)
    : QObject(nullptr)
    , m_text(text)
    , m_line(0)
{
}

KFindRecorder::~KFindRecorder()
{
}

void KFindRecorder::find(const QString &pattern, long options)
{
    m_find.reset(new KFind(pattern, options, nullptr));
    // Prevent dialogs from popping up
    m_find->closeFindNextDialog();

    connect(m_find.get(), &KFind::textFound, this, [this](const QString &text, int matchingIndex, int matchedLength) {
        slotHighlight(text, matchingIndex, matchedLength);
    });

    connect(m_find.get(), &KFind::textFoundAtId, this, [this](int id, int matchingIndex, int matchedLength) {
        slotHighlight(id, matchingIndex, matchedLength);
    });

    m_line = 0;
    KFind::Result result = KFind::NoMatch;
    do {
        if (options & KFind::FindIncremental) {
            m_find->setData(m_line, m_text[m_line]);
        } else {
            m_find->setData(m_text[m_line]);
        }

        m_line++;

        result = m_find->find();
    } while (result == KFind::NoMatch && m_line < m_text.count());
}

bool KFindRecorder::findNext(const QString &pattern)
{
    Q_ASSERT(m_find != nullptr);

    if (!pattern.isNull()) {
        m_find->setPattern(pattern);
    }

    KFind::Result result = KFind::NoMatch;
    do {
        // qDebug() << "m_line: " << m_line;

        result = m_find->find();

        if (result == KFind::NoMatch && m_line < m_text.count()) {
            // qDebug() << "incrementing m_line...";
            if (m_find->options() & KFind::FindIncremental) {
                m_find->setData(m_line, m_text[m_line]);
            } else {
                m_find->setData(m_text[m_line]);
            }

            m_line++;
        }
    } while (result == KFind::NoMatch && m_line < m_text.count());
    // qDebug() << "find next completed" << m_line;

    return result != KFind::NoMatch;
}

void KFindRecorder::slotHighlight(const QString &text, int index, int matchedLength)
{
    m_hits.append(QLatin1String("line: \"") + text + QLatin1String("\", index: ") + QString::number(index) + QLatin1String(", length: ")
                  + QString::number(matchedLength) + QLatin1Char('\n'));
}

void KFindRecorder::slotHighlight(int id, int index, int matchedLength)
{
    m_hits.append(QLatin1String("line: \"") + m_text[id] + QLatin1String("\", index: ") + QString::number(index) + QLatin1String(", length: ")
                  + QString::number(matchedLength) + QLatin1Char('\n'));
}

////

TestKFind::TestKFind()
    : QObject()
{
    m_text = QLatin1String("This file is part of the KDE project.\n") + QLatin1String("This library is free software; you can redistribute it and/or\n")
        + QLatin1String("modify it under the terms of the GNU Library General Public\n")
        + QLatin1String("License version 2, as published by the Free Software Foundation.\n") + QLatin1Char('\n')
        + QLatin1String("    This library is distributed in the hope that it will be useful,\n")
        + QLatin1String("    but WITHOUT ANY WARRANTY; without even the implied warranty of\n")
        + QLatin1String("    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n")
        + QLatin1String("    Library General Public License for more details.\n") + QLatin1Char('\n')
        + QLatin1String("    You should have received a copy of the GNU Library General Public License\n")
        + QLatin1String("    along with this library; see the file COPYING.LIB.  If not, write to\n")
        + QLatin1String("    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n") + QLatin1String("    Boston, MA 02110-1301, USA.\n");
}

void TestKFind::testStaticFindRegexp_data()
{
    // Tests for the core method "static KFind::find"
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<int>("startIndex");
    QTest::addColumn<int>("options");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<int>("expectedMatchedLength");

    /* clang-format off */
    QTest::newRow("simple (0)") << "abc" << "a" << 0 << 0 << 0 << 1;
    QTest::newRow("simple (1)") << "abc" << "b" << 0 << 0 << 1 << 1;
    QTest::newRow("not found") << "abca" << "ba" << 0 << 0 << -1 << 0;
    QTest::newRow("from index") << "abc bc" << "b" << 3 << 0 << 4 << 1;
    QTest::newRow("from exact index") << "abc bc" << "b" << 4 << 0 << 4 << 1;
    QTest::newRow("past index (not found)") << "abc bc" << "b" << 5 << 0 << -1 << 0;
    QTest::newRow("dot") << "abc" << "b." << 0 << 0 << 1 << 2;
    QTest::newRow("^simple") << "text" << "^tex" << 0 << 0 << 0 << 3;
    QTest::newRow("^multiline first") << "foo\nbar" << "^f" << 0 << 0 << 0 << 1;
    QTest::newRow("^multiline last") << "foo\nbar" << "^bar" << 0 << 0 << 4 << 3;
    QTest::newRow("^multiline with index") << "boo\nbar" << "^b" << 1 << 0 << 4 << 1;
    QTest::newRow("simple$") << "text" << "xt$" << 0 << 0 << 2 << 2;
    QTest::newRow("$ backwards") << "text" << "xt$" << 4 << int(KFind::FindBackwards) << 2 << 2;
    QTest::newRow("multiline$") << "foo\nbar" << "oo$" << 0 << 0 << 1 << 2;
    QTest::newRow("multiline$ intermediary line") << "foo\nbar\nagain bar" << "r$" << 0 << 0 << 6 << 1;
    QTest::newRow("multiline$ with index, last line") << "foo\nbar\nagain bar" << "r$" << 7 << 0 << 16 << 1;
    QTest::newRow("multiline$ backwards") << "foo\nbar" << "oo$" << 7 << int(KFind::FindBackwards) << 1 << 2;
    QTest::newRow("multiline with \\n") << "foo\nbar" << "o\nb" << 0 << 0 << 2 << 3;
    QTest::newRow("whole words ok") << "abc bcbc bc bmore be" << "b." << 0 << int(KFind::WholeWordsOnly) << 9 << 2;
    QTest::newRow("whole words not found") << "abab abx" << "ab" << 0 << int(KFind::WholeWordsOnly) << -1 << 0;
    QTest::newRow("whole words not found (_)") << "abab ab_" << "ab" << 0 << int(KFind::WholeWordsOnly) << -1 << 0;
    QTest::newRow("whole words ok (.)") << "ab." << "ab" << 0 << int(KFind::WholeWordsOnly) << 0 << 2;
    QTest::newRow("backwards") << "abc bcbc bc" << "b." << 10 << int(KFind::FindBackwards) << 9 << 2;
    QTest::newRow("empty (0)") << "a" << "" << 0 << int(0) << 0 << 0;
    QTest::newRow("empty (1)") << "a" << "" << 1 << int(0) << 1 << 0; // kreplacetest testReplaceBlankSearch relies on this
    QTest::newRow("at end, not found") << "a" << "b" << 1 << int(0) << -1 << 0; // just for catching the while(index<text.length()) bug
    QTest::newRow("back, not found") << "a" << "b" << 0 << int(KFind::FindBackwards) << -1 << 0;
    QTest::newRow("back, at begin, found") << "a" << "a" << 0 << int(KFind::FindBackwards) << 0 << 1;
    QTest::newRow("back, at end, found") << "a" << "a" << 1 << int(KFind::FindBackwards) << 0 << 1;
    QTest::newRow("back, text shorter than pattern") << "a" << "abcd" << 0 << int(KFind::FindBackwards) << -1 << 0;
    /* clang-format on */
}

void TestKFind::testStaticFindRegexp()
{
    // Tests for the core method "static KFind::find(text, regexp)"
    QFETCH(QString, text);
    QFETCH(QString, pattern);
    QFETCH(int, startIndex);
    QFETCH(int, options);
    QFETCH(int, expectedResult);
    QFETCH(int, expectedMatchedLength);

    int matchedLength = 0;
    const int result2 = KFind::find(text, pattern, startIndex, options | KFind::RegularExpression, &matchedLength, nullptr);
    QCOMPARE(result2, expectedResult);
    QCOMPARE(matchedLength, expectedMatchedLength);
}

void TestKFind::testRegexpUnicode_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<int>("startIndex");
    QTest::addColumn<int>("options");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<int>("expectedMatchedLength");

    /* clang-format off */
    // Test matching with Unicode properties in QRegularExpression
    QTest::newRow("unicode-word-boundary") << "aoé" << "\\b" << 1 << 0 << 3 << 0;
    QTest::newRow("unicode-word-char") << "aoé" << "\\w$" << 0 << 0 << 2 << 1;
    QTest::newRow("unicode-non-word-char") << "aoé" << "\\W" << 0 << 0 << -1 << 0;
    /* clang-format on */
}

void TestKFind::testRegexpUnicode()
{
    // Tests for the core method "static KFind::find(text, regexp)"
    QFETCH(QString, text);
    QFETCH(QString, pattern);
    QFETCH(int, startIndex);
    QFETCH(int, options);
    QFETCH(int, expectedResult);
    QFETCH(int, expectedMatchedLength);

    int matchedLength = 0;

    const int result = KFind::find(text, pattern, startIndex, options | KFind::RegularExpression, &matchedLength, nullptr);
    QCOMPARE(result, expectedResult);
    QCOMPARE(matchedLength, expectedMatchedLength);
}

void TestKFind::testSimpleSearch()
{
    // first we do a simple text searching the text and doing a few find nexts
    KFindRecorder test(m_text.split(QLatin1Char('\n')));
    test.find(QStringLiteral("This"), 0);
    while (test.findNext()) { }

    const QString output1 = QLatin1String("line: \"This file is part of the KDE project.\", index: 0, length: 4\n")
        + QLatin1String("line: \"This library is free software; you can redistribute it and/or\", index: 0, length: 4\n")
        + QLatin1String("line: \"    This library is distributed in the hope that it will be useful,\", index: 4, length: 4\n")
        + QLatin1String("line: \"    along with this library; see the file COPYING.LIB.  If not, write to\", index: 15, length: 4\n");

    QCOMPARE(test.hits().join(QString()), output1);
}

void TestKFind::testSimpleRegexp()
{
    KFindRecorder test(m_text.split(QLatin1Char('\n')));
    test.find(QStringLiteral("W.R+ANT[YZ]"), KFind::RegularExpression | KFind::CaseSensitive);
    while (test.findNext()) { }
    const QString output = QStringLiteral("line: \"    but WITHOUT ANY WARRANTY; without even the implied warranty of\", index: 20, length: 8\n");
    QCOMPARE(test.hits().join(QString()), output);
}

void TestKFind::testLineBeginRegularExpression()
{
    int matchedLength;
    QRegularExpressionMatch match;
    KFind::find(m_text, QStringLiteral("^License.+"), 0, KFind::RegularExpression, &matchedLength, &match);
    QCOMPARE(match.captured(0), QStringLiteral("License version 2, as published by the Free Software Foundation."));
}

void TestKFind::testFindIncremental()
{
    // FindIncremental with static contents...

    KFindRecorder test(m_text.split(QLatin1Char('\n')));
    test.find(QString(), KFind::FindIncremental);
    test.findNext(QStringLiteral("i"));
    test.findNext(QStringLiteral("is"));
    test.findNext(QStringLiteral("ist"));
    test.findNext();
    test.findNext(QStringLiteral("istri"));
    test.findNext(QStringLiteral("istr"));
    test.findNext(QStringLiteral("ist"));
    test.findNext(QStringLiteral("is"));
    test.findNext(QStringLiteral("W"));
    test.findNext(QStringLiteral("WA"));
    test.findNext(QStringLiteral("WARRANTY"));
    test.findNext(QStringLiteral("Free"));
    test.findNext(QStringLiteral("Software Foundation"));

    const QString output2 = QLatin1String("line: \"This file is part of the KDE project.\", index: 0, length: 0\n")
        + QLatin1String("line: \"This file is part of the KDE project.\", index: 2, length: 1\n")
        + QLatin1String("line: \"This file is part of the KDE project.\", index: 2, length: 2\n")
        + QLatin1String("line: \"This library is free software; you can redistribute it and/or\", index: 42, length: 3\n")
        + QLatin1String("line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 3\n")
        + QLatin1String("line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 5\n")
        + QLatin1String("line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 4\n")
        + QLatin1String("line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 3\n")
        + QLatin1String("line: \"This file is part of the KDE project.\", index: 2, length: 2\n")
        + QLatin1String("line: \"This library is free software; you can redistribute it and/or\", index: 25, length: 1\n")
        + QLatin1String("line: \"This library is free software; you can redistribute it and/or\", index: 25, length: 2\n")
        + QLatin1String("line: \"    but WITHOUT ANY WARRANTY; without even the implied warranty of\", index: 20, length: 8\n")
        + QLatin1String("line: \"This library is free software; you can redistribute it and/or\", index: 16, length: 4\n")
        + QLatin1String("line: \"License version 2, as published by the Free Software Foundation.\", index: 44, length: 19\n");

    QCOMPARE(test.hits().join(QString()), output2);
}

void TestKFind::testFindIncrementalDynamic()
{
    // Now do that again but with pages that change between searches
    KFindRecorder test(m_text.split(QLatin1Char('\n')));

    test.find(QString(), KFind::FindIncremental);
    test.findNext(QStringLiteral("i"));
    test.findNext(QStringLiteral("is"));
    test.findNext(QStringLiteral("ist"));
    test.findNext(QStringLiteral("istr"));
    test.findNext();
    test.changeText(1, QStringLiteral("The second line now looks a whole lot different."));
    test.findNext(QStringLiteral("istri"));
    test.findNext(QStringLiteral("istr"));
    test.findNext(QStringLiteral("ist"));
    test.findNext(QStringLiteral("is"));
    test.findNext(QStringLiteral("i"));
    test.findNext(QStringLiteral("W"));
    test.findNext(QStringLiteral("WA"));
    test.findNext(QStringLiteral("WARRANTY"));
    test.changeText(6, QStringLiteral("    but WITHOUT ANY xxxx; without even the implied warranty of"));
    test.findNext(QStringLiteral("WARRAN"));
    test.findNext(QStringLiteral("Free"));
    test.findNext(QStringLiteral("Software Foundation"));

    const QString output3 = QLatin1String("line: \"This file is part of the KDE project.\", index: 0, length: 0\n")
        + QLatin1String("line: \"This file is part of the KDE project.\", index: 2, length: 1\n")
        + QLatin1String("line: \"This file is part of the KDE project.\", index: 2, length: 2\n")
        + QLatin1String("line: \"This library is free software; you can redistribute it and/or\", index: 42, length: 3\n")
        + QLatin1String("line: \"This library is free software; you can redistribute it and/or\", index: 42, length: 4\n")
        + QLatin1String("line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 4\n")
        + QLatin1String("line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 5\n")
        + QLatin1String("line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 4\n")
        + QLatin1String("line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 3\n")
        + QLatin1String("line: \"This file is part of the KDE project.\", index: 2, length: 2\n")
        + QLatin1String("line: \"This file is part of the KDE project.\", index: 2, length: 1\n")
        + QLatin1String("line: \"The second line now looks a whole lot different.\", index: 18, length: 1\n")
        + QLatin1String("line: \"License version 2, as published by the Free Software Foundation.\", index: 48, length: 2\n")
        + QLatin1String("line: \"    but WITHOUT ANY WARRANTY; without even the implied warranty of\", index: 20, length: 8\n")
        + QLatin1String("line: \"    but WITHOUT ANY xxxx; without even the implied warranty of\", index: 51, length: 6\n")
        + QLatin1String("line: \"License version 2, as published by the Free Software Foundation.\", index: 39, length: 4\n")
        + QLatin1String("line: \"License version 2, as published by the Free Software Foundation.\", index: 44, length: 19\n");

    QCOMPARE(test.hits().join(QString()), output3);
}

QTEST_MAIN(TestKFind)

#include "moc_kfindtest.cpp"
