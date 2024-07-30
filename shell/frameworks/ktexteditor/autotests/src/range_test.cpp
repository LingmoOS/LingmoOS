/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2016 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2005 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "range_test.h"
#include "moc_range_test.cpp"

#include <kateconfig.h>
#include <katedocument.h>
#include <kateview.h>
#include <ktexteditor/movingcursor.h>
#include <ktexteditor/movingrange.h>

#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(RangeTest)

#define testNewRow() (QTest::newRow(QString("line %1").arg(__LINE__).toAscii().data()))

RangeTest::RangeTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

RangeTest::~RangeTest()
{
}

void RangeTest::rangeCheck(KTextEditor::Range &valid)
{
    QVERIFY(valid.isValid() && valid.start() <= valid.end());

    KTextEditor::Cursor before(0, 1);
    KTextEditor::Cursor start(0, 2);
    KTextEditor::Cursor end(1, 4);
    KTextEditor::Cursor after(1, 10);

    KTextEditor::Range result(start, end);
    QVERIFY(valid.isValid() && valid.start() <= valid.end());

    valid.setRange(start, end);
    QVERIFY(valid.isValid() && valid.start() <= valid.end());
    QCOMPARE(valid, result);

    valid.setRange(end, start);
    QVERIFY(valid.isValid() && valid.start() <= valid.end());
    QCOMPARE(valid, result);

    valid.setStart(after);
    QVERIFY(valid.isValid() && valid.start() <= valid.end());
    QCOMPARE(valid, KTextEditor::Range(after, after));

    valid = result;
    QCOMPARE(valid, result);

    valid.setEnd(before);
    QVERIFY(valid.isValid() && valid.start() <= valid.end());
    QCOMPARE(valid, KTextEditor::Range(before, before));
}

void RangeTest::testTextEditorRange()
{
    // test simple range
    KTextEditor::Range range;
    rangeCheck(range);
}

void RangeTest::testTextRange()
{
    // test text range
    KTextEditor::DocumentPrivate doc;
    KTextEditor::MovingRange *complexRange = doc.newMovingRange(KTextEditor::Range());
    KTextEditor::Range range = *complexRange;
    rangeCheck(range);
    delete complexRange;
}

void RangeTest::testInsertText()
{
    KTextEditor::DocumentPrivate doc;

    // Multi-line insert
    KTextEditor::MovingCursor *cursor1 = doc.newMovingCursor(KTextEditor::Cursor(), KTextEditor::MovingCursor::StayOnInsert);
    KTextEditor::MovingCursor *cursor2 = doc.newMovingCursor(KTextEditor::Cursor(), KTextEditor::MovingCursor::MoveOnInsert);

    doc.insertText(KTextEditor::Cursor(), QLatin1String("Test Text\nMore Test Text"));
    QCOMPARE(doc.documentEnd(), KTextEditor::Cursor(1, 14));

    QString text = doc.text(KTextEditor::Range(1, 0, 1, 14));
    QCOMPARE(text, QLatin1String("More Test Text"));

    // Check cursors and ranges have moved properly
    QCOMPARE(cursor1->toCursor(), KTextEditor::Cursor(0, 0));
    QCOMPARE(cursor2->toCursor(), KTextEditor::Cursor(1, 14));

    KTextEditor::Cursor cursor3 = doc.endOfLine(1);

    // Set up a few more lines
    doc.insertText(*cursor2, QLatin1String("\nEven More Test Text"));
    QCOMPARE(doc.documentEnd(), KTextEditor::Cursor(2, 19));
    QCOMPARE(cursor3, doc.endOfLine(1));
}

void RangeTest::testCornerCaseInsertion()
{
    KTextEditor::DocumentPrivate doc;

    // lock first revision
    doc.lockRevision(0);

    KTextEditor::MovingRange *rangeEdit = doc.newMovingRange(KTextEditor::Range(0, 0, 0, 0));
    QCOMPARE(rangeEdit->toRange(), KTextEditor::Range(0, 0, 0, 0));

    doc.insertText(KTextEditor::Cursor(0, 0), QLatin1String("\n"));
    QCOMPARE(rangeEdit->toRange(), KTextEditor::Range(1, 0, 1, 0));

    // test translate
    KTextEditor::Range translateTest(0, 0, 0, 0);
    doc.transformRange(translateTest, KTextEditor::MovingRange::DoNotExpand, KTextEditor::MovingRange::AllowEmpty, 0);
    QCOMPARE(translateTest, KTextEditor::Range(1, 0, 1, 0));

    // test translate reverse
    KTextEditor::Range reverseTranslateTest(1, 0, 1, 0);
    doc.transformRange(reverseTranslateTest, KTextEditor::MovingRange::DoNotExpand, KTextEditor::MovingRange::AllowEmpty, -1, 0);
    QCOMPARE(reverseTranslateTest, KTextEditor::Range(0, 0, 0, 0));
}

