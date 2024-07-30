/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "postingiterator.h"

using namespace Baloo;

PostingIterator::~PostingIterator()
{
}

quint64 PostingIterator::skipTo(quint64 id)
{
    quint64 currentId = docId();
    while (currentId < id) {
        currentId = next();
        if (!currentId) {
            break;
        }
    }
    return currentId;
}
