/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "vectorpositioninfoiterator.h"
#include "positioninfo.h"

using namespace Baloo;

VectorPositionInfoIterator::VectorPositionInfoIterator(const QVector<PositionInfo>& vector)
    : m_vector(vector)
    , m_pos(-1)
{
}

quint64 VectorPositionInfoIterator::next()
{
    m_pos++;
    if (m_pos >= m_vector.size()) {
        m_pos = m_vector.size();
        m_vector.clear();
        return 0;
    }

    return m_vector[m_pos].docId;
}

quint64 VectorPositionInfoIterator::docId() const
{
    if (m_pos < 0 || m_pos >= m_vector.size()) {
        return 0;
    }

    return m_vector[m_pos].docId;
}

QVector<uint> VectorPositionInfoIterator::positions()
{
    if (m_pos < 0 || m_pos >= m_vector.size()) {
        return QVector<uint>();
    }

    return m_vector[m_pos].positions;
}
