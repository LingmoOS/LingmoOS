/*
 * This file is part of the KDE Milou Project
 * SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>
 * SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "resultsmodel.h"

#include "runnerresultsmodel_p.h"

#include <QIdentityProxyModel>
#include <QPointer>

#include <KConfigGroup>
#include <KDescendantsProxyModel>
#include <KModelIndexProxyMapper>
#include <KRunner/AbstractRunner>
#include <QTimer>
#include <cmath>

using namespace KRunner;

/**
 * Sorts the matches and categories by their type and relevance
 *
 * A category gets type and relevance of the highest
 * scoring match within.
 */
class SortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit SortProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
        sort(0, Qt::DescendingOrder);
    }

    void setQueryString(const QString &queryString)
    {
        const QStringList words = queryString.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (m_words != words) {
            m_words = words;
            invalidate();
        }
    }

protected:
    bool lessThan(const QModelIndex &sourceA, const QModelIndex &sourceB) const override
    {
        bool isCategoryComparison = !sourceA.internalId() && !sourceB.internalId();
        Q_ASSERT((bool)sourceA.internalId() == (bool)sourceB.internalId());
        // Only check the favorite index if we compare categories. For individual matches, they will always be the same
        if (isCategoryComparison) {
            const int favoriteA = sourceA.data(ResultsModel::FavoriteIndexRole).toInt();
            const int favoriteB = sourceB.data(ResultsModel::FavoriteIndexRole).toInt();
            bool isFavoriteA = favoriteA != -1;
            bool isFavoriteB = favoriteB != -1;
            if (isFavoriteA && !isFavoriteB) {
                return false;
            } else if (!isFavoriteA && isFavoriteB) {
                return true;
            }

            const int favoritesCount = sourceA.data(ResultsModel::FavoriteCountRole).toInt();
            const double favoriteAMultiplicationFactor = (favoriteA ? 1 + ((favoritesCount - favoriteA) * 0.2) : 1);
            const double typeA = sourceA.data(ResultsModel::CategoryRelevanceRole).toReal() * favoriteAMultiplicationFactor;
            const double favoriteBMultiplicationFactor = (favoriteB ? 1 + ((favoritesCount - favoriteB) * 0.2) : 1);
            const double typeB = sourceB.data(ResultsModel::CategoryRelevanceRole).toReal() * favoriteBMultiplicationFactor;
            return typeA < typeB;
        }

        const qreal relevanceA = sourceA.data(ResultsModel::RelevanceRole).toReal();
        const qreal relevanceB = sourceB.data(ResultsModel::RelevanceRole).toReal();

        if (!qFuzzyCompare(relevanceA, relevanceB)) {
            return relevanceA < relevanceB;
        }

        return QSortFilterProxyModel::lessThan(sourceA, sourceB);
    }

public:
    QStringList m_words;
};

/**
 * Distributes the number of matches shown per category
 *
 * Each category may occupy a maximum of 1/(n+1) of the given @c limit,
 * this means the further down you get, the less matches there are.
 * There is at least one match shown per category.
 *
 * This model assumes the results to already be sorted
 * descending by their relevance/score.
 */
class CategoryDistributionProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit CategoryDistributionProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent)
    {
    }
    void setSourceModel(QAbstractItemModel *sourceModel) override
    {
        if (this->sourceModel()) {
            disconnect(this->sourceModel(), nullptr, this, nullptr);
        }

        QSortFilterProxyModel::setSourceModel(sourceModel);

        if (sourceModel) {
            connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &CategoryDistributionProxyModel::invalidateFilter);
            connect(sourceModel, &QAbstractItemModel::rowsMoved, this, &CategoryDistributionProxyModel::invalidateFilter);
            connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, &CategoryDistributionProxyModel::invalidateFilter);
        }
    }

    int limit() const
    {
        return m_limit;
    }

    void setLimit(int limit)
    {
        if (m_limit == limit) {
            return;
        }
        m_limit = limit;
        invalidateFilter();
        Q_EMIT limitChanged();
    }

