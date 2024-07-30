/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef VIEW_TEST_H
#define VIEW_TEST_H

#include "base.h"

namespace Kate
{
class TextRange;
}

class ViewTest : public BaseTest
{
    Q_OBJECT

private Q_SLOTS:
    void yankHighlightingTests();
    void visualLineUpDownTests();
    void ScrollViewTests();
    void clipboardTests_data();
    void clipboardTests();

private:
    QList<Kate::TextRange *> rangesOnFirstLine();
};

#endif /* VIEW_TEST_H */
