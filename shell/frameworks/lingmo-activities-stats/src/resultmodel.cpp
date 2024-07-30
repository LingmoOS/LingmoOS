/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

// Self
#include "resultmodel.h"

// Qt
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QPointer>

// STL
#include <functional>
#include <thread>

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

// Local
#include "cleaning.h"
#include "lingmo-activities-stats-logsettings.h"
#include "lingmoactivities/consumer.h"
#include "resultset.h"
#include "resultwatcher.h"
#include <common/database/Database.h>
#include <utils/member_matcher.h>
#include <utils/qsqlquery_iterator.h>
#include <utils/slide.h>

#include <common/specialvalues.h>

constexpr int s_defaultCacheSize = 50;

#define QDBG qCDebug(LINGMO_ACTIVITIES_STATS_LOG) << "LingmoActivitiesStats(" << (void *)this << ")"

namespace KActivities
{
namespace Stats
{
using Common::Database;

class ResultModelPrivate
{
public:
    ResultModelPrivate(Query query, const QString &clientId, ResultModel *parent)
        : cache(this, clientId, query.limit())
        , query(query)
        , watcher(query)
        , hasMore(true)
        , database(Database::instance(Database::ResourcesDatabase, Database::ReadOnly))
        , q(parent)
    {
        s_privates << this;
    }

    ~ResultModelPrivate()
    {
        s_privates.removeAll(this);
    }

    enum Fetch {
        FetchReset, // Remove old data and reload
        FetchReload, // Update all data
        FetchMore, // Load more data if there is any
    };

    class Cache
    { //_
    public:
        typedef QList<ResultSet::Result> Items;

        Cache(ResultModelPrivate *d, const QString &clientId, int limit)
            : d(d)
            , m_countLimit(limit)
            , m_clientId(clientId)
        {
            if (!m_clientId.isEmpty()) {
                m_configFile = KSharedConfig::openConfig(QStringLiteral("kactivitymanagerd-statsrc"));
            }
        }

        ~Cache()
        {
        }

        inline int size() const
        {
            return m_items.size();
        }

        inline void setLinkedResultPosition(const QString &resourcePath, int position)
        {
            if (!m_orderingConfig.isValid()) {
                qCWarning(LINGMO_ACTIVITIES_STATS_LOG) << "We can not reorder the results, no clientId was specified";
                return;
            }

            // Preconditions:
            //  - cache is ordered properly, first on the user's desired order,
            //    then on the query specified order
            //  - the resource that needs to be moved is a linked resource, not
            //    one that comes from the stats (there are overly many
            //    corner-cases that need to be covered in order to support
            //    reordering of the statistics-based resources)
            //  - the new position for the resource is not outside of the cache

            auto resourcePosition = find(resourcePath);

            if (resourcePosition) {
                if (resourcePosition.index == position) {
                    return;
                }
                if (resourcePosition.iterator->linkStatus() == ResultSet::Result::NotLinked) {
                    return;
                }
            }

            // Lets make a list of linked items - we can only reorder them,
            // not others
            QStringList linkedItems;

            for (const ResultSet::Result &item : std::as_const(m_items)) {
                if (item.linkStatus() == ResultSet::Result::NotLinked) {
                    break;
                }
                linkedItems << item.resource();
            }

            // We have two options:
            //  - we are planning to add an item to the desired position,
            //    but the item is not yet in the model
            //  - we want to move an existing item
            if (!resourcePosition || resourcePosition.iterator->linkStatus() == ResultSet::Result::NotLinked) {
                linkedItems.insert(position, resourcePath);

                m_fixedOrderedItems = linkedItems;

            } else {
                // We can not accept the new position to be outside
                // of the linked items area
                if (position >= linkedItems.size()) {
                    position = linkedItems.size() - 1;
                }

                Q_ASSERT(resourcePosition.index == linkedItems.indexOf(resourcePath));
                auto oldPosition = linkedItems.indexOf(resourcePath);

                kamd::utils::move_one(linkedItems.begin() + oldPosition, linkedItems.begin() + position);

                // When we change this, the cache is not valid anymore,
                // destinationFor will fail and we can not use it
                m_fixedOrderedItems = linkedItems;

                // We are prepared to reorder the cache
                d->repositionResult(resourcePosition, d->destinationFor(*resourcePosition));
            }

            m_orderingConfig.writeEntry("kactivitiesLinkedItemsOrder", m_fixedOrderedItems);
            m_orderingConfig.sync();

            // We need to notify others to reload
            for (const auto &other : std::as_const(s_privates)) {
                if (other != d && other->cache.m_clientId == m_clientId) {
                    other->fetch(FetchReset);
                }
            }
        }

