/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bug286887.h"

#include <kateconfig.h>
#include <katedocument.h>
#include <kateview.h>

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

void BugTest::ctrlShiftLeft()
{
    KTextEditor::DocumentPrivate doc(false, false);

    // view must be visible...
    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    view->show();
    view->resize(400, 300);

    // enable block mode, then set cursor after last character, then shift+left
    doc.clear();
    view->setBlockSelection(true);
    view->setCursorPosition(Cursor(0, 2));
    view->shiftCursorLeft();

    QTest::qWait(500);

    // enable block mode, then set cursor after last character, then delete word left
    doc.clear();
    view->setBlockSelection(true);
    view->setCursorPosition(Cursor(0, 2));
    view->deleteWordLeft();

    QTest::qWait(500);

    // disable wrap-cursor, then set cursor after last character, then shift+left
    doc.clear();
    view->setBlockSelection(false);
    view->setCursorPosition(Cursor(0, 2));
    view->shiftCursorLeft();

    QTest::qWait(500);

    // disable wrap-cursor, then set cursor after last character, then delete word left
    doc.clear();
    view->setCursorPosition(Cursor(0, 2));
    view->deleteWordLeft();
}

#include "moc_bug286887.cpp"
