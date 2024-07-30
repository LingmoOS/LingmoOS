/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bug313769.h"
#include "moc_bug313769.cpp"

#include <katebuffer.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <kateview.h>
#include <ktexteditor/range.h>

#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(BugTest)

using namespace KTextEditor;

BugTest::BugTest()
    : QObject()
{
}

BugTest::~BugTest()
{
}

void BugTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void BugTest::cleanupTestCase()
{
}

void BugTest::tryCrash()
{
    KTextEditor::DocumentPrivate doc(false, false);
    const QUrl url = QUrl::fromLocalFile(QLatin1String(TEST_DATA_DIR "bug313769.cpp"));
    doc.openUrl(url);
    doc.discardDataRecovery();
    doc.setHighlightingMode(QStringLiteral("C++"));
    doc.buffer().ensureHighlighted(doc.lines());

    // view must be visible...
    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    view->show();
    view->resize(900, 800);
    view->config()->setDynWordWrap(true);
    view->setSelection(Range(2, 0, 74, 0));
    view->setCursorPosition(Cursor(74, 0));

    doc.editStart();
    QString text = doc.line(1);
    doc.insertLine(74, text);
    doc.removeLine(1);
    view->setCursorPosition(Cursor(1, 0));
    doc.editEnd();

    QTest::qWait(200);
    // fold toplevel nodes
    for (int line = 0; line < doc.lines(); ++line) {
        if (view->textFolding().isLineVisible(line)) {
            view->foldLine(line);
        }
    }
    doc.buffer().ensureHighlighted(doc.lines());

    view->setCursorPosition(Cursor(0, 0));

    QTest::qWait(100);
    doc.undo();
    QTest::qWait(100);
    doc.redo();
    QTest::qWait(500);
    qDebug() << "!!! Does undo crash?";
    doc.undo();

    QTest::qWait(500);
    qDebug() << "!!! No crash (qWait not long enough)? Nice!";
}