        inline void debug() const
        {
            for (const auto &item : m_items) {
                qCDebug(LINGMO_ACTIVITIES_STATS_LOG) << "Item: " << item;
            }
        }

        void loadOrderingConfig(const QString &activityTag)
        {
            if (!m_configFile) {
                qCDebug(LINGMO_ACTIVITIES_STATS_LOG) << "Nothing to load - the client id is empty";
                return;
            }

            m_orderingConfig = KConfigGroup(m_configFile, QStringLiteral("ResultModel-OrderingFor-") + m_clientId + activityTag);

            if (m_orderingConfig.hasKey("kactivitiesLinkedItemsOrder")) {
                // If we have the ordering defined, use it
                m_fixedOrderedItems = m_orderingConfig.readEntry("kactivitiesLinkedItemsOrder", QStringList());
            } else {
                // Otherwise, copy the order from the previous activity to this one
                m_orderingConfig.writeEntry("kactivitiesLinkedItemsOrder", m_fixedOrderedItems);
                m_orderingConfig.sync();
            }
        }

    private:
        ResultModelPrivate *const d;

        QList<ResultSet::Result> m_items;
        int m_countLimit;

        QString m_clientId;
        KSharedConfig::Ptr m_configFile;
        KConfigGroup m_orderingConfig;
        QStringList m_fixedOrderedItems;

        friend QDebug operator<<(QDebug out, const Cache &cache)
        {
            for (const auto &item : cache.m_items) {
                out << "Cache item: " << item << "\n";
            }

            return out;
        }

    public:
        inline const QStringList &fixedOrderedItems() const
        {
            return m_fixedOrderedItems;
        }

        //_ Fancy iterator, find, lowerBound
        struct FindCacheResult {
            Cache *const cache;
            Items::iterator iterator;
            int index;

            FindCacheResult(Cache *cache, Items::iterator iterator)
                : cache(cache)
                , iterator(iterator)
                , index(std::distance(cache->m_items.begin(), iterator))
            {
            }

            operator bool() const
            {
                return iterator != cache->m_items.end();
            }

            ResultSet::Result &operator*() const
            {
                return *iterator;
            }

            ResultSet::Result *operator->() const
            {
                return &(*iterator);
            }
        };

        inline FindCacheResult find(const QString &resource)
        {
            using namespace kamd::utils::member_matcher;

            // Non-const iterator because the result is constructed from it
            return FindCacheResult(this, std::find_if(m_items.begin(), m_items.end(), member(&ResultSet::Result::resource) == resource));
        }

        template<typename Predicate>
        inline FindCacheResult lowerBoundWithSkippedResource(Predicate &&lessThanPredicate)
        {
            using namespace kamd::utils::member_matcher;
            const int count = std::count_if(m_items.cbegin(), m_items.cend(), [&](const ResultSet::Result &result) {
                return lessThanPredicate(result, _);
            });

            return FindCacheResult(this, m_items.begin() + count);

            // using namespace kamd::utils::member_matcher;
            //
            // const auto position =
            //     std::lower_bound(m_items.begin(), m_items.end(),
            //                      _, std::forward<Predicate>(lessThanPredicate));
            //
            // // We seem to have found the position for the item.
            // // The problem is that we might have found the same position
            // // we were previously at. Since this function is usually used
            // // to reposition the result, we might not be in a completely
            // // sorted collection, so the next item(s) could be less than us.
            // // We could do this with count_if, but it would be slower
            //
            // if (position >= m_items.cend() - 1) {
            //     return FindCacheResult(this, position);
            //
            // } else if (lessThanPredicate(_, *(position + 1))) {
            //     return FindCacheResult(this, position);
            //
            // } else {
            //     return FindCacheResult(
            //         this, std::lower_bound(position + 1, m_items.end(),
            //                                _, std::forward<Predicate>(lessThanPredicate)));
            // }
        }
        //^

