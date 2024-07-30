/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_CORE_RESULT_ITERATOR_H
#define BALOO_CORE_RESULT_ITERATOR_H

#include "core_export.h"

#include <QString>

#include <memory>

namespace Baloo {

class ResultList;
class ResultIteratorPrivate;

/**
 * @class ResultIterator resultiterator.h <Baloo/ResultIterator>
 */
class BALOO_CORE_EXPORT ResultIterator
{
public:
    ResultIterator(ResultIterator &&rhs);
    ~ResultIterator();

    ResultIterator(const ResultIterator& rhs) = delete;
    ResultIterator &operator=(const ResultIterator& rhs) = delete;

    bool next();
    QString filePath() const;
    QByteArray documentId() const;

private:
    BALOO_CORE_NO_EXPORT explicit ResultIterator(ResultList&& res);
    std::unique_ptr<ResultIteratorPrivate> d;

    friend class Query;
};

}
#endif // BALOO_CORE_RESULT_ITERATOR_H
