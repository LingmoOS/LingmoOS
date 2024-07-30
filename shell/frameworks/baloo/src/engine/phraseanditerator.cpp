/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "phraseanditerator.h"
#include "positioninfo.h"

using namespace Baloo;

PhraseAndIterator::PhraseAndIterator(const QVector<VectorPositionInfoIterator*>& iterators)
    : m_iterators(iterators)
    , m_docId(0)
{
    if (m_iterators.contains(nullptr)) {
        qDeleteAll(m_iterators);
        m_iterators.clear();
    }
}

PhraseAndIterator::~PhraseAndIterator()
{
    qDeleteAll(m_iterators);
}

quint64 PhraseAndIterator::docId() const
{
    return m_docId;
}

bool PhraseAndIterator::checkIfPositionsMatch()
{
    using Offset = decltype(m_iterators[0]->positions().size());
    using Position = std::remove_reference<decltype(m_iterators[0]->positions()[0])>::type;

    std::vector<Offset> offsets;
    offsets.resize(m_iterators.size());

    const auto firstPositions = m_iterators[0]->positions();
    Position lower_bound = 0;

    while (offsets[0] < firstPositions.size()) {
        for (int i = 0; i < m_iterators.size(); i++) {
            const auto positions = m_iterators[i]->positions();
            Offset off = offsets[i];

            for (; off < positions.size(); ++off) {
                Position pos = positions[off];
                // Adjust the position. We have a match iff
                // term0 is at pos N, term1 at N+1, term2 at N+2 ...
                if (pos >= (lower_bound + i)) {
                    lower_bound = pos - i;
                    break;
                }
            }
            if (off >= positions.size()) {
                return false;
            }
            offsets[i] = off;
        }

        if (lower_bound == firstPositions[offsets[0]]) {
            // lower_bound has not changed, i.e. all offsets are aligned
            for (int i = 0; i < m_iterators.size(); i++) {
                auto positions = m_iterators[i]->positions();
            }
            return true;
        } else {
            offsets[0]++;
        }
    }
    return false;
}

quint64 PhraseAndIterator::skipTo(quint64 id)
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
            if (checkIfPositionsMatch()) {
                m_docId = lower_bound;
                return lower_bound;
            } else {
                lower_bound = m_iterators[0]->next();
            }
        }
        id = lower_bound;
    }
}

quint64 PhraseAndIterator::next()
{
    if (m_iterators.isEmpty()) {
        m_docId = 0;
        return 0;
    }

    m_docId = m_iterators[0]->next();
    m_docId = skipTo(m_docId);

    return m_docId;
}