        inline void insertAt(const FindCacheResult &at, const ResultSet::Result &result)
        {
            m_items.insert(at.iterator, result);
        }

        inline void removeAt(const FindCacheResult &at)
        {
            m_items.removeAt(at.index);
        }

        inline const ResultSet::Result &operator[](int index) const
        {
            return m_items[index];
        }

        inline void clear()
        {
            if (m_items.size() == 0) {
                return;
            }

            d->q->beginRemoveRows(QModelIndex(), 0, m_items.size() - 1);
            m_items.clear();
            d->q->endRemoveRows();
        }

        //  Algorithm to calculate the edit operations to allow
        //_ replaceing items without model reset
        inline void replace(const Items &newItems, int from = 0)
        {
            using namespace kamd::utils::member_matcher;

#if 0
            QDBG << "======";
            QDBG << "Old items {";
            for (const auto &item : m_items) {
                QDBG << item;
            }
            QDBG << "}";

            QDBG << "New items to be added at " << from << " {";
            for (const auto &item : newItems) {
                QDBG << item;
            }
            QDBG << "}";
#endif

            // Based on 'The string to string correction problem
            // with block moves' paper by Walter F. Tichy
            //
            // In essence, it goes like this:
            //
            // Take the first element from the new list, and try to find
            // it in the old one. If you can not find it, it is a new item
            // item - send the 'inserted' event.
            // If you did find it, test whether the following items also
            // match. This detects blocks of items that have moved.
            //
            // In this example, we find 'b', and then detect the rest of the
            // moved block 'b' 'c' 'd'
            //
            // Old items:  a[b c d]e f g
            //               ^
            //              /
            // New items: [b c d]a f g
            //
            // After processing one block, just repeat until the end of the
            // new list is reached.
            //
            // Then remove all remaining elements from the old list.
            //
            // The main addition here compared to the original papers is that
            // our 'strings' can not hold two instances of the same element,
            // and that we support updating from arbitrary position.

            auto newBlockStart = newItems.cbegin();

            // How many items should we add?
            // This should remove the need for post-replace-trimming
            // in the case where somebody called this with too much new items.
            const int maxToReplace = m_countLimit - from;

            if (maxToReplace <= 0) {
                return;
            }

            const auto newItemsEnd = newItems.size() <= maxToReplace ? newItems.cend() : newItems.cbegin() + maxToReplace;

            // Finding the blocks until we reach the end of the newItems list
            //
            // from = 4
            // Old items: X Y Z U a b c d e f g
            //                      ^ oldBlockStart points to the first element
            //                        of the currently processed block in the old list
            //
            // New items: _ _ _ _ b c d a f g
            //                    ^ newBlockStartIndex is the index of the first
            //                      element of the block that is currently being
            //                      processed (with 'from' offset)

            while (newBlockStart != newItemsEnd) {
                const int newBlockStartIndex = from + std::distance(newItems.cbegin(), newBlockStart);

                const auto oldBlockStart =
                    std::find_if(m_items.begin() + from, m_items.end(), member(&ResultSet::Result::resource) == newBlockStart->resource());

                if (oldBlockStart == m_items.end()) {
                    // This item was not found in the old cache, so we are
                    // inserting a new item at the same position it had in
                    // the newItems array
                    d->q->beginInsertRows(QModelIndex(), newBlockStartIndex, newBlockStartIndex);

                    m_items.insert(newBlockStartIndex, *newBlockStart);
                    d->q->endInsertRows();

                    // This block contained only one item, move on to find
                    // the next block - it starts from the next item
                    ++newBlockStart;

                } else {
                    // We are searching for a block of matching items.
                    // This is a reimplementation of std::mismatch that
                    // accepts two complete ranges that is available only
                    // since C++14, so we can not use it.
                    auto newBlockEnd = newBlockStart;
                    auto oldBlockEnd = oldBlockStart;

                    while (newBlockEnd != newItemsEnd && oldBlockEnd != m_items.end() && newBlockEnd->resource() == oldBlockEnd->resource()) {
                        ++newBlockEnd;
                        ++oldBlockEnd;
                    }

                    // We have found matching blocks
                    // [newBlockStart, newBlockEnd) and [oldBlockStart, newBlockEnd)
                    const int oldBlockStartIndex = std::distance(m_items.begin() + from, oldBlockStart);

                    const int blockSize = std::distance(oldBlockStart, oldBlockEnd);

                    if (oldBlockStartIndex != newBlockStartIndex) {
                        // If these blocks do not have the same start,
                        // we need to send the move event.

                        // Note: If there is a crash here, it means we
                        // are getting a bad query which has duplicate
                        // results

                        d->q->beginMoveRows(QModelIndex(), oldBlockStartIndex, oldBlockStartIndex + blockSize - 1, QModelIndex(), newBlockStartIndex);

                        // Moving the items from the old location to the new one
                        kamd::utils::slide(oldBlockStart, oldBlockEnd, m_items.begin() + newBlockStartIndex);

                        d->q->endMoveRows();
                    }

                    // Skip all the items in this block, and continue with
                    // the search
                    newBlockStart = newBlockEnd;
                }
            }

            // We have avoided the need for trimming for the most part,
            // but if the newItems list was shorter than needed, we still
            // need to trim the rest.
            trim(from + newItems.size());

            // Check whether we got an item representing a non-existent file,
            // if so, schedule its removal from the database
            // we want to do this async so that we don't block
            QPointer model{d->q};
            std::thread([model, newItems] {
                QList<QString> missingResources;
                for (const auto &item : newItems) {
                    // QFile.exists() can be incredibly slow (eg. if resource is on remote filesystem)
                    if (item.resource().startsWith(QLatin1Char('/')) && !QFile(item.resource()).exists()) {
                        missingResources << item.resource();
                    }
                }

                if (missingResources.empty()) {
                    return;
                }

                QTimer::singleShot(0, model, [missingResources, model] {
                    if (model) {
                        model->forgetResources(missingResources);
                    }
                });
            }).detach();
        }
        //^

