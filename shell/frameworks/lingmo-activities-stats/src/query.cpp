/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "query.h"
#include "common/specialvalues.h"
#include <QDate>
#include <QDebug>

constexpr int s_defaultCacheSize = 50;

namespace KActivities
{
namespace Stats
{
namespace details
{
inline void validateTypes(QStringList &types)
{
    // Nothing at the moment
    Q_UNUSED(types);
}

inline void validateAgents(QStringList &agents)
{
    // Nothing at the moment
    Q_UNUSED(agents);
}

inline void validateActivities(QStringList &activities)
{
    // Nothing at the moment
    Q_UNUSED(activities);
}

inline void validateFilters(QStringList &filters)
{
    for (auto it = filters.begin(), end = filters.end(); it != end; ++it) {
        it->replace(QLatin1String("'"), QLatin1String(""));
    }
}
inline void validateUrlFilters(QStringList &urlFilters)
{
    validateFilters(urlFilters);
}

inline void validateTitleFilters(QStringList &titleFilters)
{
    validateFilters(titleFilters);
}

} // namespace details

class QueryPrivate
{
public:
    QueryPrivate()
        : ordering(Terms::HighScoredFirst)
        , limit(s_defaultCacheSize)
        , offset(0)
    {
    }

    Terms::Select selection;
    QStringList types;
    QStringList agents;
    QStringList activities;
    QStringList urlFilters;
    QStringList titleFilters;
    Terms::Order ordering;
    QDate start, end;
    int limit;
    int offset;
};

Query::Query(Terms::Select selection)
    : d(new QueryPrivate())
{
    d->selection = selection;
}

Query::Query(Query &&source)
    : d(nullptr)
{
    std::swap(d, source.d);
}

Query::Query(const Query &source)
    : d(new QueryPrivate(*source.d))
{
}

Query &Query::operator=(Query source)
{
    std::swap(d, source.d);
    return *this;
}

Query::~Query()
{
    delete d;
}

bool Query::operator==(const Query &right) const
{
    return selection() == right.selection() //
        && types() == right.types() //
        && agents() == right.agents() //
        && activities() == right.activities() //
        && selection() == right.selection() //
        && urlFilters() == right.urlFilters() //
        && dateStart() == right.dateStart() //
        && dateEnd() == right.dateEnd();
}

bool Query::operator!=(const Query &right) const
{
    return !(*this == right);
}

#define IMPLEMENT_QUERY_LIST_FIELD(WHAT, What, Term, Default)                                                                                                  \
    void Query::add##WHAT(const QStringList &What)                                                                                                             \
    {                                                                                                                                                          \
        d->What << What;                                                                                                                                       \
        details::validate##WHAT(d->What);                                                                                                                      \
    }                                                                                                                                                          \
                                                                                                                                                               \
    void Query::set##WHAT(const Terms::Term &What)                                                                                                             \
    {                                                                                                                                                          \
        d->What = What.values;                                                                                                                                 \
        details::validate##WHAT(d->What);                                                                                                                      \
    }                                                                                                                                                          \
                                                                                                                                                               \
    QStringList Query::What() const                                                                                                                            \
    {                                                                                                                                                          \
        return d->What.size() ? d->What : Default;                                                                                                             \
    }                                                                                                                                                          \
                                                                                                                                                               \
    void Query::clear##WHAT()                                                                                                                                  \
    {                                                                                                                                                          \
        d->What.clear();                                                                                                                                       \
    }

IMPLEMENT_QUERY_LIST_FIELD(Types, types, Type, QStringList(ANY_TYPE_TAG))
IMPLEMENT_QUERY_LIST_FIELD(Agents, agents, Agent, QStringList(CURRENT_AGENT_TAG))
IMPLEMENT_QUERY_LIST_FIELD(Activities, activities, Activity, QStringList(CURRENT_ACTIVITY_TAG))
IMPLEMENT_QUERY_LIST_FIELD(UrlFilters, urlFilters, Url, QStringList(QStringLiteral("*")))
IMPLEMENT_QUERY_LIST_FIELD(TitleFilters, titleFilters, Title, QStringList(QStringLiteral("*")))

#undef IMPLEMENT_QUERY_LIST_FIELD

void Query::setOrdering(Terms::Order ordering)
{
    d->ordering = ordering;
}

void Query::setSelection(Terms::Select selection)
{
    d->selection = selection;
}

void Query::setLimit(int limit)
{
    d->limit = limit;
}

void Query::setOffset(int offset)
{
    d->offset = offset;
}

void Query::setDate(const Terms::Date &date)
{
    d->start = date.start;
    d->end = date.end;
}

void Query::setDateStart(QDate start)
{
    d->start = start;
}

void Query::setDateEnd(QDate end)
{
    d->end = end;
}

Terms::Order Query::ordering() const
{
    return d->ordering;
}

Terms::Select Query::selection() const
{
    return d->selection;
}

int Query::limit() const
{
    return d->limit;
}

int Query::offset() const
{
    Q_ASSERT_X(d->limit > 0, "Query::offset", "Offset can only be specified if limit is set");
    return d->offset;
}

QDate Query::dateStart() const
{
    return d->start;
}

QDate Query::dateEnd() const
{
    return d->end;
}
} // namespace Stats
} // namespace KActivities

namespace KAStats = KActivities::Stats;

QDebug operator<<(QDebug dbg, const KAStats::Query &query)
{
    using namespace KAStats::Terms;

    // clang-format off
    dbg.nospace()
        << "Query { "
        << query.selection()
        << ", " << Type(query.types())
        << ", " << Agent(query.agents())
        << ", " << Activity(query.activities())
        << ", " << Url(query.urlFilters())
        << ", " << Date(query.dateStart(), query.dateEnd())
        << ", " << query.ordering()
        << ", Limit: " << query.limit()
        << " }";
    // clang-format on

    return dbg;
}
