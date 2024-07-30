/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KACTIVITIES_STATS_TERMS_H
#define KACTIVITIES_STATS_TERMS_H

#include <initializer_list>

#include <QDate>
#include <QString>
#include <QStringList>

#include "plasmaactivitiesstats_export.h"

namespace KActivities
{
namespace Stats
{
/**
 * @namespace KActivities::Stats::Terms
 * Provides enums and strucss to use.for building queries with @c Query.
 */
namespace Terms
{
/**
 * Enumerator specifying the ordering in which the
 * results of the query should be listed
 */
enum PLASMAACTIVITIESSTATS_EXPORT Order {
    HighScoredFirst, ///< Resources with the highest scores first
    RecentlyUsedFirst, ///< Recently used resources first
    RecentlyCreatedFirst, ///< Recently created resources first
    OrderByUrl, ///< Order by uri, alphabetically
    OrderByTitle, ///< Order by uri, alphabetically
};

/**
 * Which resources should be returned
 */
enum PLASMAACTIVITIESSTATS_EXPORT Select {
    LinkedResources, ///< Resources linked to an activity, or globally
    UsedResources, ///< Resources that have been accessed
    AllResources, ///< Combined set of accessed and linked resources
};

/**
 * @struct KActivities::Stats::Terms::Limit terms.h <KActivities/Stats/Terms>
 *
 * How many items you need. The default is 50
 */
struct PLASMAACTIVITIESSTATS_EXPORT Limit {
    Limit(int value);
    int value;
};

/**
 * @struct KActivities::Stats::Terms::Offset terms.h <KActivities/Stats/Terms>
 *
 * How many items to skip?
 * This can be specified only if limit is also set to a finite value.
 */
struct PLASMAACTIVITIESSTATS_EXPORT Offset {
    Offset(int value);
    int value;
};

/**
 * @struct KActivities::Stats::Terms::Type terms.h <KActivities/Stats/Terms>
 *
 * Term to filter the resources according to their types
 */
struct PLASMAACTIVITIESSTATS_EXPORT Type {
    /**
     * Show resources of any type
     */
    static Type any();
    /**
     * Show non-directory resources
     */
    static Type files();
    /**
     * Show directory resources aka folders
     */
    static Type directories();

    inline Type(std::initializer_list<QString> types)
        : values(types)
    {
    }

    Type(QStringList types);
    Type(QString type);

    const QStringList values;
};

/**
 * @struct KActivities::Stats::Terms::Agent terms.h <KActivities/Stats/Terms>
 *
 * Term to filter the resources according the agent (application) which
 * accessed it
 */
struct PLASMAACTIVITIESSTATS_EXPORT Agent {
    /**
     * Show resources accessed/linked by any application
     */
    static Agent any();

    /**
     * Show resources not tied to a specific agent
     */
    static Agent global();

    /**
     * Show resources accessed/linked by the current application
     */
    static Agent current();

    inline Agent(std::initializer_list<QString> agents)
        : values(agents)
    {
    }

    Agent(QStringList agents);
    Agent(QString agent);

    const QStringList values;
};

/**
 * @struct KActivities::Stats::Terms::Activity terms.h <KActivities/Stats/Terms>
 *
 * Term to filter the resources according the activity in which they
 * were accessed
 */
struct PLASMAACTIVITIESSTATS_EXPORT Activity {
    /**
     * Show resources accessed in / linked to any activity
     */
    static Activity any();

    /**
     * Show resources linked to all activities
     */
    static Activity global();

    /**
     * Show resources linked to all activities
     */
    static Activity current();

    inline Activity(std::initializer_list<QString> activities)
        : values(activities)
    {
    }

    Activity(QStringList activities);
    Activity(QString activity);

    const QStringList values;
};

/**
 * @struct KActivities::Stats::Terms::Url terms.h <KActivities/Stats/Terms>
 *
 * Url filtering.
 */
struct PLASMAACTIVITIESSTATS_EXPORT Url {
    /**
     * Show only resources that start with the specified prefix
     */
    static Url startsWith(const QString &prefix);

    /**
     * Show resources that contain the specified infix
     */
    static Url contains(const QString &infix);

    /**
     * Show local files
     */
    static Url localFile();

    /**
     * Show local files, smb, fish, ftp and sftp
     */
    static Url file();

    inline Url(std::initializer_list<QString> urlPatterns)
        : values(urlPatterns)
    {
    }

    Url(QStringList urlPatterns);
    Url(QString urlPattern);

    const QStringList values;
};
/**
 * @since 6.0
 */
struct PLASMAACTIVITIESSTATS_EXPORT Title {
    /**
     * Show resources with title that contain the specified pattern
     */
    Title(const QString &titlePattern)
        : values({titlePattern})
    {
    }
    /// Default constructor
    Title()
    {
    }

    const QStringList values;
};

/**
 * @struct KActivities::Stats::Terms::Date terms.h <KActivities/Stats/Terms>
 *
 * On which start access date do you want to filter ?
 */
struct PLASMAACTIVITIESSTATS_EXPORT Date {
    Date(QDate value);
    Date(QDate start, QDate end);

    static Date today();
    static Date yesterday();
    static Date currentWeek();
    static Date previousWeek();
    static Date fromString(QString);

    QDate start, end;
};

} // namespace Terms

} // namespace Stats
} // namespace KActivities

PLASMAACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Stats::Terms::Order &order);

PLASMAACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Stats::Terms::Select &select);

PLASMAACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Stats::Terms::Type &type);

PLASMAACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Stats::Terms::Agent &agent);

PLASMAACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Stats::Terms::Activity &activity);

PLASMAACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Stats::Terms::Url &url);

PLASMAACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Stats::Terms::Limit &limit);

PLASMAACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Stats::Terms::Offset &offset);

PLASMAACTIVITIESSTATS_EXPORT
QDebug operator<<(QDebug dbg, const KActivities::Stats::Terms::Date &date);

#endif // KACTIVITIES_STATS_TERMS_H
