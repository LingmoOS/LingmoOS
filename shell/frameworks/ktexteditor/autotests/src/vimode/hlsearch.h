/*
   SPDX-FileCopyrightText: 2022 Martin Seher <martin.seher@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef HLSEARCH_TEST_H
#define HLSEARCH_TEST_H

#include "base.h"
#include <array>

namespace Kate
{
class TextRange;
}

class EmulatedCommandBarSetUpAndTearDown;

class HlSearchTest : public BaseTest
{
    Q_OBJECT

private Q_SLOTS:
    void highlightModeTests();

private:
    QList<Kate::TextRange *> rangesOnLine(int line);
    void setWindowSize();

    void TestHighlight_(int line, const char *file, const Kate::TextRange &r, std::array<int, 2> start, std::array<int, 2> end, const QColor &bg);

    std::unique_ptr<EmulatedCommandBarSetUpAndTearDown> m_emulatedCommandBarSetUpAndTearDown;
};

#endif /* HLSEARCH_TEST_H */