Q_SIGNALS:
    void limitChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (m_limit <= 0) {
            return true;
        }

        if (!sourceParent.isValid()) {
            return true;
        }

        const int categoryCount = sourceModel()->rowCount();

        int maxItemsInCategory = m_limit;

        if (categoryCount > 1) {
            int itemsBefore = 0;
            for (int i = 0; i <= sourceParent.row(); ++i) {
                const int itemsInCategory = sourceModel()->rowCount(sourceModel()->index(i, 0));

                // Take into account that every category gets at least one item shown
                const int availableSpace = m_limit - itemsBefore - std::ceil(m_limit / qreal(categoryCount));

                // The further down the category is the less relevant it is and the less space it my occupy
                // First category gets max half the total limit, second category a third, etc
                maxItemsInCategory = std::min(availableSpace, int(std::ceil(m_limit / qreal(i + 2))));

                // At least show one item per category
                maxItemsInCategory = std::max(1, maxItemsInCategory);

                itemsBefore += std::min(itemsInCategory, maxItemsInCategory);
            }
        }

        if (sourceRow >= maxItemsInCategory) {
            return false;
        }

        return true;
    }

private:
    // if you change this, update the default in resetLimit()
    int m_limit = 0;
};

/**
 * This model hides the root items of data originally in a tree structure
 *
 * KDescendantsProxyModel collapses the items but keeps all items in tact.
 * The root items of the RunnerMatchesModel represent the individual cateories
 * which we don't want in the resulting flat list.
 * This model maps the items back to the given @c treeModel and filters
 * out any item with an invalid parent, i.e. "on the root level"
 */
class HideRootLevelProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit HideRootLevelProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent)
    {
    }

    QAbstractItemModel *treeModel() const
    {
        return m_treeModel;
    }
    void setTreeModel(QAbstractItemModel *treeModel)
    {
        m_treeModel = treeModel;
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        KModelIndexProxyMapper mapper(sourceModel(), m_treeModel);
        const QModelIndex treeIdx = mapper.mapLeftToRight(sourceModel()->index(sourceRow, 0, sourceParent));
        return treeIdx.parent().isValid();
    }

private:
    QAbstractItemModel *m_treeModel = nullptr;
};

class KRunner::ResultsModelPrivate
{
public:
    explicit ResultsModelPrivate(const KConfigGroup &configGroup, const KConfigGroup &stateConfigGroup, ResultsModel *q)
        : q(q)
        , resultsModel(new RunnerResultsModel(configGroup, stateConfigGroup, q))
    {
    }

    ResultsModel *q;

    QPointer<KRunner::AbstractRunner> runner = nullptr;

    RunnerResultsModel *const resultsModel;
    SortProxyModel *const sortModel = new SortProxyModel(q);
    CategoryDistributionProxyModel *const distributionModel = new CategoryDistributionProxyModel(q);
    KDescendantsProxyModel *const flattenModel = new KDescendantsProxyModel(q);
    HideRootLevelProxyModel *const hideRootModel = new HideRootLevelProxyModel(q);
    const KModelIndexProxyMapper mapper{q, resultsModel};
};

ResultsModel::ResultsModel(QObject *parent)
    : ResultsModel(KConfigGroup(), KConfigGroup(), parent)
{
}
ResultsModel::ResultsModel(const KConfigGroup &configGroup, const KConfigGroup &stateConfigGroup, QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new ResultsModelPrivate(configGroup, stateConfigGroup, this))
{
    connect(d->resultsModel, &RunnerResultsModel::queryStringChanged, this, &ResultsModel::queryStringChanged);
    connect(d->resultsModel, &RunnerResultsModel::queryingChanged, this, &ResultsModel::queryingChanged);
    connect(d->resultsModel, &RunnerResultsModel::queryStringChangeRequested, this, &ResultsModel::queryStringChangeRequested);

    // The matches for the old query string remain on display until the first set of matches arrive for the new query string.
    // Therefore we must not update the query string inside RunnerResultsModel exactly when the query string changes, otherwise it would
    // re-sort the old query string matches based on the new query string.
    // So we only make it aware of the query string change at the time when we receive the first set of matches for the new query string.
    connect(d->resultsModel, &RunnerResultsModel::matchesChanged, this, [this]() {
        d->sortModel->setQueryString(queryString());
    });

    connect(d->distributionModel, &CategoryDistributionProxyModel::limitChanged, this, &ResultsModel::limitChanged);

    // The data flows as follows:
    // - RunnerResultsModel
    //   - SortProxyModel
    //     - CategoryDistributionProxyModel
    //       - KDescendantsProxyModel
    //         - HideRootLevelProxyModel

    d->sortModel->setSourceModel(d->resultsModel);

    d->distributionModel->setSourceModel(d->sortModel);

    d->flattenModel->setSourceModel(d->distributionModel);

    d->hideRootModel->setSourceModel(d->flattenModel);
    d->hideRootModel->setTreeModel(d->resultsModel);

    setSourceModel(d->hideRootModel);

    // Initialize the runners, this will speed the first query up.
    // While there were lots of optimizations, instantiating plugins, creating threads and AbstractRunner::init is still heavy work
    QTimer::singleShot(0, this, [this]() {
        runnerManager()->runners();
    });
}

