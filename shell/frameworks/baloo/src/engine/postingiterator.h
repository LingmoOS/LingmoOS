/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_POSTINGITERATOR_H
#define BALOO_POSTINGITERATOR_H

#include <QVector>
#include "engine_export.h"

namespace Baloo {

/**
 * A PostingIterator is an abstract base class which can be used to iterate
 * over all the "postings" or "documents" which are particular term appears.
 *
 * All PostingIterators should iterate over a list of non-decreasing document ids.
 */
class BALOO_ENGINE_EXPORT PostingIterator
{
public:
    virtual ~PostingIterator();

    virtual quint64 next() = 0;
    virtual quint64 docId() const = 0;
    virtual quint64 skipTo(quint64 docId);
};
}

#endif // BALOO_POSTINGITERATOR_H
