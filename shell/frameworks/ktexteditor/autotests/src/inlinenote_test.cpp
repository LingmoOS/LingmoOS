/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "inlinenote_test.h"
#include "moc_inlinenote_test.cpp"

#include <katedocument.h>
#include <kateview.h>
#include <ktexteditor/inlinenote.h>
#include <ktexteditor/inlinenoteprovider.h>

#include <QPainter>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTest>
#include <QTestMouseEvent>

using namespace KTextEditor;

QTEST_MAIN(InlineNoteTest)

namespace
{
QWidget *findViewInternal(KTextEditor::View *view)
{
    for (QObject *child : view->children()) {
        if (child->metaObject()->className() == QByteArrayLiteral("KateViewInternal")) {
            return qobject_cast<QWidget *>(child);
        }
    }
    return nullptr;
}

class NoteProvider : public InlineNoteProvider
{
public:
    QList<int> inlineNotes(int line) const override
    {
        if (line == 0) {
            return {5, 10};
        }

        return {};
    }

    QSize inlineNoteSize(const InlineNote &note) const override
    {
        if (note.position().column() == 5) {
            const auto xWidth = QFontMetrics(note.font()).boundingRect(QStringLiteral("x")).width();
            return QSize(xWidth, note.lineHeight());
        } else if (note.position().column() == 10) {
            return QSize(note.lineHeight(), note.lineHeight());
        }

        return QSize();
    }

    void paintInlineNote(const InlineNote &note, QPainter &painter, Qt::LayoutDirection) const override
    {
        if (note.position().column() == 5) {
            painter.setPen(Qt::darkGreen);
            painter.setBrush(Qt::green);
            painter.drawEllipse(1, 1, note.width() - 2, note.lineHeight() - 2);
        } else if (note.position().column() == 10) {
            painter.setPen(Qt::darkRed);
            if (note.underMouse()) {
                painter.setBrush(Qt::red);
            } else {
                painter.setBrush(Qt::yellow);
            }
            painter.drawRoundedRect(1, 1, note.width() - 2, note.lineHeight() - 2, 2, 2);
        }
    }

    void inlineNoteActivated(const InlineNote &note, Qt::MouseButtons buttons, const QPoint &globalPos) override
    {
        Q_UNUSED(buttons)
        Q_UNUSED(globalPos)
        ++noteActivatedCount;
        lastUnderMouse = note.underMouse();
    }

    void inlineNoteFocusInEvent(const InlineNote &note, const QPoint &globalPos) override
    {
        Q_UNUSED(globalPos)
        ++focusInCount;
        lastUnderMouse = note.underMouse();
    }

    void inlineNoteFocusOutEvent(const InlineNote &note) override
    {
        ++focusOutCount;
        // Here it should not be under the mosu
        lastUnderMouse = note.underMouse();
    }

    void inlineNoteMouseMoveEvent(const InlineNote &note, const QPoint &globalPos) override
    {
        Q_UNUSED(globalPos)
        ++mouseMoveCount;
        lastUnderMouse = note.underMouse();
    }

public:
    int noteActivatedCount = 0;
    int focusInCount = 0;
    int focusOutCount = 0;
    int mouseMoveCount = 0;
    bool lastUnderMouse = false;
};
}

InlineNoteTest::InlineNoteTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

InlineNoteTest::~InlineNoteTest()
{
}

