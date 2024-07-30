/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_ANDPOSTINGITERATOR_H
#define BALOO_ANDPOSTINGITERATOR_H

#include "postingiterator.h"
#include <QVector>

namespace Baloo {

class BALOO_ENGINE_EXPORT AndPostingIterator : public PostingIterator
{
public:
    explicit AndPostingIterator(const QVector<PostingIterator*>& iterators);
    ~AndPostingIterator() override;

    quint64 next() override;
    quint64 docId() const override;
    quint64 skipTo(quint64 docId) override;

private:
    QVector<PostingIterator*> m_iterators;
    quint64 m_docId;
};

}

#endif // BALOO_ANDPOSTINGITERATOR_H
