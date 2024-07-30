/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_SEARCHSTORE_H
#define BALOO_SEARCHSTORE_H

#include <QString>
#include "result_p.h"

namespace Baloo {

class Term;
class Database;
class Transaction;
class PostingIterator;

class SearchStore
{
public:
    SearchStore();
    ~SearchStore();

    ResultList exec(const Term& term, uint offset, int limit, bool sortResults);

private:
    Database* m_db;

    PostingIterator* constructQuery(Transaction* tr, const Term& term);
};

}

#endif // BALOO_SEARCHSTORE_H
