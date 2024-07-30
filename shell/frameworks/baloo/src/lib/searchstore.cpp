/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "baloodebug.h"
#include "searchstore.h"
#include "global.h"

#include "database.h"
#include "term.h"
#include "transaction.h"
#include "enginequery.h"
#include "termgenerator.h"
#include "andpostingiterator.h"
#include "orpostingiterator.h"

#include <QDateTime>

#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/TypeInfo>
#include <KFileMetaData/Types>

#include <algorithm>
#include <array>
#include <tuple>

namespace Baloo {

namespace {
QPair<quint32, quint32> calculateTimeRange(const QDateTime& dt, Term::Comparator com)
{
    Q_ASSERT(dt.isValid());

    if (com == Term::Equal) {
        // Timestamps in DB are quint32 relative to Epoch (1970...2106)
        auto start = static_cast<quint32>(dt.date().startOfDay().toSecsSinceEpoch());
        auto end = static_cast<quint32>(dt.date().endOfDay().toSecsSinceEpoch());
        return {start, end};
    }

    quint32 timet = dt.toSecsSinceEpoch();
    if (com == Term::LessEqual) {
        return {0, timet};
    }
    if (com == Term::Less) {
        return {0, timet - 1};
    }
    if (com == Term::GreaterEqual) {
        return {timet, std::numeric_limits<quint32>::max()};
    }
    if (com == Term::Greater) {
        return {timet + 1, std::numeric_limits<quint32>::max()};
    }

    Q_ASSERT_X(0, __func__, "mtime query must contain a valid comparator");
    return {0, 0};
}

struct InternalProperty {
    const char* propertyName;
    const char* prefix;
    QMetaType::Type valueType;
};
constexpr std::array<InternalProperty, 7> internalProperties{{{"content", "", QMetaType::QString},
                                                              {"filename", "F", QMetaType::QString},
                                                              {"mimetype", "M", QMetaType::QString},
                                                              {"rating", "R", QMetaType::Int},
                                                              {"tag", "TAG-", QMetaType::QString},
                                                              {"tags", "TA", QMetaType::QString},
                                                              {"usercomment", "C", QMetaType::QString}}};

std::pair<QByteArray, QMetaType::Type> propertyInfo(const QByteArray &property)
{
    auto it = std::find_if(std::begin(internalProperties), std::end(internalProperties),
        [&property] (const InternalProperty& entry) { return property == entry.propertyName; });
    if (it != std::end(internalProperties)) {
        return { (*it).prefix, (*it).valueType };
    } else {
        KFileMetaData::PropertyInfo pi = KFileMetaData::PropertyInfo::fromName(QString::fromUtf8(property));
        if (pi.property() == KFileMetaData::Property::Empty) {
            return {QByteArray(), QMetaType::UnknownType};
        }
        int propPrefix = static_cast<int>(pi.property());
        return {QByteArray('X' + QByteArray::number(propPrefix) + '-'), pi.valueType()};
    }
}

EngineQuery constructEqualsQuery(const QByteArray& prefix, const QString& value)
{
    // We use the TermGenerator to normalize the words in the value and to
    // split it into other words. If we split the words, we then add them as a
    // phrase query.
    const QByteArrayList terms = TermGenerator::termList(value);

    QVector<EngineQuery> queries;
    queries.reserve(terms.size());
    for (const QByteArray& term : terms) {
        QByteArray arr = prefix + term;
        // FIXME - compatibility hack, to find truncated terms with old
        // DBs, remove on next DB bump
        if (arr.size() > 25) {
            queries << EngineQuery(arr.left(25), EngineQuery::StartsWith);
        } else {
            queries << EngineQuery(arr);
        }
    }

    if (queries.isEmpty()) {
        return EngineQuery();
    } else if (queries.size() == 1) {
        return queries.first();
    } else {
        return EngineQuery(queries);
    }
}

EngineQuery constructContainsQuery(const QByteArray& prefix, const QString& value)
{
    auto query = constructEqualsQuery(prefix, value);
    if (query.op() == EngineQuery::Equal) {
        if (query.term().size() >= 3) {
            query.setOp(EngineQuery::StartsWith);
	}
    }
    return query;
}

EngineQuery constructTypeQuery(const QString& value)
{
    Q_ASSERT(!value.isEmpty());

    KFileMetaData::TypeInfo ti = KFileMetaData::TypeInfo::fromName(value);
    if (ti == KFileMetaData::Type::Empty) {
        qCDebug(BALOO) << "Type" << value << "does not exist";
        return EngineQuery();
    }
    int num = static_cast<int>(ti.type());

    return EngineQuery('T' + QByteArray::number(num));
}
} // namespace

SearchStore::SearchStore()
    : m_db(nullptr)
{
    m_db = globalDatabaseInstance();
    if (!m_db->open(Database::ReadOnlyDatabase)) {
        m_db = nullptr;
    }
}

SearchStore::~SearchStore()
{
}

// Return the result with-in [offset, offset + limit)
ResultList SearchStore::exec(const Term& term, uint offset, int limit, bool sortResults)
{
    if (!m_db || !m_db->isOpen()) {
        return ResultList();
    }

    Transaction tr(m_db, Transaction::ReadOnly);
    std::unique_ptr<PostingIterator> it(constructQuery(&tr, term));
    if (!it) {
        return ResultList();
    }

    if (sortResults) {
        QVector<std::pair<quint64, quint32>> resultIds;
        while (it->next()) {
            quint64 id = it->docId();
            quint32 mtime = tr.documentTimeInfo(id).mTime;
            resultIds << std::pair<quint64, quint32>{id, mtime};

            Q_ASSERT(id > 0);
        }

        // Not enough results within range, no need to sort.
        if (offset >= static_cast<uint>(resultIds.size())) {
            return ResultList();
        }

        auto compFunc = [](const std::pair<quint64, quint32>& lhs,
                           const std::pair<quint64, quint32>& rhs) {
            return lhs.second > rhs.second;
        };

        std::sort(resultIds.begin(), resultIds.end(), compFunc);
        if (limit < 0) {
            limit = resultIds.size();
        }

        ResultList results;
        const uint end = qMin(static_cast<uint>(resultIds.size()), offset + static_cast<uint>(limit));
        results.reserve(end - offset);
        for (uint i = offset; i < end; i++) {
            const quint64 id = resultIds[i].first;
            Result res{tr.documentUrl(id), id};

            results.emplace_back(res);
        }

        return results;
    }
    else {
        ResultList results;
        uint ulimit = limit < 0 ? UINT_MAX : limit;

        while (offset && it->next()) {
            offset--;
        }

        while (ulimit && it->next()) {
            const quint64 id = it->docId();
            Q_ASSERT(id > 0);
            Result res{tr.documentUrl(id), id};
            Q_ASSERT(!res.filePath.isEmpty());

            results.emplace_back(res);

            ulimit--;
        }

        return results;
    }
}

PostingIterator* SearchStore::constructQuery(Transaction* tr, const Term& term)
{
    Q_ASSERT(tr);

    if (term.operation() == Term::And || term.operation() == Term::Or) {
        const QList<Term> subTerms = term.subTerms();
        QVector<PostingIterator*> vec;
        vec.reserve(subTerms.size());

        for (const Term& t : subTerms) {
            auto iterator = constructQuery(tr, t);
            // constructQuery returns a nullptr to signal an empty list
            if (iterator) {
                vec << iterator;
            } else if (term.operation() == Term::And) {
                return nullptr;
            }
        }

        if (vec.isEmpty()) {
            return nullptr;
        } else if (vec.size() == 1) {
            return vec.takeFirst();
        }

        if (term.operation() == Term::And) {
            return new AndPostingIterator(vec);
        } else {
            return new OrPostingIterator(vec);
        }
    }

    if (term.value().isNull()) {
        return nullptr;
    }
    Q_ASSERT(term.value().isValid());
    Q_ASSERT(term.comparator() != Term::Auto);
    Q_ASSERT(term.comparator() == Term::Contains ? term.value().typeId() == QMetaType::QString : true);

    const QVariant value = term.value();
    const QByteArray property = term.property().toLower().toUtf8();

    if (property == "type" || property == "kind") {
        EngineQuery q = constructTypeQuery(value.toString());
        return tr->postingIterator(q);
    }
    else if (property == "includefolder") {
        const QByteArray folder = value.toString().toUtf8();

        if (folder.isEmpty()) {
            return nullptr;
        }
        if (!folder.startsWith('/')) {
            return nullptr;
        }

        quint64 id = tr->documentId(folder);
        if (!id) {
            qCDebug(BALOO) << "Folder" << value.toString() << "not indexed";
            return nullptr;
        }

        return tr->docUrlIter(id);
    }
    else if (property == "modified" || property == "mtime") {
        if (value.typeId() == QMetaType::QByteArray) {
            // Used by Baloo::Query
            QByteArray ba = value.toByteArray();
            Q_ASSERT(ba.size() >= 4);

            int year = ba.mid(0, 4).toInt();
            int month = ba.mid(4, 2).toInt();
            int day = ba.mid(6, 2).toInt();

            Q_ASSERT(year);

            // uses 0 to represent whole month or whole year
            month = month >= 0 && month <= 12 ? month : 0;
            day = day >= 0 && day <= 31 ? day : 0;

            QDate startDate(year, month ? month : 1, day ? day : 1);
            QDate endDate(startDate);

            if (month == 0) {
                endDate.setDate(endDate.year(), 12, 31);
            } else if (day == 0) {
                endDate.setDate(endDate.year(), endDate.month(), endDate.daysInMonth());
            }

            return tr->mTimeRangeIter(startDate.startOfDay().toSecsSinceEpoch(), endDate.endOfDay().toSecsSinceEpoch());
        } else if (value.typeId() == QMetaType::QString) {
            const QDateTime dt = value.toDateTime();
            QPair<quint32, quint32> timerange = calculateTimeRange(dt, term.comparator());
            if ((timerange.first == 0) && (timerange.second == 0)) {
                return nullptr;
            }
            return tr->mTimeRangeIter(timerange.first, timerange.second);
        } else {
            Q_ASSERT_X(0, "SearchStore::constructQuery", "modified property must contain date/datetime values");
            return nullptr;
        }
    } else if (property == "tag") {
        if (term.comparator() == Term::Equal) {
            const QByteArray prefix = "TAG-";
            EngineQuery q = EngineQuery(prefix + value.toByteArray());
            return tr->postingIterator(q);
        } else if (term.comparator() == Term::Contains) {
            const QByteArray prefix = "TA";
            EngineQuery q = constructEqualsQuery(prefix, value.toString());
            return tr->postingIterator(q);
        } else {
            Q_ASSERT(0);
            return nullptr;
        }
    } else if (property == "") {
        Term cterm(QStringLiteral("content"), term.value(), term.comparator());
        Term fterm(QStringLiteral("filename"), term.value(), term.comparator());
        return constructQuery(tr, Term{cterm, Term::Operation::Or, fterm});
    }

    QByteArray prefix;
    QMetaType::Type valueType = QMetaType::QString;
    if (!property.isEmpty()) {
        std::tie(prefix, valueType) = propertyInfo(property);
        if (valueType == QMetaType::UnknownType) {
            return nullptr;
        }
    }

    auto com = term.comparator();
    if (com == Term::Contains && valueType == QMetaType::Int) {
        com = Term::Equal;
    }
    if (com == Term::Contains) {
        EngineQuery q = constructContainsQuery(prefix, value.toString());
        return tr->postingIterator(q);
    }

    if (com == Term::Equal) {
        EngineQuery q = constructEqualsQuery(prefix, value.toString());
        return tr->postingIterator(q);
    }

    PostingDB::Comparator pcom;
    if (com == Term::Greater || com == Term::GreaterEqual) {
        pcom = PostingDB::GreaterEqual;
    } else if (com == Term::Less || com == Term::LessEqual) {
        pcom = PostingDB::LessEqual;
    }

    // FIXME -- has to be kept in sync with the code from
    // Baloo::Result::add
    if (valueType == QMetaType::Int) {
        qlonglong intVal = value.toLongLong();

        if (term.comparator() == Term::Greater) {
            intVal++;
        } else if (term.comparator() == Term::Less) {
            intVal--;
        }

        return tr->postingCompIterator(prefix, intVal, pcom);

    } else if (valueType == QMetaType::Double) {
        double dVal = value.toDouble();
        return tr->postingCompIterator(prefix, dVal, pcom);

    } else if (valueType == QMetaType::QDateTime) {
        QDateTime dt = value.toDateTime();
        const QByteArray ba = dt.toString(Qt::ISODate).toUtf8();
        return tr->postingCompIterator(prefix, ba, pcom);

    } else {
        qCDebug(BALOO) << "Comparison must be with an integer";
    }

    return nullptr;
}

} // namespace Baloo