        inline void trim()
        {
            trim(m_countLimit);
        }

        inline void trim(int limit)
        {
            if (m_items.size() <= limit) {
                return;
            }

            // Example:
            //   limit is 5,
            //   current cache (0, 1, 2, 3, 4, 5, 6, 7), size = 8
            // We need to delete from 5 to 7

            d->q->beginRemoveRows(QModelIndex(), limit, m_items.size() - 1);
            m_items.erase(m_items.begin() + limit, m_items.end());
            d->q->endRemoveRows();
        }

    } cache; //^

    struct FixedItemsLessThan {
        //_ Compartor that orders the linked items by user-specified order
        typedef kamd::utils::member_matcher::placeholder placeholder;

        enum Ordering {
            PartialOrdering,
            FullOrdering,
        };

        FixedItemsLessThan(Ordering ordering, const Cache &cache, const QString &matchResource = QString())
            : cache(cache)
            , matchResource(matchResource)
            , ordering(ordering)
        {
        }

        bool lessThan(const QString &leftResource, const QString &rightResource) const
        {
            const auto fixedOrderedItems = cache.fixedOrderedItems();

            const auto indexLeft = fixedOrderedItems.indexOf(leftResource);
            const auto indexRight = fixedOrderedItems.indexOf(rightResource);

            const bool hasLeft = indexLeft != -1;
            const bool hasRight = indexRight != -1;

            return (hasLeft && !hasRight) ? true
                : (!hasLeft && hasRight)  ? false
                : (hasLeft && hasRight)   ? indexLeft < indexRight
                                          : (ordering == PartialOrdering ? false : leftResource < rightResource);
        }

