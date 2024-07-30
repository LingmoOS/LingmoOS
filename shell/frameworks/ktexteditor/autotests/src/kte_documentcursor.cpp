/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kte_documentcursor.h"

#include <katedocument.h>
#include <ktexteditor/documentcursor.h>
#include <ktexteditor/view.h>

#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(DocumentCursorTest)

using namespace KTextEditor;

DocumentCursorTest::DocumentCursorTest()
    : QObject()
{
}

DocumentCursorTest::~DocumentCursorTest()
{
}

void DocumentCursorTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void DocumentCursorTest::cleanupTestCase()
{
}

// tests:
// - atStartOfDocument
// - atStartOfLine
// - atEndOfDocument
// - atEndOfLine
// - move forward with Wrap
// - move forward with NoWrap
// - move backward
// - gotoNextLine
// - gotoPreviousLine
void DocumentCursorTest::testConvenienceApi()
{
    KTextEditor::DocumentPrivate doc;
    doc.setText(
        QStringLiteral("\n"
                       "1\n"
                       "22\n"
                       "333\n"
                       "4444\n"
                       "55555"));

    // check start and end of document
    DocumentCursor startOfDoc(&doc);
    startOfDoc.setPosition(Cursor(0, 0));
    DocumentCursor endOfDoc(&doc);
    endOfDoc.setPosition(Cursor(5, 5));
    QVERIFY(startOfDoc.atStartOfDocument());
    QVERIFY(startOfDoc.atStartOfLine());
    QVERIFY(endOfDoc.atEndOfDocument());
    QVERIFY(endOfDoc.atEndOfLine());

    // set cursor to (2, 2) and then move to the left two times
    DocumentCursor moving(&doc);
    moving.setPosition(Cursor(2, 2));
    QVERIFY(moving.atEndOfLine()); // at 2, 2
    QVERIFY(moving.move(-1)); // at 2, 1
    QCOMPARE(moving.toCursor(), Cursor(2, 1));
    QVERIFY(!moving.atEndOfLine());
    QVERIFY(moving.move(-1)); // at 2, 0
    QCOMPARE(moving.toCursor(), Cursor(2, 0));
    QVERIFY(moving.atStartOfLine());

    // now move again to the left, should wrap to (1, 1)
    QVERIFY(moving.move(-1)); // at 1, 1
    QCOMPARE(moving.toCursor(), Cursor(1, 1));
    QVERIFY(moving.atEndOfLine());

    // advance 7 characters to position (3, 3)
    QVERIFY(moving.move(7)); // at 3, 3
    QCOMPARE(moving.toCursor(), Cursor(3, 3));

    // advance 20 characters in NoWrap mode, then go back 10 characters
    QVERIFY(moving.move(20, DocumentCursor::NoWrap)); // at 3, 23
    QCOMPARE(moving.toCursor(), Cursor(3, 23));
    QVERIFY(moving.move(-10)); // at 3, 13
    QCOMPARE(moving.toCursor(), Cursor(3, 13));

    // still at invalid text position. move one char to wrap around
    QVERIFY(!moving.isValidTextPosition()); // at 3, 13
    QVERIFY(moving.move(1)); // at 4, 0
    QCOMPARE(moving.toCursor(), Cursor(4, 0));

    // moving 11 characters in wrap mode moves to (5, 6), which is not a valid
    // text position anymore. Hence, moving should be rejected.
    QVERIFY(!moving.move(11));
    QVERIFY(moving.move(10));
    QVERIFY(moving.atEndOfDocument());

    // try to move to next line, which fails. then go to previous line
    QVERIFY(!moving.gotoNextLine());
    QVERIFY(moving.gotoPreviousLine());
    QCOMPARE(moving.toCursor(), Cursor(4, 0));
}