void RangeTest::testCursorStringConversion()
{
    using KTextEditor::Cursor;

    KTextEditor::Cursor c;
    QCOMPARE(c.line(), 0);
    QCOMPARE(c.column(), 0);
    QCOMPARE(c.toString(), QStringLiteral("(0, 0)"));
    c = Cursor::fromString(QStringLiteral("(0, 0)"));
    QCOMPARE(c.toString(), QStringLiteral("(0, 0)"));
    c = Cursor::fromString(QStringLiteral("(0,0)"));
    QCOMPARE(c.toString(), QStringLiteral("(0, 0)"));

    c.setPosition(-1, -1);
    QCOMPARE(c.toString(), QStringLiteral("(-1, -1)"));
    c = Cursor::fromString(QStringLiteral("(-1, -1)"));
    QCOMPARE(c.toString(), QStringLiteral("(-1, -1)"));
    c = Cursor::fromString(QStringLiteral("(-1,-1)"));
    QCOMPARE(c.toString(), QStringLiteral("(-1, -1)"));

    c.setPosition(12, 42);
    QCOMPARE(c.toString(), QStringLiteral("(12, 42)"));
    c = Cursor::fromString(QStringLiteral("(12, 42)"));
    QCOMPARE(c.toString(), QStringLiteral("(12, 42)"));
    c = Cursor::fromString(QStringLiteral("( 12,42)"));
    QCOMPARE(c.toString(), QStringLiteral("(12, 42)"));

    c.setPosition(12, 42);
    QCOMPARE(c.toString(), QStringLiteral("(12, 42)"));
    c = Cursor::fromString(QStringLiteral("(12, 42)"));
    QCOMPARE(c.toString(), QStringLiteral("(12, 42)"));

    c.setPosition(-12, 42);
    QCOMPARE(c.toString(), QStringLiteral("(-12, 42)"));
    c = Cursor::fromString(QStringLiteral("(-12, 42)"));
    QCOMPARE(c.toString(), QStringLiteral("(-12, 42)"));
    c = Cursor::fromString(QStringLiteral("(-12, +42)"));
    QCOMPARE(c.toString(), QStringLiteral("(-12, 42)"));
    c = Cursor::fromString(QStringLiteral("( -12 ,  +42)"));
    QCOMPARE(c.toString(), QStringLiteral("(-12, 42)"));
    c = Cursor::fromString(QStringLiteral("(-12 , 42 )"));
    QCOMPARE(c.toString(), QStringLiteral("(-12, 42)"));

    // test invalid input
    c = Cursor::fromString(QStringLiteral("( - 12 ,  + 42)"));
    QCOMPARE(c.toString(), QStringLiteral("(-1, -1)"));
    c = Cursor::fromString(QStringLiteral("(, 42)"));
    QCOMPARE(c.toString(), QStringLiteral("(-1, -1)"));
    c = Cursor::fromString(QStringLiteral("(-, -)"));
    QCOMPARE(c.toString(), QStringLiteral("(-1, -1)"));
    c = Cursor::fromString(QStringLiteral("(-, -)"));
    QCOMPARE(c.toString(), QStringLiteral("(-1, -1)"));
    c = Cursor::fromString(QStringLiteral("(-x,y)"));
    QCOMPARE(c.toString(), QStringLiteral("(-1, -1)"));
    c = Cursor::fromString(QStringLiteral("(-3,-2y)"));
    QCOMPARE(c.toString(), QStringLiteral("(-1, -1)"));
}