        template<typename T>
        bool operator()(const T &left, placeholder) const
        {
            return lessThan(left.resource(), matchResource);
        }

        template<typename T>
        bool operator()(placeholder, const T &right) const
        {
            return lessThan(matchResource, right.resource());
        }

        template<typename T, typename V>
        bool operator()(const T &left, const V &right) const
        {
            return lessThan(left.resource(), right.resource());
        }

        const Cache &cache;
        const QString matchResource;
        Ordering ordering;
        //^
    };

    inline Cache::FindCacheResult destinationFor(const ResultSet::Result &result)
    {
        using namespace kamd::utils::member_matcher;
        using namespace Terms;

        const auto resource = result.resource();
        const auto score = result.score();
        const auto firstUpdate = result.firstUpdate();
        const auto lastUpdate = result.lastUpdate();
        const auto linkStatus = result.linkStatus();

#define FIXED_ITEMS_LESS_THAN FixedItemsLessThan(FixedItemsLessThan::PartialOrdering, cache, resource)
#define ORDER_BY(Field) member(&ResultSet::Result::Field) > Field
#define ORDER_BY_FULL(Field)                                                                                                                                   \
    (query.selection() == Terms::AllResources                                                                                                                  \
         ? cache.lowerBoundWithSkippedResource(FIXED_ITEMS_LESS_THAN && ORDER_BY(linkStatus) && ORDER_BY(Field) && ORDER_BY(resource))                         \
         : cache.lowerBoundWithSkippedResource(FIXED_ITEMS_LESS_THAN && ORDER_BY(Field) && ORDER_BY(resource)))

        const auto destination = query.ordering() == HighScoredFirst ? ORDER_BY_FULL(score)
            : query.ordering() == RecentlyUsedFirst                  ? ORDER_BY_FULL(lastUpdate)
            : query.ordering() == RecentlyCreatedFirst               ? ORDER_BY_FULL(firstUpdate)
                                                                     :
                                                       /* otherwise */ ORDER_BY_FULL(resource);
#undef ORDER_BY
#undef ORDER_BY_FULL
#undef FIXED_ITEMS_LESS_THAN

        return destination;
    }

    inline void removeResult(const Cache::FindCacheResult &result)
    {
        q->beginRemoveRows(QModelIndex(), result.index, result.index);
        cache.removeAt(result);
        q->endRemoveRows();

        if (query.selection() != Terms::LinkedResources) {
            fetch(cache.size(), 1);
        }
    }

    inline void repositionResult(const Cache::FindCacheResult &result, const Cache::FindCacheResult &destination)
    {
        // We already have the resource in the cache
        // So, it is the time for a reshuffle
        const int oldPosition = result.index;
        int position = destination.index;

        Q_EMIT q->dataChanged(q->index(oldPosition), q->index(oldPosition));

        if (oldPosition == position) {
            return;
        }

        if (position > oldPosition) {
            position++;
        }

        bool moving = q->beginMoveRows(QModelIndex(), oldPosition, oldPosition, QModelIndex(), position);

        kamd::utils::move_one(result.iterator, destination.iterator);

        if (moving) {
            q->endMoveRows();
        }
    }

    void reload()
    {
        fetch(FetchReload);
    }

