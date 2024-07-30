/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MODES_TEST_H
#define MODES_TEST_H

#include "base.h"

class ModesTest : public BaseTest
{
    Q_OBJECT

private Q_SLOTS:
    // Normal mode.
    void NormalMotionsTests();
    void NormalCommandsTests();
    void NormalControlTests();
    void NormalNotYetImplementedFeaturesTests();

    // Insert mode.
    void InsertTests();
    void InsertKeysTests();

    // Visual mode.
    void VisualMotionsTests();
    void VisualCommandsTests();
    void VisualExternalTests();

    // Command mode.
    void CommandTests();
    void CommandSedTests();
    void CommandDeleteTests();

    // Replace mode.
    void ReplaceCharacter();
    void ReplaceBasicTests();
    void ReplaceUndoTests();
    void ReplaceInsertFromLineTests();
};

#endif /* MODES_TEST_H */
