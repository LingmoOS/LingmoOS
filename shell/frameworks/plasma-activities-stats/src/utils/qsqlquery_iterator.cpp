/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qsqlquery_iterator.h"

NextValueIterator<QSqlQuery> begin(QSqlQuery &query)
{
    return NextValueIterator<QSqlQuery>(query);
}

NextValueIterator<QSqlQuery> end(QSqlQuery &query)
{
    return NextValueIterator<QSqlQuery>(query, NextValueIterator<QSqlQuery>::EndIterator);
}
