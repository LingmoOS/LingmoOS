/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "andpostingiterator.h"

using namespace Baloo;

AndPostingIterator::AndPostingIterator(const QVector<PostingIterator*>& iterators)
    : m_iterators(iterators)
    , m_docId(0)
{
    if (m_iterators.contains(nullptr)) {
        qDeleteAll(m_iterators);
        m_iterators.clear();
    }
}

AndPostingIterator::~AndPostingIterator()
{
    qDeleteAll(m_iterators);
}

quint64 AndPostingIterator::docId() const
{
    return m_docId;
}

quint64 AndPostingIterator::skipTo(quint64 id)
{
    if (m_iterators.isEmpty()) {
        m_docId = 0;
        return 0;
    }

    while (true) {
        quint64 lower_bound = id;
        for (PostingIterator* iter : std::as_const(m_iterators)) {
            lower_bound = iter->skipTo(lower_bound);

            if (lower_bound == 0) {
                m_docId = 0;
                return 0;
            }
        }

        if (lower_bound == id) {
            m_docId = lower_bound;
            return lower_bound;
        }
        id = lower_bound;
    }
}

quint64 AndPostingIterator::next()
{
    if (m_iterators.isEmpty()) {
        m_docId = 0;
        return 0;
    }

    m_docId = m_iterators[0]->next();
    m_docId = skipTo(m_docId);

    return m_docId;
}
