/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "commentsmodel.h"

#include "core/commentsmodel.h"

namespace KNewStuffQuick
{
class CommentsModelPrivate
{
public:
    CommentsModelPrivate(CommentsModel *qq)
        : q(qq)
    {
    }
    CommentsModel *q;
    ItemsModel *itemsModel{nullptr};
    KNSCore::Entry entry;
    bool componentCompleted{false};
    CommentsModel::IncludedComments includedComments{CommentsModel::IncludeAllComments};

    QSharedPointer<KNSCore::Provider> provider;
    void resetConnections()
    {
        if (componentCompleted && itemsModel) {
            q->setSourceModel(qobject_cast<QAbstractListModel *>(
                itemsModel->data(itemsModel->index(itemsModel->indexOfEntry(entry)), ItemsModel::CommentsModelRole).value<QObject *>()));
        }
    }

    bool hasReview(const QModelIndex &index, bool checkParents = false)
    {
        bool result{false};
        if (q->sourceModel()) {
            if (q->sourceModel()->data(index, KNSCore::CommentsModel::ScoreRole).toInt() > 0) {
                result = true;
            }
            if (result == false && checkParents) {
                QModelIndex parentIndex = q->sourceModel()->index(q->sourceModel()->data(index, KNSCore::CommentsModel::ParentIndexRole).toInt(), 0);
                if (parentIndex.isValid()) {
                    result = hasReview(parentIndex, true);
                }
            }
        }
        return result;
    }
};
}

using namespace KNewStuffQuick;

CommentsModel::CommentsModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new CommentsModelPrivate(this))
{
}

CommentsModel::~CommentsModel() = default;

void KNewStuffQuick::CommentsModel::classBegin()
{
}

void KNewStuffQuick::CommentsModel::componentComplete()
{
    d->componentCompleted = true;
    d->resetConnections();
}

ItemsModel *CommentsModel::itemsModel() const
{
    return d->itemsModel;
}

void CommentsModel::setItemsModel(ItemsModel *newItemsModel)
{
    if (d->itemsModel != newItemsModel) {
        d->itemsModel = newItemsModel;
        d->resetConnections();
        Q_EMIT itemsModelChanged();
    }
}

KNSCore::Entry CommentsModel::entry() const
{
    return d->entry;
}

void CommentsModel::setEntry(const KNSCore::Entry &entry)
{
    d->entry = entry;
    d->resetConnections();
    Q_EMIT entryChanged();
}

CommentsModel::IncludedComments KNewStuffQuick::CommentsModel::includedComments() const
{
    return d->includedComments;
}

void KNewStuffQuick::CommentsModel::setIncludedComments(CommentsModel::IncludedComments includedComments)
{
    if (d->includedComments != includedComments) {
        d->includedComments = includedComments;
        invalidateFilter();
        Q_EMIT includedCommentsChanged();
    }
}

bool KNewStuffQuick::CommentsModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    bool result{false};
    switch (d->includedComments) {
    case IncludeOnlyReviews:
        result = d->hasReview(sourceModel()->index(sourceRow, 0, sourceParent));
        break;
    case IncludeReviewsAndReplies:
        result = d->hasReview(sourceModel()->index(sourceRow, 0, sourceParent), true);
        break;
    case IncludeAllComments:
    default:
        result = true;
        break;
    }
    return result;
}

#include "moc_commentsmodel.cpp"
