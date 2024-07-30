/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_VECTORPOSITIONINFOITERATOR_H
#define BALOO_VECTORPOSITIONINFOITERATOR_H

#include "postingiterator.h"
#include "positiondb.h"

namespace Baloo {

class BALOO_ENGINE_EXPORT VectorPositionInfoIterator : public PostingIterator
{
public:
    explicit VectorPositionInfoIterator(const QVector<PositionInfo>& vector);

    quint64 docId() const override;
    quint64 next() override;
    QVector<uint> positions();

private:
    QVector<PositionInfo> m_vector;
    int m_pos;
};
}

#endif // BALOO_VECTORPOSITIONINFOITERATOR_H
