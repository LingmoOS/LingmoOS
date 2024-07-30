/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "resultiterator.h"
#include "result_p.h"
#include "searchstore.h"
#include <limits>
#include <vector>
#include <utility>

using namespace Baloo;

class Baloo::ResultIteratorPrivate {
public:
    ResultIteratorPrivate() = default;

    ~ResultIteratorPrivate() {
    }

    ResultList results;
    std::size_t pos = std::numeric_limits<size_t>::max();
};

ResultIterator::ResultIterator(ResultList&& res)
    : d(new ResultIteratorPrivate)
{
    d->results = res;
}

ResultIterator::ResultIterator(ResultIterator &&rhs)
    : d(std::move(rhs.d))
{
}

ResultIterator::~ResultIterator() = default;

bool ResultIterator::next()
{
    d->pos++; // overflows to 0 on first use
    return d->pos < d->results.size();
}

QString ResultIterator::filePath() const
{
    Q_ASSERT(d->pos < d->results.size());
    return QString::fromUtf8(d->results.at(d->pos).filePath);
}

QByteArray ResultIterator::documentId() const
{
    Q_ASSERT(d->pos < d->results.size());
    return QByteArray::number(d->results.at(d->pos).documentId, 16);
}
