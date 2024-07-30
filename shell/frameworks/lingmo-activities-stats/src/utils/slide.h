/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UTILS_SLIDE_H
#define UTILS_SLIDE_H

#include <algorithm>

// Inspired by C++ Seasoning talk by Sean Parent

namespace kamd
{
namespace utils
{
template<typename Iterator>
void slide(Iterator f, Iterator l, Iterator p)
{
    if (p < f) {
        std::rotate(p, f, l);
    } else if (l < p) {
        std::rotate(f, l, p);
    }
}

template<typename Iterator>
void slide_one(Iterator f, Iterator p)
{
    slide(f, f + 1, p);
}

template<typename Iterator>
void move_one(Iterator from, Iterator to)
{
    if (from < to) {
        while (from != to) {
            using std::swap;
            swap(*from, *(from + 1));
            ++from;
        }
    } else {
        while (from != to) {
            using std::swap;
            swap(*from, *(from - 1));
            --from;
        }
    }
}

} // namespace utils
} // namespace kamd

#endif // UTILS_SLIDE_H
