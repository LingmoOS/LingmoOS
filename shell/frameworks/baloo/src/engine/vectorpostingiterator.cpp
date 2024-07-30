/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "vectorpostingiterator.h"

using namespace Baloo;

VectorPostingIterator::VectorPostingIterator(const QVector<quint64>& values)
    : m_values(values)
    , m_pos(-1)
{
}

quint64 VectorPostingIterator::docId() const
{
    if (m_pos < 0 || m_pos >= m_values.size()) {
        return 0;
    }

    return m_values[m_pos];
}

quint64 VectorPostingIterator::next()
{
    if (m_pos >= m_values.size() - 1) {
        m_pos = m_values.size();
        m_values.clear();
        return 0;
    }

    m_pos++;
    return m_values[m_pos];
}
