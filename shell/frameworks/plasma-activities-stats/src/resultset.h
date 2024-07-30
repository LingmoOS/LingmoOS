/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KACTIVITIES_STATS_RESULTSET
#define KACTIVITIES_STATS_RESULTSET

#include "query.h"

#include <QDebug>

namespace KActivities
{
namespace Stats
{
class ResultSetPrivate;
class ResultSet_ResultPrivate;
class ResultSet_IteratorPrivate;

/**
 * @class KActivities::Stats::ResultSet resultset.h <KActivities/Stats/ResultSet>
 *
 * Class that can query the KActivities usage tracking mechanism
 * for resources.
 *
 * Note: It is important to note that you should not create
 * long-living instances of ResultSet. It might lock the database
 * and break proper updating mechanisms. If you want a list of results
 * that automatically get updated, use ResultModel.
 *
 * ResultSet is meant to be used when you just need to fetch a few results
 * like this:
 *
 * @code
 * auto results = ResultSet(AllResources | Agent("org.kde.kate"));
 * for (const auto &result: results) {
 *     // ...
 * }
 * @endcode
 */
class PLASMAACTIVITIESSTATS_EXPORT ResultSet
{
public:
    /**
     * Structure containing data of one of the results
     */
    class Result
    {
    public:
        Result();
        ~Result();

        Result(Result &&result);
        Result(const Result &result);
        Result &operator=(Result result);

        enum LinkStatus {
            NotLinked = 0,
            Unknown = 1,
            Linked = 2,
        };

        // TODO: KF6 rething the function names, and maybe their signature, perhaps leverage std::variant or std::optional to add semantics to the API
        QString resource() const; ///< String representation of resource (can represent an url or a path)
        QUrl url() const; ///< Url representation of a resource based on internal resource, readonly, @since 5.64
        QString title() const; ///< Title of the resource, or URL if title is not known
        QString mimetype() const; ///< Mimetype of the resource, or URL if title is not known
        double score() const; ///< The score calculated based on the usage statistics
        uint lastUpdate() const; ///< Timestamp of the last update
        uint firstUpdate() const; ///< Timestamp of the first update
        LinkStatus linkStatus() const; ///< Differentiates between linked and non-linked resources in mixed queries
        QStringList linkedActivities() const; ///< Contains the activities this resource is linked to for the queries that care about resource linking
        QString agent() const; /// Contains the initiating agent for this resource

        void setResource(QString resource);
        void setTitle(QString title);
        void setMimetype(QString mimetype);
        void setScore(double score);
        void setLastUpdate(uint lastUpdate);
        void setFirstUpdate(uint firstUpdate);
        void setLinkStatus(LinkStatus linkedStatus);
        void setLinkedActivities(QStringList activities);
        void setAgent(QString agent);

    private:
        ResultSet_ResultPrivate *d;
    };

    /**
     * ResultSet is a container. This notifies the generic algorithms
     * from STL and others of the contained type.
     */
    typedef Result value_type;

    /**
     * Creates the ResultSet from the specified query
     */
    ResultSet(Query query);

    ResultSet(ResultSet &&source);
    ResultSet(const ResultSet &source) = delete;
    ResultSet &operator=(ResultSet source) = delete;
    ~ResultSet();

    /**
     * @returns a result at the specified index
     * @param index of the result
     * @note You should use iterators instead
     */
    Result at(int index) const;

    // Iterators

    /**
     * An STL-style constant forward iterator for accessing the results in a ResultSet
     * TODO: Consider making this to be more than just forward iterator.
     *       Maybe even a random-access one.
     */
    class const_iterator
    {
    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef int difference_type;

        typedef const Result value_type;
        typedef const Result &reference;
        typedef const Result *pointer;

        const_iterator();
        const_iterator(const const_iterator &source);
        const_iterator &operator=(const const_iterator &source);

        ~const_iterator();

        bool isSourceValid() const;

        reference operator*() const;
        pointer operator->() const;

