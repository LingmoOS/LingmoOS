/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_PHRASEANDITERATOR_H
#define BALOO_PHRASEANDITERATOR_H

#include "postingiterator.h"
#include "vectorpositioninfoiterator.h"

#include <QVector>

namespace Baloo {

class BALOO_ENGINE_EXPORT PhraseAndIterator : public PostingIterator
{
public:
    explicit PhraseAndIterator(const QVector<VectorPositionInfoIterator*>& iterators);
    ~PhraseAndIterator();

    quint64 next() override;
    quint64 docId() const override;
    quint64 skipTo(quint64 docId) override;

private:
    QVector<VectorPositionInfoIterator*> m_iterators;
    quint64 m_docId;

    BALOO_ENGINE_NO_EXPORT bool checkIfPositionsMatch();
};
}

#endif // BALOO_PHRASEANDITERATOR_H