void InlineNoteTest::testInlineNote()
{
    KTextEditor::DocumentPrivate doc;
    doc.setText(QStringLiteral("xxxxxxxxxx\nxxxxxxxxxx"));

    KTextEditor::ViewPrivate view(&doc, nullptr);
    view.show();
    view.resize(400, 300);

    view.setCursorPosition({0, 5});
    QCOMPARE(view.cursorPosition(), Cursor(0, 5));

    const auto coordCol04 = view.cursorToCoordinate({0, 4});
    const auto coordCol05 = view.cursorToCoordinate({0, 5});
    const auto coordCol10 = view.cursorToCoordinate({0, 10});
    QVERIFY(coordCol05.x() > coordCol04.x());
    QVERIFY(coordCol10.x() > coordCol05.x());

    const auto xWidth = coordCol05.x() - coordCol04.x();

    NoteProvider noteProvider;
    const QList<int> expectedColumns = {5, 10};
    QCOMPARE(noteProvider.inlineNotes(0), expectedColumns);
    QCOMPARE(noteProvider.inlineNotes(1), QList<int>());
    view.registerInlineNoteProvider(&noteProvider);

    const auto newCoordCol04 = view.cursorToCoordinate({0, 4});
    const auto newCoordCol05 = view.cursorToCoordinate({0, 5});
    const auto newCoordCol10 = view.cursorToCoordinate({0, 10});

    QVERIFY(newCoordCol05.x() > newCoordCol04.x());
    QVERIFY(newCoordCol10.x() > newCoordCol05.x());

    QCOMPARE(newCoordCol04, coordCol04);
    QVERIFY(newCoordCol05.x() > coordCol05.x());
    QVERIFY(newCoordCol10.x() > coordCol10.x());

    // so far, we should not have any activation event
    QCOMPARE(noteProvider.noteActivatedCount, 0);
    QCOMPARE(noteProvider.focusInCount, 0);
    QCOMPARE(noteProvider.focusOutCount, 0);
    QCOMPARE(noteProvider.mouseMoveCount, 0);
    QVERIFY(noteProvider.lastUnderMouse == false);

    // mouse move only on X11
    if (qApp->platformName() != QLatin1String("xcb")) {
        view.unregisterInlineNoteProvider(&noteProvider);
        QSKIP("mouse moving only on X11");
    }

    // move mouse onto first note
    auto internalView = findViewInternal(&view);
    QVERIFY(internalView);

    // focus in
    QTest::mouseMove(&view, coordCol05 + QPoint(xWidth / 2, 1));
    QTest::qWait(100);
    QCOMPARE(noteProvider.focusInCount, 1);
    QCOMPARE(noteProvider.focusOutCount, 0);
    QCOMPARE(noteProvider.mouseMoveCount, 0);
    QCOMPARE(noteProvider.noteActivatedCount, 0);
    QVERIFY(noteProvider.lastUnderMouse);

    // move one pixel
    QTest::mouseMove(&view, coordCol05 + QPoint(xWidth / 2 + 1, 1));
    QTest::qWait(100);
    QCOMPARE(noteProvider.focusInCount, 1);
    QCOMPARE(noteProvider.focusOutCount, 0);
    QCOMPARE(noteProvider.mouseMoveCount, 1);
    QCOMPARE(noteProvider.noteActivatedCount, 0);
    QVERIFY(noteProvider.lastUnderMouse);

    // activate
    QTest::mousePress(internalView, Qt::LeftButton, Qt::NoModifier, internalView->mapFromGlobal(view.mapToGlobal(coordCol05 + QPoint(xWidth / 2 + 1, 1))));
    QTest::mouseRelease(internalView, Qt::LeftButton, Qt::NoModifier, internalView->mapFromGlobal(view.mapToGlobal(coordCol05 + QPoint(xWidth / 2 + 1, 1))));
    QTest::qWait(100);
    QCOMPARE(noteProvider.focusInCount, 1);
    QCOMPARE(noteProvider.focusOutCount, 0);
    QCOMPARE(noteProvider.mouseMoveCount, 1);
    QCOMPARE(noteProvider.noteActivatedCount, 1);
    QVERIFY(noteProvider.lastUnderMouse);

    // focus out
    QTest::mouseMove(&view, coordCol04 + QPoint(0, 1));
    QTest::mouseMove(&view, coordCol04 + QPoint(-1, 1));
    QTest::qWait(200);
    QCOMPARE(noteProvider.focusInCount, 1);
    QCOMPARE(noteProvider.focusOutCount, 1);
    QCOMPARE(noteProvider.mouseMoveCount, 1);
    QCOMPARE(noteProvider.noteActivatedCount, 1);
    QVERIFY(noteProvider.lastUnderMouse == false);

    view.unregisterInlineNoteProvider(&noteProvider);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