void DocumentCursorTest::testOperators()
{
    KTextEditor::DocumentPrivate doc;
    doc.setText(
        QStringLiteral("--oo--\n"
                       "--oo--\n"
                       "--oo--"));

    // create lots of cursors for comparison
    Cursor invalid = Cursor::invalid();
    Cursor c02(0, 2);
    Cursor c04(0, 4);
    Cursor c14(1, 4);

    DocumentCursor m02(&doc);
    DocumentCursor m04(&doc);
    DocumentCursor m14(&doc);

    QVERIFY(m02 == invalid);
    QVERIFY(m04 == invalid);
    QVERIFY(m14 == invalid);

    m02.setPosition(Cursor(0, 2));
    m04.setPosition(Cursor(0, 4));
    m14.setPosition(Cursor(1, 4));

    // invalid comparison
    // cppcheck-suppress duplicateExpression
    QVERIFY(invalid == invalid);
    QVERIFY(invalid <= c02);
    QVERIFY(invalid < c02);
    QVERIFY(!(invalid > c02));
    QVERIFY(!(invalid >= c02));

    QVERIFY(!(invalid == m02));
    QVERIFY(invalid <= m02);
    QVERIFY(invalid < m02);
    QVERIFY(!(invalid > m02));
    QVERIFY(!(invalid >= m02));

    QVERIFY(!(m02 == invalid));
    QVERIFY(!(m02 <= invalid));
    QVERIFY(!(m02 < invalid));
    QVERIFY(m02 > invalid);
    QVERIFY(m02 >= invalid);

    // DocumentCursor <-> DocumentCursor
    // cppcheck-suppress duplicateExpression
    QVERIFY(m02 == m02);
    // cppcheck-suppress duplicateExpression
    QVERIFY(m02 <= m02);
    // cppcheck-suppress duplicateExpression
    QVERIFY(m02 >= m02);
    // cppcheck-suppress duplicateExpression
    QVERIFY(!(m02 < m02));
    // cppcheck-suppress duplicateExpression
    QVERIFY(!(m02 > m02));
    // cppcheck-suppress duplicateExpression
    QVERIFY(!(m02 != m02));

    QVERIFY(!(m02 == m04));
    QVERIFY(m02 <= m04);
    QVERIFY(!(m02 >= m04));
    QVERIFY(m02 < m04);
    QVERIFY(!(m02 > m04));
    QVERIFY(m02 != m04);

    QVERIFY(!(m04 == m02));
    QVERIFY(!(m04 <= m02));
    QVERIFY(m04 >= m02);
    QVERIFY(!(m04 < m02));
    QVERIFY(m04 > m02);
    QVERIFY(m04 != m02);

    QVERIFY(!(m02 == m14));
    QVERIFY(m02 <= m14);
    QVERIFY(!(m02 >= m14));
    QVERIFY(m02 < m14);
    QVERIFY(!(m02 > m14));
    QVERIFY(m02 != m14);

    QVERIFY(!(m14 == m02));
    QVERIFY(!(m14 <= m02));
    QVERIFY(m14 >= m02);
    QVERIFY(!(m14 < m02));
    QVERIFY(m14 > m02);
    QVERIFY(m14 != m02);

    // DocumentCursor <-> Cursor
    QVERIFY(m02 == c02);
    QVERIFY(m02 <= c02);
    QVERIFY(m02 >= c02);
    QVERIFY(!(m02 < c02));
    QVERIFY(!(m02 > c02));
    QVERIFY(!(m02 != c02));

    QVERIFY(!(m02 == c04));
    QVERIFY(m02 <= c04);
    QVERIFY(!(m02 >= c04));
    QVERIFY(m02 < c04);
    QVERIFY(!(m02 > c04));
    QVERIFY(m02 != c04);

    QVERIFY(!(m04 == c02));
    QVERIFY(!(m04 <= c02));
    QVERIFY(m04 >= c02);
    QVERIFY(!(m04 < c02));
    QVERIFY(m04 > c02);
    QVERIFY(m04 != c02);

    QVERIFY(!(m02 == c14));
    QVERIFY(m02 <= c14);
    QVERIFY(!(m02 >= c14));
    QVERIFY(m02 < c14);
    QVERIFY(!(m02 > c14));
    QVERIFY(m02 != c14);

    QVERIFY(!(m14 == c02));
    QVERIFY(!(m14 <= c02));
    QVERIFY(m14 >= c02);
    QVERIFY(!(m14 < c02));
    QVERIFY(m14 > c02);
    QVERIFY(m14 != c02);

    // Cursor <-> DocumentCursor
    QVERIFY(c02 == m02);
    QVERIFY(c02 <= m02);
    QVERIFY(c02 >= m02);
    QVERIFY(!(c02 < m02));
    QVERIFY(!(c02 > m02));
    QVERIFY(!(c02 != m02));

    QVERIFY(!(c02 == m04));
    QVERIFY(c02 <= m04);
    QVERIFY(!(c02 >= m04));
    QVERIFY(c02 < m04);
    QVERIFY(!(c02 > m04));
    QVERIFY(c02 != m04);

    QVERIFY(!(c04 == m02));
    QVERIFY(!(c04 <= m02));
    QVERIFY(c04 >= m02);
    QVERIFY(!(c04 < m02));
    QVERIFY(c04 > m02);
    QVERIFY(c04 != m02);

    QVERIFY(!(c02 == m14));
    QVERIFY(c02 <= m14);
    QVERIFY(!(c02 >= m14));
    QVERIFY(c02 < m14);
    QVERIFY(!(c02 > m14));
    QVERIFY(c02 != m14);

    QVERIFY(!(c14 == m02));
    QVERIFY(!(c14 <= m02));
    QVERIFY(c14 >= m02);
    QVERIFY(!(c14 < m02));
    QVERIFY(c14 > m02);
    QVERIFY(c14 != m02);
}