    void init()
    {
        using namespace std::placeholders;

        QObject::connect(&watcher, &ResultWatcher::resultScoreUpdated, q, std::bind(&ResultModelPrivate::onResultScoreUpdated, this, _1, _2, _3, _4));
        QObject::connect(&watcher, &ResultWatcher::resultRemoved, q, std::bind(&ResultModelPrivate::onResultRemoved, this, _1));
        QObject::connect(&watcher, &ResultWatcher::resultLinked, q, std::bind(&ResultModelPrivate::onResultLinked, this, _1));
        QObject::connect(&watcher, &ResultWatcher::resultUnlinked, q, std::bind(&ResultModelPrivate::onResultUnlinked, this, _1));

        QObject::connect(&watcher, &ResultWatcher::resourceTitleChanged, q, std::bind(&ResultModelPrivate::onResourceTitleChanged, this, _1, _2));
        QObject::connect(&watcher, &ResultWatcher::resourceMimetypeChanged, q, std::bind(&ResultModelPrivate::onResourceMimetypeChanged, this, _1, _2));

        QObject::connect(&watcher, &ResultWatcher::resultsInvalidated, q, std::bind(&ResultModelPrivate::reload, this));

        if (query.activities().contains(CURRENT_ACTIVITY_TAG)) {
            QObject::connect(&activities,
                             &KActivities::Consumer::currentActivityChanged,
                             q,
                             std::bind(&ResultModelPrivate::onCurrentActivityChanged, this, _1));
        }

        fetch(FetchReset);
    }

    void fetch(const int from, int count)
    {
        using namespace Terms;

        if (from + count > query.limit()) {
            count = query.limit() - from;
        }

        if (count <= 0) {
            return;
        }

        // In order to see whether there are more results, we need to pass
        // the count increased by one
        ResultSet results(query | Offset(from) | Limit(count + 1));

        auto it = results.begin();

        Cache::Items newItems;

        while (count-- > 0 && it != results.end()) {
            newItems << *it;
            ++it;
        }

        hasMore = (it != results.end());

        // We need to sort the new items for the linked resources
        // user-defined reordering. This needs only to be a partial sort,
        // the main sorting is done by sqlite
        if (query.selection() != Terms::UsedResources) {
            std::stable_sort(newItems.begin(), newItems.end(), FixedItemsLessThan(FixedItemsLessThan::PartialOrdering, cache));
        }

        cache.replace(newItems, from);
    }

    void fetch(Fetch mode)
    {
        if (mode == FetchReset) {
            // Removing the previously cached data
            // and loading all from scratch
            cache.clear();

            const QString activityTag = query.activities().contains(CURRENT_ACTIVITY_TAG) //
                ? (QStringLiteral("-ForActivity-") + activities.currentActivity())
                : QStringLiteral("-ForAllActivities");

            cache.loadOrderingConfig(activityTag);

            // If the user has requested less than 50 entries, only fetch those. If more, they should be fetched in subsequent batches
            fetch(0, qMin(s_defaultCacheSize, query.limit()));

        } else if (mode == FetchReload) {
            if (cache.size() > s_defaultCacheSize) {
                // If the cache is big, we are pretending
                // we were asked to reset the model
                fetch(FetchReset);

            } else {
                // We are only updating the currently
                // cached items, nothing more
                fetch(0, cache.size());
            }

        } else { // FetchMore
            // Load a new batch of data
            fetch(cache.size(), s_defaultCacheSize);
        }
    }

