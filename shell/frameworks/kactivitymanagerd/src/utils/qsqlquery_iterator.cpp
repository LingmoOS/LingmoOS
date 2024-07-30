/*
 *   SPDX-FileCopyrightText: 2013-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
