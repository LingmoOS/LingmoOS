/*
 *   SPDX-FileCopyrightText: 2013-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QSqlQuery>
#include <QVariant>

template<typename ResultSet>
class NextValueIterator
{
public:
    enum Type {
        NormalIterator,
        EndIterator,
    };

    NextValueIterator(ResultSet &query, Type type = NormalIterator)
        : m_query(query)
        , m_type(type)
    {
        if (type != EndIterator) {
            m_query.next();
        }
    }

    inline bool operator!=(const NextValueIterator<ResultSet> &other) const
    {
        Q_UNUSED(other);
        return m_query.isValid();
    }

    inline NextValueIterator<ResultSet> &operator*()
    {
        return *this;
    }

    inline QVariant operator[](int index) const
    {
        return m_query.value(index);
    }

    inline QVariant operator[](const QString &name) const
    {
        return m_query.value(name);
    }

    inline NextValueIterator<ResultSet> &operator++()
    {
        m_query.next();
        return *this;
    }

private:
    ResultSet &m_query;
    Type m_type;
};

NextValueIterator<QSqlQuery> begin(QSqlQuery &query);
NextValueIterator<QSqlQuery> end(QSqlQuery &query);