void DocumentCursorTest::testValidTextPosition()
{
    // test: utf-32 characters that are visually only one single character (Unicode surrogates)
    // Inside such a utf-32 character, the text position is not valid.
    KTextEditor::DocumentPrivate doc;
    DocumentCursor c(&doc);

    // 0xfffe: byte-order-mark (BOM)
    // 0x002d: '-'
    // 0x3dd8, 0x38de: an utf32-surrogate, see https://www.fileformat.info/info/unicode/char/1f638/browsertest.htm
    const char16_t line0[] = {0xfffe, 0x2d00, 0x3dd8, 0x38de, 0x2d00}; // -xx- where xx is one utf32 char
    const char16_t line1[] = {0xfffe, 0x2d00, 0x3dd8, 0x2d00, 0x2d00}; // -x-- where x is a high surrogate (broken utf32)
    const char16_t line2[] = {0xfffe, 0x2d00, 0x2d00, 0x38de, 0x2d00}; // --x- where x is a low surrogate (broken utf32)
    doc.setText(QString::fromUtf16(line0, 5));
    doc.insertLine(1, QString::fromUtf16(line1, 5));
    doc.insertLine(2, QString::fromUtf16(line2, 5));

    // set to true if you want to see the contents
    const bool showView = false;
    if (showView) {
        doc.createView(nullptr)->show();
        QTest::qWait(5000);
    }

    // line 0: invalid in utf-32 char
    c.setPosition(0, 0);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(0, 1);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(0, 2);
    QVERIFY(!c.isValidTextPosition());
    c.setPosition(0, 3);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(0, 4);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(0, 5);
    QVERIFY(!c.isValidTextPosition());

    // line 1, valid in broken utf-32 char (2nd part missing)
    c.setPosition(1, 0);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(1, 1);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(1, 2);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(1, 3);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(1, 4);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(1, 5);
    QVERIFY(!c.isValidTextPosition());

    // line 2, valid in broken utf-32 char (1st part missing)
    c.setPosition(2, 0);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(2, 1);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(2, 2);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(2, 3);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(2, 4);
    QVERIFY(c.isValidTextPosition());
    c.setPosition(2, 5);
    QVERIFY(!c.isValidTextPosition());

    // test out-of-range
    c.setPosition(-1, 0);
    QVERIFY(!c.isValidTextPosition());
    c.setPosition(3, 0);
    QVERIFY(!c.isValidTextPosition());
    c.setPosition(0, -1);
    QVERIFY(!c.isValidTextPosition());
}

#include "moc_kte_documentcursor.cpp"
