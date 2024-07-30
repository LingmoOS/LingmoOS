/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "revision_test.h"
#include "moc_revision_test.cpp"

#include <katebuffer.h>
#include <katedocument.h>
#include <ktexteditor/cursor.h>
#include <ktexteditor/range.h>

#include <QStandardPaths>
#include <QTest>

using namespace KTextEditor;

QTEST_MAIN(RevisionTest)

RevisionTest::RevisionTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

RevisionTest::~RevisionTest()
{
}

// tests: MovingInterface
// - lockRevision()
// - revision()
// - unlockRevision()
// - transformCursor()
void RevisionTest::testTransformCursor()
{
    KTextEditor::DocumentPrivate doc;

    // initial saved revision of unsaved document is -1
    QVERIFY(doc.lastSavedRevision() == -1);

    // initial revision is always 0
    QCOMPARE(doc.revision(), (qint64)0);

    // one edit action -> revision now 1, last saved still -1
    doc.insertText(Cursor(0, 0), QStringLiteral("0000"));

    qint64 rev = doc.revision();
    QCOMPARE(rev, (qint64)1);

    // now lock current revision 1
    doc.lockRevision(rev);

    // wrapLine + insertText + wrapLine + insertText
    doc.insertText(Cursor(0, 2), QStringLiteral("\n1111\n2222"));

    // create some cursors, then transform them
    Cursor c01(0, 1);
    Cursor stayOnInsert(0, 2);
    Cursor moveOnInsert(0, 2);

    doc.transformCursor(c01, MovingCursor::MoveOnInsert, rev, -1);
    doc.transformCursor(moveOnInsert, MovingCursor::MoveOnInsert, rev, -1);
    doc.transformCursor(stayOnInsert, MovingCursor::StayOnInsert, rev, -1);

    QCOMPARE(c01, Cursor(0, 1));
    QCOMPARE(stayOnInsert, Cursor(0, 2));
    QCOMPARE(moveOnInsert, Cursor(2, 4));

    // free revision and lock current again
    doc.unlockRevision(rev);
    rev = doc.revision();
    doc.lockRevision(rev);

    // now undo, the cursors should move to original positions again
    doc.undo();

    // inverse transformation
    doc.transformCursor(c01, MovingCursor::MoveOnInsert, rev, -1);
    doc.transformCursor(moveOnInsert, MovingCursor::MoveOnInsert, rev, -1);
    doc.transformCursor(stayOnInsert, MovingCursor::StayOnInsert, rev, -1);

    QCOMPARE(c01, Cursor(0, 1));
    QCOMPARE(stayOnInsert, Cursor(0, 2));
    QCOMPARE(moveOnInsert, Cursor(0, 2));
}

// tests:
// - transformRange()
void RevisionTest::testTransformRange()
{
    KTextEditor::DocumentPrivate doc;

    QCOMPARE(doc.revision(), (qint64)0);

    doc.setText(
        QStringLiteral("00\n"
                       "11"));

    // now lock current revision
    qint64 rev = doc.revision();
    doc.lockRevision(rev);

    Range r1(Cursor(0, 0), Cursor(1, 2));
    Range r2(Cursor(0, 1), Cursor(1, 1));
    Range invalidOnEmpty(Cursor(0, 1), Cursor(1, 1));

    // remove text
    doc.removeText(Range(Cursor(0, 0), Cursor(1, 2)));

    doc.transformRange(r1, MovingRange::ExpandLeft | MovingRange::ExpandRight, MovingRange::AllowEmpty, rev, -1);
    doc.transformRange(r2, MovingRange::ExpandLeft | MovingRange::ExpandRight, MovingRange::AllowEmpty, rev, -1);
    doc.transformRange(invalidOnEmpty, MovingRange::ExpandLeft | MovingRange::ExpandRight, MovingRange::InvalidateIfEmpty, rev, -1);

    QCOMPARE(r1, Range(Cursor(0, 0), Cursor(0, 0)));
    QCOMPARE(r2, Range(Cursor(0, 0), Cursor(0, 0)));
    QCOMPARE(invalidOnEmpty, Range::invalid());

    // free revision and lock current again
    doc.unlockRevision(rev);
    rev = doc.revision();
    doc.lockRevision(rev);

    // now undo
    doc.undo();

    // r1 should span the entire document
    // r2 should be empty at end of document
    // invalidOnEmpty should stay invalid
    doc.transformRange(r1, MovingRange::ExpandLeft | MovingRange::ExpandRight, MovingRange::AllowEmpty, rev, -1);
    doc.transformRange(r2, MovingRange::ExpandRight, MovingRange::AllowEmpty, rev, -1);
    doc.transformRange(invalidOnEmpty, MovingRange::ExpandLeft | MovingRange::ExpandRight, MovingRange::AllowEmpty, rev, -1);

    QCOMPARE(r1, Range(Cursor(0, 0), Cursor(1, 2)));
    QCOMPARE(r2, Range(Cursor(1, 2), Cursor(1, 2)));
    QCOMPARE(invalidOnEmpty, Range::invalid());
}
