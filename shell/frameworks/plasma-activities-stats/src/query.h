/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KACTIVITIES_STATS_QUERY_H
#define KACTIVITIES_STATS_QUERY_H

#include <initializer_list>

#include <QString>
#include <QStringList>

#include "plasmaactivitiesstats_export.h"

#include "terms.h"

namespace KActivities
{
namespace Stats
{
class QueryPrivate;

/**
 * @class KActivities::Stats::Query query.h <KActivities/Stats/Query>
 *
 * The activities system tracks resources (documents, contacts, etc.)
 * that the user has used. It also allows linking resources to
 * specific activities (like bookmarks, favorites, etc.).
 *
 * The Query class specifies which resources to return -
 * the previously used ones, the linked ones, or to
 * combine these two.
 *
 * It allows filtering the results depending on the resource type,
 * the agent (application that reported the usage event,
 * see KActivities::ResourceInstance) and the activity the resource
 * has been used in, or linked to. It also allows filtering
 * on the URL of the resource.
 *
 * While it can be explicitly instantiated, a preferred approach
 * is to use the pipe syntax like this:
 *
 * @code
 * auto query = UsedResources
 *                 | RecentlyUsedFirst
 *                 | Agent::any()
 *                 | Type::any()
 *                 | Activity::current();
 * @endcode
 */
class PLASMAACTIVITIESSTATS_EXPORT Query
{
public:
    Query(Terms::Select selection = Terms::AllResources);

    // The damned rule of five minus one :)
    Query(Query &&source);
    Query(const Query &source);
    Query &operator=(Query source);
    ~Query();

    // Not all are born equal
    bool operator==(const Query &right) const;
    bool operator!=(const Query &right) const;

    Terms::Select selection() const;
    QStringList types() const;
    QStringList agents() const;
    QStringList activities() const;

    QStringList urlFilters() const;
    Terms::Order ordering() const;
    QStringList titleFilters() const;
    int offset() const;
    int limit() const;
    QDate dateStart() const;
    QDate dateEnd() const;

    void setSelection(Terms::Select selection);

    void addTypes(const QStringList &types);
    void addAgents(const QStringList &agents);
    void addActivities(const QStringList &activities);
    void addUrlFilters(const QStringList &urlFilters);
    void addTitleFilters(const QStringList &urlFilters);
    /**
     * @since 5.62
     */
    void setTypes(const Terms::Type &types);
    /**
     * @since 5.62
     */
    void setAgents(const Terms::Agent &agents);
    /**
     * @since 5.62
     */
    void setActivities(const Terms::Activity &activities);
    /**
     * @since 5.62
     */
    void setUrlFilters(const Terms::Url &urlFilters);
    void setOrdering(Terms::Order ordering);
    void setOffset(int offset);
    void setLimit(int limit);
    void setTitleFilters(const Terms::Title &title);
    /**
     * @since 5.62
     */
    void setDate(const Terms::Date &date);
    void setDateStart(QDate date);
    void setDateEnd(QDate date);

    void clearTypes();
    void clearAgents();
    void clearActivities();
    void clearUrlFilters();
    void clearTitleFilters();

    void removeTypes(const QStringList &types);
    void removeAgents(const QStringList &agents);
    void removeActivities(const QStringList &activities);
    void removeUrlFilters(const QStringList &urlFilters);

private:
    inline void addTerm(const Terms::Type &term)
    {
        addTypes(term.values);
    }

    inline void addTerm(const Terms::Agent &term)
    {
        addAgents(term.values);
    }

    inline void addTerm(const Terms::Activity &term)
    {
        addActivities(term.values);
    }

    inline void addTerm(const Terms::Url &term)
    {
        addUrlFilters(term.values);
    }

    inline void addTerm(Terms::Order ordering)
    {
        setOrdering(ordering);
    }

    inline void addTerm(Terms::Select selection)
    {
        setSelection(selection);
    }

    inline void addTerm(Terms::Limit limit)
    {
        setLimit(limit.value);
    }

    inline void addTerm(Terms::Offset offset)
    {
        setOffset(offset.value);
    }

    inline void addTerm(Terms::Date date)
    {
        setDateStart(date.start);
        setDateEnd(date.end);
    }
    inline void addTerm(Terms::Title title)
    {
        setTitleFilters(title);
    }

public:
    template<typename Term>
    friend inline Query operator|(const Query &query, Term &&term)
    {
        Query result(query);
        result.addTerm(term);
        return result;
    }

    template<typename Term>
    friend inline Query operator|(Query &&query, Term &&term)
    {
        query.addTerm(term);
        return std::move(query);
    }

private:
    QueryPrivate *d;
};

template<typename Term>
inline Query operator|(Terms::Select selection, Term &&term)
{
    return Query(selection) | term;
}

} // namespace Stats
} // namespace KActivities

PLASMAACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Stats::Query &query);

#endif // KACTIVITIES_STATS_QUERY_H