void RangeTest::testRangeStringConversion()
{
    using KTextEditor::Cursor;
    using KTextEditor::Range;

    KTextEditor::Range r;
    QCOMPARE(r.start(), Cursor(0, 0));
    QCOMPARE(r.end(), Cursor(0, 0));
    QCOMPARE(r.toString(), QStringLiteral("[(0, 0), (0, 0)]"));

    r = Range::fromString(QStringLiteral("[(0, 0), (0, 0)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(0, 0), (0, 0)]"));
    r = Range::fromString(QStringLiteral("[(0,0),(0,0)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(0, 0), (0, 0)]"));
    r = Range::fromString(QStringLiteral("[(-1, -1), (-1, -1)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(-1, -1), (-1, -1)]"));
    r = Range::fromString(QStringLiteral("[(-1, -1), (0, 0)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(-1, -1), (0, 0)]"));
    r = Range::fromString(QStringLiteral("[(0, 0), (-1, -1)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(-1, -1), (0, 0)]")); // start > end -> swap

    r = Range::fromString(QStringLiteral("[(0, 0), (12, 42)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(0, 0), (12, 42)]"));
    r = Range::fromString(QStringLiteral("[(12, 42), (0, 0)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(0, 0), (12, 42)]")); // start > end -> swap
    r = Range::fromString(QStringLiteral("[(12,42),(0,0)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(0, 0), (12, 42)]")); // start > end -> swap
    r = Range::fromString(QStringLiteral("[(-12, -42), (0, 0)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(-12, -42), (0, 0)]"));
    r = Range::fromString(QStringLiteral("[(0, 0), (-12, -42)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(-12, -42), (0, 0)]")); // start > end -> swap

    // invalid input
    r = Range::fromString(QStringLiteral("[(0:0)(-12:-42)]"));
    QCOMPARE(r.toString(), QStringLiteral("[(-1, -1), (-1, -1)]"));
    r = Range::fromString(QStringLiteral("[0,1]"));
    QCOMPARE(r.toString(), QStringLiteral("[(-1, -1), (-1, -1)]"));
}

void RangeTest::testLineRangeStringConversion()
{
    using KTextEditor::LineRange;

    LineRange r;
    QCOMPARE(r.start(), 0);
    QCOMPARE(r.end(), 0);
    QCOMPARE(r.toString(), QStringLiteral("[0, 0]"));

    r = LineRange::fromString(QStringLiteral("[0, 0]"));
    QCOMPARE(r.toString(), QStringLiteral("[0, 0]"));
    r = LineRange::fromString(QStringLiteral("[0,0]"));
    QCOMPARE(r.toString(), QStringLiteral("[0, 0]"));
    r = LineRange::fromString(QStringLiteral("[-1, -1]"));
    QCOMPARE(r.toString(), QStringLiteral("[-1, -1]"));
    r = LineRange::fromString(QStringLiteral("[-1, 0]"));
    QCOMPARE(r.toString(), QStringLiteral("[-1, 0]"));
    r = LineRange::fromString(QStringLiteral("[0, -1]"));
    QCOMPARE(r.toString(), QStringLiteral("[-1, 0]")); // start > end -> swap

    r = LineRange::fromString(QStringLiteral("[12, 42]"));
    QCOMPARE(r.toString(), QStringLiteral("[12, 42]"));
    r = LineRange::fromString(QStringLiteral("[12, 0]"));
    QCOMPARE(r.toString(), QStringLiteral("[0, 12]")); // start > end -> swap
    r = LineRange::fromString(QStringLiteral("[12, 0]"));
    QCOMPARE(r.toString(), QStringLiteral("[0, 12]")); // start > end -> swap
    r = LineRange::fromString(QStringLiteral("[-12, 0]"));
    QCOMPARE(r.toString(), QStringLiteral("[-12, 0]"));
    r = LineRange::fromString(QStringLiteral("[0, -12]"));
    QCOMPARE(r.toString(), QStringLiteral("[-12, 0]")); // start > end -> swap

    // invalid input
    r = LineRange::fromString(QStringLiteral("[0:0]"));
    QCOMPARE(r.toString(), QStringLiteral("[-1, -1]"));
    r = LineRange::fromString(QStringLiteral("[0-1]"));
    QCOMPARE(r.toString(), QStringLiteral("[-1, -1]"));
}

void RangeTest::lineRangeCheck(KTextEditor::LineRange &range)
{
    QVERIFY(range.isValid());
    QVERIFY(range.start() <= range.end());
    QCOMPARE(range.numberOfLines(), range.end() - range.start());
}

void RangeTest::testLineRange()
{
    KTextEditor::LineRange range;
    QCOMPARE(range.start(), 0);
    QCOMPARE(range.end(), 0);
    lineRangeCheck(range);

    range.setRange(3, 5);
    QCOMPARE(range.start(), 3);
    QCOMPARE(range.end(), 5);
    lineRangeCheck(range);

    range.setRange(5, 3);
    QCOMPARE(range.start(), 3);
    QCOMPARE(range.end(), 5);
    lineRangeCheck(range);

    range.setStart(2);
    QCOMPARE(range.start(), 2);
    QCOMPARE(range.end(), 5);
    lineRangeCheck(range);

    range.setStart(6);
    QCOMPARE(range.start(), 6);
    QCOMPARE(range.end(), 6);
    lineRangeCheck(range);

    range.setEnd(8);
    QCOMPARE(range.start(), 6);
    QCOMPARE(range.end(), 8);
    lineRangeCheck(range);

    range.setEnd(4);
    QCOMPARE(range.start(), 4);
    QCOMPARE(range.end(), 4);
    lineRangeCheck(range);
}
