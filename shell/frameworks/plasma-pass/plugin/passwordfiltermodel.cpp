// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "passwordfiltermodel.h"
#include "abbreviations.h"
#include "passwordsmodel.h"

#include <KDescendantsProxyModel>

#include <QFutureWatcher>
#include <QtConcurrent>

#include <chrono>
#include <iterator>

using namespace PlasmaPass;

namespace
{
constexpr const auto invalidateDelay = std::chrono::milliseconds(100);
constexpr const char *newFilterProperty = "newFilter";

class ModelIterator
{
public:
    using reference = const QModelIndex &;
    using pointer = QModelIndex *;
    using value_type = QModelIndex;
    using difference_type = int;
    using iterator_category = std::forward_iterator_tag;

    static ModelIterator begin(QAbstractItemModel *model)
    {
        return ModelIterator{model, model->index(0, 0)};
    }

    static ModelIterator end(QAbstractItemModel *model)
    {
        return ModelIterator{model, {}};
    }

    bool operator==(const ModelIterator &other) const
    {
        return mModel == other.mModel && mIndex == other.mIndex;
    }

    bool operator!=(const ModelIterator &other) const
    {
        return !(*this == other);
    }

    QModelIndex operator*() const
    {
        return mIndex;
    }

    const QModelIndex *operator->() const
    {
        return &mIndex;
    }

    ModelIterator &operator++()
    {
        if (mIndex.row() < mModel->rowCount() - 1) {
            mIndex = mModel->index(mIndex.row() + 1, mIndex.column());
        } else {
            mIndex = {};
        }
        return *this;
    }

    ModelIterator operator++(int)
    {
        ModelIterator it = *this;
        ++*this;
        return it;
    }

private:
    ModelIterator(QAbstractItemModel *model, const QModelIndex &index)
        : mModel(model)
        , mIndex(index)
    {
    }

private:
    QAbstractItemModel *mModel = nullptr;
    QModelIndex mIndex;
};

} // namespace

PasswordFilterModel::PathFilter::PathFilter(QString filter)
    : filter(std::move(filter))
{
}

PasswordFilterModel::PathFilter::PathFilter(const PathFilter &other)
    : filter(other.filter)
{
    updateParts();
}

PasswordFilterModel::PathFilter &PasswordFilterModel::PathFilter::operator=(const PathFilter &other)
{
    filter = other.filter;
    updateParts();
    return *this;
}

PasswordFilterModel::PathFilter::PathFilter(PathFilter &&other) noexcept
    : filter(std::move(other.filter))
{
    updateParts();
}

PasswordFilterModel::PathFilter &PasswordFilterModel::PathFilter::operator=(PathFilter &&other) noexcept
{
    filter = std::move(other.filter);
    updateParts();
    return *this;
}

void PasswordFilterModel::PathFilter::updateParts()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mParts = filter.splitRef(QLatin1Char('/'), Qt::SkipEmptyParts);
#else
    mParts = QStringView(filter).split(QLatin1Char('/'), Qt::SkipEmptyParts);
#endif
}

PasswordFilterModel::PathFilter::result_type PasswordFilterModel::PathFilter::operator()(const QModelIndex &index) const
{
    const auto path = index.model()->data(index, PasswordsModel::FullNameRole).toString();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const auto weight = matchPathFilter(path.splitRef(QLatin1Char('/')), mParts);
#else
    const auto weight = matchPathFilter(QStringView(path).split(QLatin1Char('/')), mParts);
#endif
    return std::make_pair(index, weight);
}

PasswordFilterModel::PasswordFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , mFlatModel(new KDescendantsProxyModel(this))
{
    mFlatModel->setDisplayAncestorData(false);
    sort(0); // enable sorting

    mUpdateTimer.setSingleShot(true);
    connect(&mUpdateTimer, &QTimer::timeout, this, &PasswordFilterModel::delayedUpdateFilter);
    connect(&mUpdateTimer, &QTimer::timeout, this, []() {
        qDebug() << "Update timer timeout, will calculate results lazily.";
    });
}

void PasswordFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    mFlatModel->setSourceModel(sourceModel);

    if (this->sourceModel() == nullptr) {
        QSortFilterProxyModel::setSourceModel(mFlatModel);
    }
}

QString PasswordFilterModel::passwordFilter() const
{
    return mFilter.filter;
}

void PasswordFilterModel::setPasswordFilter(const QString &filter)
{
    if (mFilter.filter != filter) {
        if (mUpdateTimer.isActive()) {
            mUpdateTimer.stop();
        }

        mUpdateTimer.setProperty(newFilterProperty, filter);
        mUpdateTimer.start(invalidateDelay);

        if (mFuture.isRunning()) {
            mFuture.cancel();
        }
        if (!filter.isEmpty()) {
            mFuture = QtConcurrent::mappedReduced<QHash<QModelIndex, int>>(ModelIterator::begin(sourceModel()),
                                                                           ModelIterator::end(sourceModel()),
                                                                           PathFilter{filter},
                                                                           [](QHash<QModelIndex, int> &result, const std::pair<QModelIndex, int> &value) {
                                                                               result.insert(value.first, value.second);
                                                                           });
            auto watcher = new QFutureWatcher<QHash<QModelIndex, int>>();
            connect(watcher, &QFutureWatcherBase::finished, this, [this, watcher]() {
                mSortingLookup = mFuture.result();
                watcher->deleteLater();
                // If the timer is not active it means we were to slow, so don't invoke
                // delayedUpdateFilter() again, just update mSortingLookup with our
                // results.
                if (mUpdateTimer.isActive()) {
                    mUpdateTimer.stop();
                    delayedUpdateFilter();
                }
            });
            connect(watcher, &QFutureWatcherBase::canceled, watcher, &QObject::deleteLater);
            watcher->setFuture(mFuture);
        }
    }
}

void PasswordFilterModel::delayedUpdateFilter()
{
    mFilter = PathFilter(mUpdateTimer.property(newFilterProperty).toString());
    Q_EMIT passwordFilterChanged();
    if (mFuture.isRunning()) {
        // If the future is still running, clear up the lookup table, we will have to
        // calculate the intermediate results ourselves
        mSortingLookup.clear();
    }
    invalidate();
}

QVariant PasswordFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return data(index, PasswordsModel::FullNameRole);
    }

    return QSortFilterProxyModel::data(index, role);
}

bool PasswordFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto src_index = sourceModel()->index(source_row, 0, source_parent);
    const auto type = static_cast<PasswordsModel::EntryType>(sourceModel()->data(src_index, PasswordsModel::EntryTypeRole).toInt());
    if (type == PasswordsModel::FolderEntry) {
        return false;
    }

    if (mFilter.filter.isEmpty()) {
        return true;
    }

    // Try to lookup the weight in the lookup table, the worker thread may have put it in there
    // while the updateTimer was ticking
    auto weight = mSortingLookup.find(src_index);
    if (weight == mSortingLookup.end()) {
        // It's not there, let's calculate the value now
        const auto result = mFilter(src_index);
        weight = mSortingLookup.insert(result.first, result.second);
    }

    return *weight > -1;
}

bool PasswordFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto weightLeft = mSortingLookup.value(source_left, -1);
    const auto weightRight = mSortingLookup.value(source_right, -1);

    if (weightLeft == weightRight) {
        const auto nameLeft = source_left.data(PasswordsModel::FullNameRole).toString();
        const auto nameRight = source_right.data(PasswordsModel::FullNameRole).toString();
        return QString::localeAwareCompare(nameLeft, nameRight) < 0;
    }

    return weightLeft < weightRight;
}

#include "moc_passwordfiltermodel.cpp"