    void onResultScoreUpdated(const QString &resource, double score, uint lastUpdate, uint firstUpdate)
    {
        QDBG << "ResultModelPrivate::onResultScoreUpdated "
             << "result added:" << resource << "score:" << score << "last:" << lastUpdate << "first:" << firstUpdate;

        // This can also be called when the resource score
        // has been updated, so we need to check whether
        // we already have it in the cache
        const auto result = cache.find(resource);

        ResultSet::Result::LinkStatus linkStatus = result ? result->linkStatus()
            : query.selection() != Terms::UsedResources   ? ResultSet::Result::Unknown
            : query.selection() != Terms::LinkedResources ? ResultSet::Result::Linked
                                                          : ResultSet::Result::NotLinked;

        if (result) {
            // We are only updating a result we already had,
            // lets fill out the data and send the update signal.
            // Move it if necessary.

            auto &item = *result.iterator;

            item.setScore(score);
            item.setLinkStatus(linkStatus);
            item.setLastUpdate(lastUpdate);
            item.setFirstUpdate(firstUpdate);

            repositionResult(result, destinationFor(item));

        } else {
            // We do not have the resource in the cache,
            // lets fill out the data and insert it
            // at the desired position

            ResultSet::Result result;
            result.setResource(resource);

            result.setTitle(QStringLiteral(" "));
            result.setMimetype(QStringLiteral(" "));
            fillTitleAndMimetype(result);

            result.setScore(score);
            result.setLinkStatus(linkStatus);
            result.setLastUpdate(lastUpdate);
            result.setFirstUpdate(firstUpdate);

            const auto destination = destinationFor(result);

            q->beginInsertRows(QModelIndex(), destination.index, destination.index);

            cache.insertAt(destination, result);

            q->endInsertRows();

            cache.trim();
        }
    }

    void onResultRemoved(const QString &resource)
    {
        const auto result = cache.find(resource);

        if (!result) {
            return;
        }

        if (query.selection() == Terms::UsedResources || result->linkStatus() != ResultSet::Result::Linked) {
            removeResult(result);
        }
    }

    void onResultLinked(const QString &resource)
    {
        if (query.selection() != Terms::UsedResources) {
            onResultScoreUpdated(resource, 0, 0, 0);
        }
    }

    void onResultUnlinked(const QString &resource)
    {
        const auto result = cache.find(resource);

        if (!result) {
            return;
        }

        if (query.selection() == Terms::LinkedResources) {
            removeResult(result);

        } else if (query.selection() == Terms::AllResources) {
            // When the result is unlinked, it might go away or not
            // depending on its previous usage
            reload();
        }
    }

    Query query;
    ResultWatcher watcher;
    bool hasMore;

    KActivities::Consumer activities;
    Common::Database::Ptr database;

    //_ Title and mimetype functions
    void fillTitleAndMimetype(ResultSet::Result &result)
    {
        if (!database) {
            return;
        }

        auto query = database->execQuery(QStringLiteral("SELECT "
                                                        "title, mimetype "
                                                        "FROM "
                                                        "ResourceInfo "
                                                        "WHERE "
                                                        "targettedResource = '")
                                         + result.resource() + QStringLiteral("'"));

        // Only one item at most
        for (const auto &item : query) {
            result.setTitle(item[QStringLiteral("title")].toString());
            result.setMimetype(item[QStringLiteral("mimetype")].toString());
        }
    }

    void onResourceTitleChanged(const QString &resource, const QString &title)
    {
        const auto result = cache.find(resource);

        if (!result) {
            return;
        }

        result->setTitle(title);

        Q_EMIT q->dataChanged(q->index(result.index), q->index(result.index));
    }

    void onResourceMimetypeChanged(const QString &resource, const QString &mimetype)
    {
        // TODO: This can add or remove items from the model

        const auto result = cache.find(resource);

        if (!result) {
            return;
        }

        result->setMimetype(mimetype);

        Q_EMIT q->dataChanged(q->index(result.index), q->index(result.index));
    }
    //^

