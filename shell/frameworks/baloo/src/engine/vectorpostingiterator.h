/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_VECTORPOSTINGITERATOR_H
#define BALOO_VECTORPOSTINGITERATOR_H

#include "postingiterator.h"
#include <QVector>

namespace Baloo {

class BALOO_ENGINE_EXPORT VectorPostingIterator :  public PostingIterator
{
public:
    explicit VectorPostingIterator(const QVector<quint64>& values);

    quint64 docId() const override;
    quint64 next() override;

private:
    QVector<quint64> m_values;
    int m_pos;
};

}

#endif // BALOO_VECTORPOSTINGITERATOR_H