        // prefix
        const_iterator &operator++();
        // postfix
        const_iterator operator++(int);

        // prefix
        const_iterator &operator--();
        // postfix
        const_iterator operator--(int);

        const_iterator operator+(difference_type n) const;
        const_iterator &operator+=(difference_type n);

        const_iterator operator-(difference_type n) const;
        const_iterator &operator-=(difference_type n);

        reference operator[](difference_type n) const;

        PLASMAACTIVITIESSTATS_EXPORT friend bool operator==(const const_iterator &left, const const_iterator &right);
        PLASMAACTIVITIESSTATS_EXPORT friend bool operator!=(const const_iterator &left, const const_iterator &right);

        PLASMAACTIVITIESSTATS_EXPORT friend bool operator<(const const_iterator &left, const const_iterator &right);
        PLASMAACTIVITIESSTATS_EXPORT friend bool operator>(const const_iterator &left, const const_iterator &right);

        PLASMAACTIVITIESSTATS_EXPORT friend bool operator<=(const const_iterator &left, const const_iterator &right);
        PLASMAACTIVITIESSTATS_EXPORT friend bool operator>=(const const_iterator &left, const const_iterator &right);

        PLASMAACTIVITIESSTATS_EXPORT friend difference_type operator-(const const_iterator &left, const const_iterator &right);

    private:
        const_iterator(const ResultSet *resultSet, int currentRow);

        friend class ResultSet;

        ResultSet_IteratorPrivate *const d;
    };

    /**
     * @returns a constant iterator pointing to the start of the collection
     * (to the first item)
     * @note as usual in C++, the range of the collection is [begin, end)
     */
    const_iterator begin() const;
    /**
     * @returns a constant iterator pointing to the end of the collection
     * (after the last item)
     * @note as usual in C++, the range of the collection is [begin, end)
     */
    const_iterator end() const;

    /**
     * Alias for begin
     */
    inline const_iterator cbegin() const
    {
        return begin();
    }
    /**
     * Alias for end
     */
    inline const_iterator cend() const
    {
        return end();
    }

    /**
     * Alias for begin
     */
    inline const_iterator constBegin() const
    {
        return cbegin();
    }
    /**
     * Alias for end
     */
    inline const_iterator constEnd() const
    {
        return cend();
    }

private:
    friend class ResultSet_IteratorPrivate;
    ResultSetPrivate *d;
};

bool PLASMAACTIVITIESSTATS_EXPORT operator==(const ResultSet::const_iterator &left, const ResultSet::const_iterator &right);
bool PLASMAACTIVITIESSTATS_EXPORT operator!=(const ResultSet::const_iterator &left, const ResultSet::const_iterator &right);

bool PLASMAACTIVITIESSTATS_EXPORT operator<(const ResultSet::const_iterator &left, const ResultSet::const_iterator &right);
bool PLASMAACTIVITIESSTATS_EXPORT operator>(const ResultSet::const_iterator &left, const ResultSet::const_iterator &right);

bool PLASMAACTIVITIESSTATS_EXPORT operator<=(const ResultSet::const_iterator &left, const ResultSet::const_iterator &right);
bool PLASMAACTIVITIESSTATS_EXPORT operator>=(const ResultSet::const_iterator &left, const ResultSet::const_iterator &right);

ResultSet::const_iterator::difference_type PLASMAACTIVITIESSTATS_EXPORT operator-(const ResultSet::const_iterator &left, const ResultSet::const_iterator &right);

inline QDebug operator<<(QDebug out, const ResultSet::Result &result)
{
    return out << (result.linkStatus() == ResultSet::Result::Linked          ? "⊤"
                       : result.linkStatus() == ResultSet::Result::NotLinked ? "⊥"
                                                                             : "?")
               << result.score() << (result.title() != result.resource() ? result.title() : QString()) << result.lastUpdate()
               << QStringView(result.resource()).right(20);
}

} // namespace Stats
} // namespace KActivities

#endif // KACTIVITIES_STATS_RESULTSET
