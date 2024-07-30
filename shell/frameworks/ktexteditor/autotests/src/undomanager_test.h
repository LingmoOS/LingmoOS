/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2009-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_UNDOMANAGER_TEST_H
#define KATE_UNDOMANAGER_TEST_H

#include <QObject>

class UndoManagerTest : public QObject
{
    Q_OBJECT

public:
    UndoManagerTest();

private Q_SLOTS:
    void testUndoRedoCount();
    void testSafePoint();
    void testCursorPosition();
    void testSelectionUndo();
    void testUndoWordWrapBug301367();
    void testUndoIndentBug373009();
    void testUndoAfterPastingWrappingLine();
};

#endif
