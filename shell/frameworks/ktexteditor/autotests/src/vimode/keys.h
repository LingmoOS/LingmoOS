/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KEYS_TEST_H
#define KEYS_TEST_H

#include "base.h"

class KeysTest : public BaseTest
{
    Q_OBJECT

private Q_SLOTS:
    void MappingTests();
    void LeaderTests();
    void ParsingTests();
    void AltGr();
    void MacroTests();
    void MarkTests();
};

#endif /* KEYS_TEST_H */