ResultsModel::~ResultsModel() = default;

void ResultsModel::setFavoriteIds(const QStringList &ids)
{
    d->resultsModel->m_favoriteIds = ids;
    Q_EMIT favoriteIdsChanged();
}

QStringList ResultsModel::favoriteIds() const
{
    return d->resultsModel->m_favoriteIds;
}

QString ResultsModel::queryString() const
{
    return d->resultsModel->queryString();
}

void ResultsModel::setQueryString(const QString &queryString)
{
    d->resultsModel->setQueryString(queryString, singleRunner());
}

int ResultsModel::limit() const
{
    return d->distributionModel->limit();
}

void ResultsModel::setLimit(int limit)
{
    d->distributionModel->setLimit(limit);
}

void ResultsModel::resetLimit()
{
    setLimit(0);
}

bool ResultsModel::querying() const
{
    return d->resultsModel->querying();
}

QString ResultsModel::singleRunner() const
{
    return d->runner ? d->runner->id() : QString();
}

void ResultsModel::setSingleRunner(const QString &runnerId)
{
    if (runnerId == singleRunner()) {
        return;
    }
    if (runnerId.isEmpty()) {
        d->runner = nullptr;
    } else {
        d->runner = runnerManager()->runner(runnerId);
    }
    Q_EMIT singleRunnerChanged();
}

KPluginMetaData ResultsModel::singleRunnerMetaData() const
{
    return d->runner ? d->runner->metadata() : KPluginMetaData();
}

QHash<int, QByteArray> ResultsModel::roleNames() const
{
    auto names = QAbstractProxyModel::roleNames();
    names[IdRole] = QByteArrayLiteral("matchId"); // "id" is QML-reserved
    names[EnabledRole] = QByteArrayLiteral("enabled");
    names[CategoryRole] = QByteArrayLiteral("category");
    names[SubtextRole] = QByteArrayLiteral("subtext");
    names[UrlsRole] = QByteArrayLiteral("urls");
    names[ActionsRole] = QByteArrayLiteral("actions");
    names[MultiLineRole] = QByteArrayLiteral("multiLine");
    return names;
}

void ResultsModel::clear()
{
    d->resultsModel->clear();
}

bool ResultsModel::run(const QModelIndex &idx)
{
    KModelIndexProxyMapper mapper(this, d->resultsModel);
    const QModelIndex resultsIdx = mapper.mapLeftToRight(idx);
    if (!resultsIdx.isValid()) {
        return false;
    }
    return d->resultsModel->run(resultsIdx);
}

bool ResultsModel::runAction(const QModelIndex &idx, int actionNumber)
{
    KModelIndexProxyMapper mapper(this, d->resultsModel);
    const QModelIndex resultsIdx = mapper.mapLeftToRight(idx);
    if (!resultsIdx.isValid()) {
        return false;
    }
    return d->resultsModel->runAction(resultsIdx, actionNumber);
}

QMimeData *ResultsModel::getMimeData(const QModelIndex &idx) const
{
    if (auto resultIdx = d->mapper.mapLeftToRight(idx); resultIdx.isValid()) {
        return runnerManager()->mimeDataForMatch(d->resultsModel->fetchMatch(resultIdx));
    }
    return nullptr;
}

KRunner::RunnerManager *ResultsModel::runnerManager() const
{
    return d->resultsModel->runnerManager();
}

KRunner::QueryMatch ResultsModel::getQueryMatch(const QModelIndex &idx) const
{
    const QModelIndex resultIdx = d->mapper.mapLeftToRight(idx);
    return resultIdx.isValid() ? d->resultsModel->fetchMatch(resultIdx) : QueryMatch();
}

#include "moc_resultsmodel.cpp"
#include "resultsmodel.moc"
