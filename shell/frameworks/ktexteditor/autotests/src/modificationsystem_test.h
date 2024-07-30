/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2011-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_MODIFICATION_SYSTEM_TEST_H
#define KATE_MODIFICATION_SYSTEM_TEST_H

#include <QObject>

/**
 * Test the complete Line Modification System.
 * Covered classes:
 * - KateModification* in part/undo/
 * - modification flags in Kate::TextLine in part/buffer/
 */
class ModificationSystemTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testInsertText();
    void testRemoveText();

    void testInsertLine();
    void testRemoveLine();

    void testWrapLineMid();
    void testWrapLineAtEnd();
    void testWrapLineAtStart();

    void testUnWrapLine();
    void testUnWrapLine1Empty();
    void testUnWrapLine2Empty();

    void testNavigation();
};

#endif