    void onCurrentActivityChanged(const QString &activity)
    {
        Q_UNUSED(activity);
        // If the current activity has changed, and
        // the query lists items for the ':current' one,
        // reset the model (not a simple refresh this time)
        if (query.activities().contains(CURRENT_ACTIVITY_TAG)) {
            fetch(FetchReset);
        }
    }

private:
    ResultModel *const q;
    static QList<ResultModelPrivate *> s_privates;
};

QList<ResultModelPrivate *> ResultModelPrivate::s_privates;

ResultModel::ResultModel(Query query, QObject *parent)
    : QAbstractListModel(parent)
    , d(new ResultModelPrivate(query, QString(), this))
{
    d->init();
}

ResultModel::ResultModel(Query query, const QString &clientId, QObject *parent)
    : QAbstractListModel(parent)
    , d(new ResultModelPrivate(query, clientId, this))
{
    d->init();
}

ResultModel::~ResultModel()
{
    delete d;
}

QHash<int, QByteArray> ResultModel::roleNames() const
{
    return {
        {ResourceRole, "resource"},
        {TitleRole, "title"},
        {ScoreRole, "score"},
        {FirstUpdateRole, "created"},
        {LastUpdateRole, "modified"},
        {LinkStatusRole, "linkStatus"},
        {LinkedActivitiesRole, "linkedActivities"},
        {MimeType, "mimeType"},
    };
}

QVariant ResultModel::data(const QModelIndex &item, int role) const
{
    const auto row = item.row();

    if (row < 0 || row >= d->cache.size()) {
        return QVariant();
    }

    const auto &result = d->cache[row];

    return role == Qt::DisplayRole     ? QString(result.title() + QStringLiteral(" ") + result.resource() + QStringLiteral(" - ")
                                             + QString::number(result.linkStatus()) + QStringLiteral(" - ") + QString::number(result.score()))
        : role == ResourceRole         ? result.resource()
        : role == TitleRole            ? result.title()
        : role == ScoreRole            ? result.score()
        : role == FirstUpdateRole      ? result.firstUpdate()
        : role == LastUpdateRole       ? result.lastUpdate()
        : role == LinkStatusRole       ? result.linkStatus()
        : role == LinkedActivitiesRole ? result.linkedActivities()
        : role == MimeType             ? result.mimetype()
        : role == Agent                ? result.agent()
                                       : QVariant();
}

QVariant ResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

int ResultModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->cache.size();
}

void ResultModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }
    d->fetch(ResultModelPrivate::FetchMore);
}

bool ResultModel::canFetchMore(const QModelIndex &parent) const
{
    return parent.isValid() ? false : d->cache.size() >= d->query.limit() ? false : d->hasMore;
}

void ResultModel::forgetResources(const QList<QString> &resources)
{
    const auto lstActivities = d->query.activities();
    for (const QString &activity : lstActivities) {
        const auto lstAgents = d->query.agents();
        for (const QString &agent : lstAgents) {
            for (const QString &resource : resources) {
                Stats::forgetResource(activity, agent == CURRENT_AGENT_TAG ? QCoreApplication::applicationName() : agent, resource);
            }
        }
    }
}

void ResultModel::forgetResource(const QString &resource)
{
    ResultModel::forgetResources({resource});
}

void ResultModel::forgetResource(int row)
{
    if (row >= d->cache.size()) {
        return;
    }
    const auto lstActivities = d->query.activities();
    for (const QString &activity : lstActivities) {
        const auto lstAgents = d->query.agents();
        for (const QString &agent : lstAgents) {
            Stats::forgetResource(activity, agent == CURRENT_AGENT_TAG ? QCoreApplication::applicationName() : agent, d->cache[row].resource());
        }
    }
}

void ResultModel::forgetAllResources()
{
    Stats::forgetResources(d->query);
}

void ResultModel::setResultPosition(const QString &resource, int position)
{
    d->cache.setLinkedResultPosition(resource, position);
}

void ResultModel::sortItems(Qt::SortOrder sortOrder)
{
    // TODO
    Q_UNUSED(sortOrder);
}

void ResultModel::linkToActivity(const QUrl &resource, const Terms::Activity &activity, const Terms::Agent &agent)
{
    d->watcher.linkToActivity(resource, activity, agent);
}

void ResultModel::unlinkFromActivity(const QUrl &resource, const Terms::Activity &activity, const Terms::Agent &agent)
{
    d->watcher.unlinkFromActivity(resource, activity, agent);
}

} // namespace Stats
} // namespace KActivities

#include "moc_resultmodel.cpp"
