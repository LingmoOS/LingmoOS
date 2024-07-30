/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_ENGINEQUERY_H
#define BALOO_ENGINEQUERY_H

#include "engine_export.h"

#include <QByteArray>
#include <QVector>
#include <QDebug>

namespace Baloo {

class BALOO_ENGINE_EXPORT EngineQuery
{
public:
    enum Operation {
        Equal,
        StartsWith,
        Phrase,
    };

    EngineQuery();
    EngineQuery(const QByteArray& term, Operation op = Equal);
    EngineQuery(const QVector<EngineQuery> &subQueries);

    QByteArray term() const {
        return m_term;
    }

    Operation op() const {
        return m_op;
    }

    void setOp(const Operation& op) {
        m_op = op;
    }

    bool leaf() const {
        return !m_term.isEmpty();
    }

    bool empty() {
        return m_subQueries.isEmpty() && m_term.isEmpty();
    }

    QVector<EngineQuery> subQueries() const {
        return m_subQueries;
    }

    bool operator ==(const EngineQuery& q) const {
        return m_term == q.m_term && m_op == q.m_op && m_subQueries == q.m_subQueries;
    }
private:
    QByteArray m_term;
    Operation m_op;

    QVector<EngineQuery> m_subQueries;
};

inline QDebug operator<<(QDebug d, const Baloo::EngineQuery& q)
{
    QDebugStateSaver state(d);
    d.setAutoInsertSpaces(false);

    using Operation = Baloo::EngineQuery::Operation;
    if ((q.op() == Operation::Equal) || q.op() == Operation::StartsWith) {
        Q_ASSERT(q.subQueries().isEmpty());
        return d << q.term() << (q.op() == Operation::StartsWith ? ".." : "");
    }

    Q_ASSERT(q.op() == Operation::Phrase);
    d << "[PHRASE";
    for (auto &sq : q.subQueries()) {
        d << " " << sq;
    }
    return d << "]";
}

/**
 * Helper for QTest
 * \sa QTest::toString
 *
 * @since: 5.70
 */
inline char *toString(const EngineQuery& query)
{
    QString buffer;
    QDebug stream(&buffer);
    stream << query;
    return qstrdup(buffer.toUtf8().constData());
}

} // namespace Baloo
#endif
