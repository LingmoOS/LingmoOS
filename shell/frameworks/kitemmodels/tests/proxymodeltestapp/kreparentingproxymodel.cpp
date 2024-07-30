/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kreparentingproxymodel.h"
#include <QDebug>
#include <QList>

#include <algorithm>
#include <functional>

class KReparentingProxyModelPrivate
{
    KReparentingProxyModelPrivate(KReparentingProxyModel *proxyModel)
        : m_nextId(0)
        , q_ptr(proxyModel)
    {
    }

    qint64 newId() const
    {
        return m_nextId++;
    }

    enum MapStrategy {
        MapDescendants,
        MapChildrenOnly,
    };

    /**
      Creates mappings of indexes in the source model between @p start
      and @p end which should be represented in the proxy model as descendants
      of @p parent.
    */
    QHash<QModelIndex, QModelIndexList> recreateMappings(const QModelIndex &parent, int start, int end = -1, int strategy = MapChildrenOnly) const;

    /**
      Merges all indexes from @p mappings which are descendants of @p parent into the model.
      Returns the remaining mappings.

      Note that this changes the internal model structure and must only be called between begin/end insert/remove/move/reset calls.
    */
    QHash<QModelIndex, QModelIndexList> mergeDescendants(QHash<QModelIndex, QModelIndexList> mappings, const QModelIndex &parent, int start);

    /**
      Verifies that the indexes below @p parent between @p start and rowCount(parent)
      are in the correct positions in the proxy model. Repositions them if not.
    */
    void verifyStructure(const QModelIndex &parent, int start);

    /**
      Returns the index vertically below index in the model @p model.
      If @p model is 0, the sourceModel is used
      Returns an invalid index if there is no index below @p index.
    */
    QModelIndex getIndexBelow(const QModelIndex &index, QAbstractItemModel *model = nullptr) const;

    /**
      Returns the last descendant of @p index or itself if it has no children
    */
    QModelIndex getLastDescendant(const QModelIndex &index) const;

    bool isDescendantInModel(const QModelIndex &ancestor, const QModelIndex &descendant) const;

    /**
      Returns the ancestors of @p descendant that are already in the proxy model.

      Note that @p descendant does not have to be in the proxy yet, and it is not part of the
      result list.
    */
    QList<QModelIndex> getExistingAncestors(const QModelIndex &descendant) const;

    void sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow);
    void sourceRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow);
    void sourceModelAboutToBeReset();
    void endResetProxy();
    void sourceModelReset();
    void sourceLayoutAboutToBeChanged();
    void sourceLayoutChanged();
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    mutable QHash<qint64, QPersistentModelIndex> m_parents;
    mutable QHash<QPersistentModelIndex, QList<QPersistentModelIndex>> m_childIndexes;

    struct PendingInsertion {
        PendingInsertion()
            : parentId(-1)
            , start(-1)
            , end(-1)
        {
        }

        PendingInsertion(const QModelIndex &_index, int _start, int _end)
            : index(_index)
            , start(_start)
            , end(_end)
        {
        }

        QPersistentModelIndex index;
        QModelIndex sourceIndex;
        qint64 parentId;
        int start;
        int end;
    };

    struct PendingRemoval : PendingInsertion {
        int numTrailing;
    };
    // Needed between the beginRemoveRows and endRemoveRows signals.
    mutable QHash<qint64, QPersistentModelIndex> m_pendingRemovalParents;
    mutable QHash<QPersistentModelIndex, QList<QPersistentModelIndex>> m_pendingRemovalChildIndexes;

    QHash<QModelIndex, QModelIndexList> insertTree(QHash<QModelIndex, QModelIndexList> mappings, const QModelIndex &parent);

    void handleInsertion(const PendingInsertion &pendingInsertion);

    void handleRemoval(const PendingRemoval &pendingRemoval);

    mutable QHash<QModelIndex, PendingInsertion> m_pendingInsertions;
    mutable QList<PendingRemoval> m_pendingRemovals;

    mutable qint64 m_nextId;

    Q_DECLARE_PUBLIC(KReparentingProxyModel)
    KReparentingProxyModel *q_ptr;

    QList<QPersistentModelIndex> m_layoutChangePersistentIndexes;
    QModelIndexList m_proxyIndexes;

    void emitDataChangedSignals(const QModelIndex &parent, int maxChanged);

    /**
      Given @p parent in the proxy model, return the last index lying between @p start and @p end
      which is also a descendant of @p parent.
    */
    QModelIndex findLastInParent(QModelIndex parent, int start, int end);

    /**
      Removes @P idx (which is a source model index) and its children from the model data structures.
    */
    void removeTree(const QPersistentModelIndex &idx, int start = 0, int end = -1);

    int pendingRemovalRowCount(const QModelIndex &sourceIndex) const;
};

class LessThan
{
    const KReparentingProxyModel *m_model;

public:
    LessThan(const KReparentingProxyModel *model)
        : m_model(model)
    {
    }
    bool operator()(const QModelIndex &ancestor, const QModelIndex &descendant)
    {
        return m_model->isDescendantOf(ancestor, descendant);
    }
};

QModelIndex KReparentingProxyModelPrivate::getIndexBelow(const QModelIndex &index, QAbstractItemModel *model) const
{
    Q_Q(const KReparentingProxyModel);

    //   qDebug() << index.data() << index;

    if (!model) {
        model = q->sourceModel();
    }

    if (model->hasChildren(index)) {
        return model->index(0, 0, index);
    }

    QModelIndex sibling = index.sibling(index.row() + 1, index.column());
    if (sibling.isValid()) {
        return sibling;
    }

    QModelIndex parent = index.parent();

    if (!parent.isValid()) {
        return QModelIndex();
    }

    int affectedRow = index.row();
    const int column = 0;

    while (parent.isValid()) {
        //     qDebug() << "parent" << parent.data() << model->rowCount(parent) << affectedRow;
        if (affectedRow < model->rowCount(parent) - 1) {
            return model->index(affectedRow + 1, column, parent);
        }

        affectedRow = parent.row();
        parent = parent.parent();
    }

    if (model->rowCount(parent) >= affectedRow) {
        return model->index(affectedRow + 1, column, parent);
    }

    return QModelIndex();
}

QModelIndex KReparentingProxyModelPrivate::getLastDescendant(const QModelIndex &index) const
{
    Q_Q(const KReparentingProxyModel);

    QModelIndex proxyIndex = q->mapFromSource(index);

    while (q->hasChildren(proxyIndex)) {
        proxyIndex = q->index(q->rowCount(proxyIndex), proxyIndex.column(), proxyIndex);
        if (!proxyIndex.isValid()) {
            break;
        }
    }
    return q->mapToSource(proxyIndex);
}

QList<QModelIndex> KReparentingProxyModelPrivate::getExistingAncestors(const QModelIndex &descendant) const
{
    Q_Q(const KReparentingProxyModel);

    QList<QModelIndex> vector;
    if (!descendant.isValid()) {
        return vector;
    }

    QModelIndex parent = q->mapFromSource(descendant).parent();
    QModelIndex sourceParent = q->mapToSource(parent);
    if (!sourceParent.isValid()) {
        return vector;
    }

    vector.append(sourceParent);
    while (parent.isValid()) {
        parent = parent.parent();
        sourceParent = q->mapToSource(parent);
        if (!sourceParent.isValid()) {
            return vector;
        }
        vector.prepend(sourceParent);
    }
    return vector;
}

QHash<QModelIndex, QModelIndexList> KReparentingProxyModelPrivate::recreateMappings(const QModelIndex &ancestor, int start, int end, int strategy) const
{
    Q_Q(const KReparentingProxyModel);
    const int column = 0;

    QHash<QModelIndex, QModelIndexList> mappings;
    // Handle listing the root QModelIndex().
    if (!ancestor.isValid() && !q->sourceModel()->hasChildren())
    // Empty model. Nothing to do.
    {
        return mappings;
    }

    // A
    // - B
    // - - C
    // - D
    // If start refers to D, existing ancestors will contain only A.
    // We need to go 'up' to C and get its ancestors in case D is to be made a child of B or C (for example if B and C have just been inserted)
    QModelIndex indexAbove;
    if (start > 0) {
        indexAbove = getLastDescendant(q->sourceModel()->index(start - 1, column, ancestor));
    } else {
        indexAbove = ancestor;
    }

    QList<QModelIndex> ancestors = getExistingAncestors(indexAbove);

    //   ancestors.append(indexAbove);
    //   qDebug() << ancestors;
    QModelIndex nextIndex = ancestor;

    for (int row = start; (row <= end || end == -1);) {
        // A
        // - B
        // - - C
        // - D
        // The nextIndex of the invalid QModelIndex is A,
        // The nextIndex of A is B,
        // The nextIndex of B is C,
        // The nextIndex of C is D,
        // The nextIndex of D is invalid,
        // When the nextIndex is invalid we're finished creating mappings.
        if (MapDescendants == strategy) {
            nextIndex = getIndexBelow(nextIndex);
        } else {
            nextIndex = q->sourceModel()->index(row, column, ancestor);
        }

        if (!nextIndex.isValid()) {
            break;
        }

        const QList<QModelIndex>::iterator ancestorIt = std::lower_bound(ancestors.begin(), ancestors.end(), nextIndex, LessThan(q));

        ancestors.erase(ancestorIt, ancestors.end());

        QModelIndex parent;
        if (ancestorIt != ancestors.begin()) {
            parent = *(ancestorIt - 1);
        }

        ancestors.append(nextIndex);

        mappings[parent].append(nextIndex);
    }

    return mappings;
}

void KReparentingProxyModelPrivate::verifyStructure(const QModelIndex &sourceParent, int sourceStart)
{
    Q_Q(KReparentingProxyModel);

    // If the start structure is:

    // C
    // D
    // E

    // and then A and B are inserted, we may need to move C D and E. Not all of the siblings will
    // necessarily be moved to the same destination parent.
    // Some example finished scenarios depending on the outcome of isDescendantOf:

    // A
    // B
    // C
    // D
    // E

    // A
    // B
    // - C
    // - D
    // - E

    // A
    // - B
    // - C
    // - D
    // - E

    // A
    // - B
    // - - C
    // - D
    // E

    // Local variable mappings now contains all the information about finished state
    // When we locate the first child to be moved, we process it and its siblings

    QHash<QModelIndex, QModelIndexList> mappings = recreateMappings(sourceParent, sourceStart, -1);

    if (mappings.isEmpty()) {
        return;
    }

    QModelIndex sourceFirstIndex = q->sourceModel()->index(sourceStart, 0, sourceParent);

    QModelIndex destinationParent;
    QModelIndexList movedIndexes;

    QHashIterator<QModelIndex, QModelIndexList> it(mappings);
    while (it.hasNext()) {
        it.next();
        //     qDebug() << it.key() << it.key().data() << it.value();
        if (it.value().at(0) == sourceFirstIndex) {
            destinationParent = it.key();
            movedIndexes = it.value();
            break;
        }
    }

    Q_FOREVER {
        if (destinationParent == sourceParent)
        // No indexes moved
        {
            return;
        }

        Q_ASSERT(destinationParent.isValid());
        Q_ASSERT(!movedIndexes.isEmpty());

        // It's only possible for things to move right, and even that's only an option
        // for children of parent, but not their descendants. ie, children of C D and E will not need to be reparented.
        // They are already in the correct positions.

        QList<QPersistentModelIndex> &existingSourceIndexes = m_childIndexes[sourceParent];
        QList<QPersistentModelIndex> existingDestinationIndexes = m_childIndexes[destinationParent];

        QModelIndex proxySourceParent = q->mapFromSource(sourceParent);
        QModelIndex proxyDestinationParent = q->mapFromSource(destinationParent);

        // That is, start position of indexes to be moved from the source parent.
        int proxySourceStart = m_childIndexes.value(sourceParent).indexOf(movedIndexes.at(0));
        int proxySourceEnd = proxySourceStart + movedIndexes.size() - 1;

        // The moved indexes are appended to the destinationParent. Nothing else is possible.
        // If they were to be inserted in the middle somewhere, they would already be there.

        int destinationRow = existingDestinationIndexes.size();

        bool allowMove = q->beginMoveRows(proxySourceParent, proxySourceStart, proxySourceEnd, proxyDestinationParent, destinationRow);
        Q_ASSERT(allowMove);

        for (int row = proxySourceEnd; row >= proxySourceStart; --row) {
            existingSourceIndexes.removeAt(row);
        }

        QHash<QModelIndex, QModelIndexList> mapping;
        mapping.insert(destinationParent, movedIndexes);
        mergeDescendants(mapping, destinationParent, existingDestinationIndexes.size());

        q->endMoveRows();

        if (!mappings.contains(q->mapToSource(proxyDestinationParent.parent()))) {
            break;
        }

        destinationParent = q->mapToSource(proxyDestinationParent.parent());
        movedIndexes = mappings.value(destinationParent);
    }
}

KReparentingProxyModel::KReparentingProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , d_ptr(new KReparentingProxyModelPrivate(this))
{
}

KReparentingProxyModel::~KReparentingProxyModel()
{
    delete d_ptr;
}

bool KReparentingProxyModelPrivate::isDescendantInModel(const QModelIndex &ancestor, const QModelIndex &descendant) const
{
    //   qDebug() << ancestor.data() << descendant.data();

    //   if (!ancestor.isValid())
    //     return true;

    QModelIndex _ancestor = descendant.parent();
    while (_ancestor.isValid()) {
        if (_ancestor == ancestor) {
            return true;
        }
        _ancestor = _ancestor.parent();
    }
    return (!ancestor.isValid() && descendant.isValid());
}

bool KReparentingProxyModel::isDescendantOf(const QModelIndex &ancestor, const QModelIndex &descendant) const
{
    Q_D(const KReparentingProxyModel);
    return d->isDescendantInModel(ancestor, descendant);
    //   return (!ancestor.isValid() && descendant.isValid());
}

QModelIndex KReparentingProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    Q_D(const KReparentingProxyModel);
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }

    QModelIndex sourceIndexFirstColumn = sourceIndex.sibling(sourceIndex.row(), 0);

    QHash<QPersistentModelIndex, QList<QPersistentModelIndex>>::const_iterator it;
    const QHash<QPersistentModelIndex, QList<QPersistentModelIndex>>::const_iterator begin = d->m_childIndexes.constBegin();
    const QHash<QPersistentModelIndex, QList<QPersistentModelIndex>>::const_iterator end = d->m_childIndexes.constEnd();

    for (it = begin; it != end; ++it) {
        QList<QPersistentModelIndex> list = it.value();
        if (list.contains(sourceIndexFirstColumn)) {
            QModelIndex sourceParent = it.key();
            int row = list.indexOf(sourceIndexFirstColumn);

            // There must have been a mapping made for it.
            Q_ASSERT(d->m_parents.values().contains(sourceParent));

            qint64 id = d->m_parents.key(sourceParent);

            // id refers to the parent.
            return createIndex(row, sourceIndex.column(), reinterpret_cast<void *>(id));
        }
    }
    return QModelIndex();
}

QModelIndex KReparentingProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    Q_D(const KReparentingProxyModel);

    //   qDebug() << "MMMMMM" << proxyIndex;

    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }

    qint64 id = reinterpret_cast<qint64>(proxyIndex.internalPointer());

    //   if (!d->m_parents.contains(id))
    //     qDebug() << d->m_parents << id;

    QModelIndex sourceParent;
    if (d->m_pendingRemovalParents.contains(id)) {
        //     qDebug() << "pending";
        sourceParent = d->m_pendingRemovalParents.value(id);
    } else {
        Q_ASSERT(d->m_parents.contains(id));
        sourceParent = d->m_parents.value(id);
    }

    // qDebug() <<  sourceParent << sourceParent.data();

    QModelIndex sourceIndexFirstColumn;
    if (d->m_pendingRemovalChildIndexes.contains(sourceParent)) {
        //     qDebug() << "#############";

        for (const KReparentingProxyModelPrivate::PendingRemoval &pendingRemoval : std::as_const(d->m_pendingRemovals)) {
            //       qDebug() << "In" << pendingRemoval.index << pendingRemoval.sourceIndex << sourceParent;
            if (pendingRemoval.sourceIndex == sourceParent) {
                //         qDebug() << "Out" << pendingRemoval.sourceIndex << sourceParent;
                int proxyRow = proxyIndex.row();
                int row = proxyRow - pendingRemoval.start;

                //         qDebug() << d->m_pendingRemovalChildIndexes.value(sourceParent) << proxyRow << row << pendingRemoval.end;

                if (proxyRow > pendingRemoval.end) {
                    Q_ASSERT(d->m_childIndexes.contains(sourceParent));
                    row = proxyRow - (pendingRemoval.end - pendingRemoval.start + 1);
                    //           qDebug() << "new row" << row;
                    sourceIndexFirstColumn = d->m_childIndexes.value(sourceParent).at(row);
                } else {
                    sourceIndexFirstColumn = d->m_pendingRemovalChildIndexes.value(sourceParent).at(row);
                }
                break;
            }
        }
    } else {
        Q_ASSERT(d->m_childIndexes.contains(sourceParent));
        sourceIndexFirstColumn = d->m_childIndexes.value(sourceParent).at(proxyIndex.row());
    }

    Q_ASSERT(sourceIndexFirstColumn.isValid());

    return sourceIndexFirstColumn.sibling(sourceIndexFirstColumn.row(), proxyIndex.column());
}

int KReparentingProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_D(const KReparentingProxyModel);

    if (!sourceModel()) {
        return 0;
    }

    if (!parent.isValid()) {
        return sourceModel()->columnCount();
    }

    if (parent.column() > 0) {
        return 0;
    }
    QModelIndex sourceIndex = mapToSource(parent);

    return (d->m_childIndexes.value(sourceIndex).size() > 0) ? sourceModel()->columnCount() : 0;
}

QVariant KReparentingProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    return QAbstractProxyModel::data(proxyIndex, role);
}

QModelIndex KReparentingProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const KReparentingProxyModel);

    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    QModelIndex sourceParent = mapToSource(parent);

    //   if (!d->m_pendingRemovals.isEmpty())
    //     qDebug() << sourceParent << sourceParent.data();

    // ### This is where we need to have the children of removed indexes stored.

    //   if (!d->m_parents.values().contains(sourceParent))
    //   {
    //     qDebug() << d->m_pendingRemovalParents.values() << sourceParent << d->m_pendingRemovalParents.values().contains(sourceParent);
    //   }

    qint64 id;
    if (d->m_pendingRemovalParents.values().contains(sourceParent)) {
        id = d->m_pendingRemovalParents.key(sourceParent);
    } else {
        // There must have been a mapping made for it.
        Q_ASSERT(d->m_parents.values().contains(sourceParent));
        id = d->m_parents.key(sourceParent);
    }
    return createIndex(row, column, reinterpret_cast<void *>(id));
}

QModelIndex KReparentingProxyModel::parent(const QModelIndex &child) const
{
    Q_D(const KReparentingProxyModel);

    if (!child.isValid()) {
        return QModelIndex();
    }

    QModelIndex sourceIndex = mapToSource(child);

    QModelIndex firstColumnChild = sourceIndex;
    if (sourceIndex.column() > 0) {
        firstColumnChild = sourceIndex.sibling(sourceIndex.row(), 0);
    }

    QHashIterator<QPersistentModelIndex, QList<QPersistentModelIndex>> itPending(d->m_pendingRemovalChildIndexes);

    while (itPending.hasNext()) {
        itPending.next();

        if (itPending.value().contains(firstColumnChild)) {
            return mapFromSource(itPending.key());
        }
    }

    QHashIterator<QPersistentModelIndex, QList<QPersistentModelIndex>> it(d->m_childIndexes);

    while (it.hasNext()) {
        it.next();

        if (it.value().contains(firstColumnChild)) {
            return mapFromSource(it.key());
        }
    }
    return QModelIndex();
}

int KReparentingProxyModelPrivate::pendingRemovalRowCount(const QModelIndex &sourceIndex) const
{
    for (const PendingRemoval &pendingRemoval : std::as_const(m_pendingRemovals)) {
        //     qDebug() << pendingRemoval.sourceIndex;
        if (pendingRemoval.sourceIndex == sourceIndex) {
            return pendingRemoval.end - pendingRemoval.start + 1;
        }
    }
    return 0;
}

int KReparentingProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const KReparentingProxyModel);

    if (parent.column() > 0) {
        return 0;
    }

    QModelIndex sourceIndex = mapToSource(parent);

    int size = d->m_childIndexes.value(sourceIndex).size() + d->m_pendingRemovalChildIndexes.value(sourceIndex).size();

    //   qDebug() << d->m_pendingRemovalChildIndexes.value(sourceIndex).size();

    //   if (!d->m_pendingRemovals.isEmpty())
    //   {
    //     qDebug() << "SIZE" << sourceIndex << sourceIndex.data() << size << d->m_pendingRemovals.size() << d->pendingRemovalRowCount(sourceIndex);
    //   }

    return size;
}

bool KReparentingProxyModel::hasChildren(const QModelIndex &parent) const
{
    return rowCount(parent) > 0;
}

void KReparentingProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    Q_D(KReparentingProxyModel);

    beginResetModel();

    disconnect(sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)), this, SLOT(sourceRowsAboutToBeInserted(QModelIndex, int, int)));
    disconnect(sourceModel, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(sourceRowsInserted(QModelIndex, int, int)));
    disconnect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
    disconnect(sourceModel, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(sourceRowsRemoved(QModelIndex, int, int)));
    disconnect(sourceModel,
               SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
               this,
               SLOT(sourceRowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
    disconnect(sourceModel, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)), this, SLOT(sourceRowsMoved(QModelIndex, int, int, QModelIndex, int)));
    disconnect(sourceModel, SIGNAL(modelAboutToBeReset()), this, SLOT(sourceModelAboutToBeReset()));
    disconnect(sourceModel, SIGNAL(modelReset()), this, SLOT(sourceModelReset()));
    disconnect(sourceModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(sourceDataChanged(QModelIndex, QModelIndex)));
    disconnect(sourceModel, SIGNAL(layoutAboutToBeChanged()), this, SLOT(sourceLayoutAboutToBeChanged()));
    disconnect(sourceModel, SIGNAL(layoutChanged()), this, SLOT(sourceLayoutChanged()));

    QAbstractProxyModel::setSourceModel(sourceModel);

    //   qDebug() << "set";
    QHash<QModelIndex, QModelIndexList> mappings =
        d->recreateMappings(QModelIndex(), 0, sourceModel->rowCount() - 1, KReparentingProxyModelPrivate::MapDescendants);
    //   qDebug() << "begin";
    d->mergeDescendants(mappings, QModelIndex(), 0);

    connect(sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)), SLOT(sourceRowsAboutToBeInserted(QModelIndex, int, int)));
    connect(sourceModel, SIGNAL(rowsInserted(QModelIndex, int, int)), SLOT(sourceRowsInserted(QModelIndex, int, int)));
    connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
    connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex, int, int)), SLOT(sourceRowsRemoved(QModelIndex, int, int)));
    connect(sourceModel,
            SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
            SLOT(sourceRowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
    connect(sourceModel, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)), SLOT(sourceRowsMoved(QModelIndex, int, int, QModelIndex, int)));
    connect(sourceModel, SIGNAL(modelAboutToBeReset()), SLOT(sourceModelAboutToBeReset()));
    connect(sourceModel, SIGNAL(modelReset()), SLOT(sourceModelReset()));
    connect(sourceModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), SLOT(sourceDataChanged(QModelIndex, QModelIndex)));
    connect(sourceModel, SIGNAL(layoutAboutToBeChanged()), SLOT(sourceLayoutAboutToBeChanged()));
    connect(sourceModel, SIGNAL(layoutChanged()), SLOT(sourceLayoutChanged()));

    endResetModel();
}

void KReparentingProxyModelPrivate::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    Q_Q(KReparentingProxyModel);
    return q->beginResetModel();
#if 0
    // We can't figure out the structure until the indexes are in the model.
    // Store the signal until the new rows are actually there in sourceRowsInserted.
    PendingInsertion pendingInsertion(parent, start, end);
    m_pendingInsertions.insert(parent, pendingInsertion);
#endif
}

QHash<QModelIndex, QModelIndexList>
KReparentingProxyModelPrivate::mergeDescendants(QHash<QModelIndex, QModelIndexList> mappings, const QModelIndex &parent, int start)
{
    const QModelIndexList childIndexes = mappings.take(parent);
    //   qDebug() << childIndexes;
    if (!childIndexes.isEmpty()) {
        if (!m_parents.values().contains(parent)) {
            m_parents.insert(newId(), QPersistentModelIndex(parent));
        }
    }
    int row = start;
    for (const QModelIndex &idx : childIndexes) {
        m_childIndexes[parent].insert(row++, QPersistentModelIndex(idx));
        mappings = mergeDescendants(mappings, idx, 0);
    }
    return mappings;
}

QHash<QModelIndex, QModelIndexList> KReparentingProxyModelPrivate::insertTree(QHash<QModelIndex, QModelIndexList>, const QModelIndex &)
{
    return QHash<QModelIndex, QModelIndexList>();
}

void KReparentingProxyModelPrivate::handleInsertion(const PendingInsertion &pendingInsertion)
{
    Q_Q(KReparentingProxyModel);
    QModelIndex parent = pendingInsertion.index;
    int start = pendingInsertion.start;
    int end = pendingInsertion.end;

    //   qDebug() << parent << parent.data() << start << end;

    //   for (int i = start; i < end; ++i)
    //   {
    //     QModelIndex idx = q->sourceModel()->index(i, 0, parent);
    //     qDebug() << idx << idx.data();
    //   }

    QHash<QModelIndex, QModelIndexList> newItemMappings = recreateMappings(parent, start, end, KReparentingProxyModelPrivate::MapDescendants);

    // iterate over keys. if key in keys iterate up. This gives list of top level parents.
    // Pick the one whose parent is @p parent. Insert it. Look up until find the parent of another one and insert that.
    // If one of the parents is invalid it is necessarily the last one to be processed (if there are more to process, they'll be children of it)
    // That case should work too.

    //   qDebug() << "new item mappings" << newItemMappings;

    const int column = 0;

    //   qDebug() << m_childIndexes.contains(parent);

    if (newItemMappings.contains(parent)) {
        QModelIndexList newItemList = newItemMappings.value(parent);
        //     qDebug() << "newItemList" << newItemList;
        int proxyStart = 0;

        // A single insertion in the source model might be multiple insertions in the proxy model.
        Q_FOREVER {
            if (newItemList.isEmpty()) {
                if (!newItemMappings.contains(parent.parent())) {
                    break;
                }

                newItemList = newItemMappings.value(parent.parent());
                continue;
            }

            proxyStart = 0;

            QModelIndex proxyParent = q->mapFromSource(parent);
            if (start > 0) {
                QModelIndex lastDesc = q->mapFromSource(getLastDescendant(q->sourceModel()->index(start - 1, column, parent)));

                while (lastDesc.parent() != proxyParent) {
                    lastDesc = lastDesc.parent();
                }
                proxyStart = lastDesc.row() + 1;
            }

            q->beginInsertRows(proxyParent, proxyStart, proxyStart + newItemList.size() - 1);

            newItemMappings = mergeDescendants(newItemMappings, parent, proxyStart);

            q->endInsertRows();

            if (!newItemMappings.contains(parent.parent())) {
                break;
            }

            newItemList = newItemMappings.value(parent.parent());
        }
    }

    //   // The rest are not descendants of pendingInsertion.index in the proxy model, but are elsewhere.
    //   Q_FOREACH(const QModelIndex &parent, newItemMappings.keys())
    //   {
    //
    //   }

    return;
}

void KReparentingProxyModelPrivate::sourceRowsInserted(const QModelIndex &parent, int, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(end);

    return endResetProxy();

#if 0
    Q_Q(KReparentingProxyModel);
    if (m_pendingInsertions.contains(parent)) {
        PendingInsertion pendingInsertion = m_pendingInsertions.value(parent);
        handleInsertion(pendingInsertion);

        if (q->sourceModel()->rowCount(parent) <= (end + 1)) {
            return;
        }

        // The presence of new rows might affect the structure of indexes below.
        verifyStructure(parent, end + 1);
    }
#endif
}

void KReparentingProxyModelPrivate::removeTree(const QPersistentModelIndex &idxToRemove, int start, int end)
{
    if (!m_childIndexes.contains(idxToRemove)) {
        return;
    }

    //   qDebug() << "idxToRemove" << idxToRemove << start << end;

    QList<QPersistentModelIndex> &toRemove = m_childIndexes[idxToRemove];
    //   qDebug() << toRemove << toRemove.size();

    //   QList<int> intList;
    //   intList << 1 << 2 << 3 << 4 << 5;
    //
    //   QList<int>::iterator intit = intList.begin();
    //   QList<int>::iterator intendIt = intList.end();
    //
    //   if (end == 0)
    //     intendIt = intit + 1;
    //
    //   if (end > 0)
    //   {
    //     intendIt = intit + (end - start + 1) + 1;
    //     qDebug() << "intend" << *intendIt;
    //   }
    //   intit += start;
    //
    //   while (intit != intendIt)
    //   {
    //     int i = *intit;
    //     qDebug() << i;
    //     intit = intList.erase(intit);
    //   }

    QList<QPersistentModelIndex>::iterator it = toRemove.begin();
    QList<QPersistentModelIndex>::iterator endIt = toRemove.end();

    if (end == 0) {
        endIt = it + 1;
    }

    if (end > 0) {
        endIt = it + (end - start + 1) + 1;
    }
    it += start;

    int i = start;
    while (it != endIt) {
        QPersistentModelIndex idx = *it;
        //     qDebug() << "removing" << idx << idx.data();

        if (m_parents.values().contains(idx)) {
            qint64 key = m_parents.key(idx);
            QPersistentModelIndex value = m_parents.take(key);
            m_pendingRemovalParents.insert(key, value);
            //       qDebug() << "take from parent" << value;
        }
        removeTree(idx);

        ++i;

        m_pendingRemovalChildIndexes[idxToRemove].append(idx);
        //     qDebug() << idxToRemove << idxToRemove.data() << idx << idx.data();

        it = toRemove.erase(it);
        //     qDebug() << (it == endIt);
        //     if (i > end)
        //       break;

        //     if (it == toRemove.end())
        //       break;
    }

    //   qDebug() << "toRemove" << toRemove;

    //   for(int i = start; (i <= end || (end == -1 && toRemove.size() > i)); )
    //   {
    //     qDebug() << i;
    //     QPersistentModelIndex idx = toRemove.takeAt(i);
    //     --end;
    //
    //     qDebug() << "removing" << idx.data();
    //
    //     if (m_parents.values().contains(idx))
    //     {
    //       QPersistentModelIndex bah = m_parents.take(m_parents.key(idx));
    // //       qDebug() << "take from parent" << bah;
    //     }
    //     removeTree(idx);
    //   }
}

void KReparentingProxyModelPrivate::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    Q_Q(KReparentingProxyModel);
    q->beginResetModel();

    return;
#if 0
//   qDebug() << parent << start << end;

    // This is really tricky.
    //
    // We could have something like:
    //
    //   A              A
    //   B              - B
    //   C    ->        - - C
    //   D              D
    //   E              - E
    //
    // And have to remove something like B to D. That would mean a remove signal for B, move E to its grandparent, remove D.

//   QHashIterator<QPersistentModelIndex, QList< QPersistentModelIndex> > it(m_childIndexes);
//   while (it.hasNext())
//   {
//     it.next();
//     qDebug() << it.key() << it.key().data();
//     qDebug() << it.value();
//   }

    const int column = 0;

    QModelIndex firstAffectedIndex = q->mapFromSource(q->sourceModel()->index(start, column, parent));
    QModelIndex lastAffectedIndex = q->mapFromSource(q->sourceModel()->index(end, column, parent));

//   qDebug() << "firstAffectedIndex" << firstAffectedIndex.data();
//   qDebug() << "lastAffectedIndex" << lastAffectedIndex.data();

    QModelIndex proxyParent = firstAffectedIndex.parent();

    Q_ASSERT(firstAffectedIndex.isValid() && lastAffectedIndex.isValid());

    Q_FOREVER {
    if (isDescendantInModel(proxyParent, lastAffectedIndex))
        {
            // They share a common ancestor.

            QModelIndex _parent = lastAffectedIndex.parent();
            QModelIndex lastAffectedAncestor = lastAffectedIndex;
//       qDebug() << "last affected ancestor" << lastAffectedAncestor.data();
            while (_parent != proxyParent) {
                lastAffectedAncestor = _parent;
                _parent = _parent.parent();
            }

            if (q->hasChildren(lastAffectedAncestor)) {
                QModelIndex next = q->index(0, 0, lastAffectedAncestor);

                QModelIndex proxySourceParent = lastAffectedAncestor;
                int startRow = next.row();
                int lastRow = q->rowCount(lastAffectedAncestor) - 1;

                QList<QPersistentModelIndex> &existingSourceIndexes = m_childIndexes[q->mapToSource(proxySourceParent)];
                QList<QPersistentModelIndex> &existingDestinationIndexes = m_childIndexes[q->mapToSource(proxyParent)];

                int destRow = lastAffectedAncestor.row() + 1;

//         qDebug() << "Move from" << lastAffectedAncestor.data() << startRow << lastRow << " To " << proxyParent.data() << destRow;
                bool allowMove = q->beginMoveRows(lastAffectedAncestor, startRow, lastRow, proxyParent, destRow);
                Q_ASSERT(allowMove);

                for (int i = startRow; i <= lastRow; ++i) {
                    QPersistentModelIndex movingIdx = existingSourceIndexes.takeAt(startRow);
                    existingDestinationIndexes.insert(destRow + (i - startRow), movingIdx);
                }

                // TODO: If source was a parent before, it might not be now.
                // dest was already a parent.

                q->endMoveRows();
            }
            PendingRemoval removal;
            removal.index = proxyParent;
            removal.start = firstAffectedIndex.row();
            removal.end = lastAffectedAncestor.row();
            removal.parentId = proxyParent.internalId();
            removal.sourceIndex = q->mapToSource(proxyParent);
            m_pendingRemovals.append(removal);

            removeTree(q->mapToSource(proxyParent), removal.start, removal.end);

//       qDebug() << "beg rem 1";
            q->beginRemoveRows(proxyParent, removal.start, removal.end);

            return;

        } else {
            QModelIndex next = getIndexBelow(firstAffectedIndex);

            proxyParent = next.parent();

            while (isDescendantInModel(proxyParent, next))
            {
                next = getIndexBelow(next);
            }
            QModelIndex _parent = next.parent();
            QModelIndex lastAffectedAncestor = next;

            while (_parent != proxyParent)
            {
                lastAffectedAncestor = _parent;
                _parent = _parent.parent();
            }

            PendingRemoval removal;
            removal.index = proxyParent;
            removal.start = firstAffectedIndex.row();
            removal.end = lastAffectedAncestor.row();
            removal.parentId = proxyParent.internalId();
            removal.sourceIndex = q->mapToSource(proxyParent);
            m_pendingRemovals.append(removal);

            removeTree(q->mapToSource(proxyParent), removal.start, removal.end);

//       qDebug() << "beg rem 1";
            q->beginRemoveRows(proxyParent, removal.start, removal.end);

            proxyParent = next.parent();
        }
    }

// //   qDebug() << proxyParent.data() << lastAffectedIndex.parent().data() << proxyParent << lastAffectedIndex.parent();
//   if (proxyParent == lastAffectedIndex.parent())
//   {
//     PendingRemoval removal;
//     removal.index = proxyParent;
//     removal.start = firstAffectedIndex.row();
//     removal.end = lastAffectedIndex.row();
//     removal.parentId = proxyParent.internalId();
//     removal.sourceIndex = q->mapToSource(proxyParent);
//     m_pendingRemovals.append(removal);
//
//     // Also need to store a removal object for each of the descendants.
//
//     removeTree(q->mapToSource(proxyParent), removal.start, removal.end);
//
// //     qDebug() << "beg rem 1";
//     q->beginRemoveRows(proxyParent, removal.start, removal.end);
//     return;
//   }
//
//   QModelIndex lastParent = lastAffectedIndex.parent();
//   while (lastParent.parent().isValid())
//   {
//     if (lastParent.parent() == proxyParent)
//     {
//       PendingRemoval removal;
//       removal.index = proxyParent;
//       removal.start = firstAffectedIndex.row();
//       removal.end = lastParent.row();
//       removal.parentId = proxyParent.internalId();
//       removal.sourceIndex = q->mapToSource(proxyParent);
//       m_pendingRemovals.append(removal);
//
// //       qDebug() << "beg rem 2";
//       q->beginRemoveRows(proxyParent, removal.start, removal.end);
//       return;
//     }
//     lastParent = lastParent.parent();
//   }
//
//   // Several blocks need to be removed from the proxy model.
//   // Divide and conquer to find them.
//
//   int proxyStart = firstAffectedIndex.row();
//   int proxyEnd = proxyStart + (end - start);
//   int processedUntil = start;
//
//   while (processedUntil <= end)
//   {
//     QModelIndex lastInParent = findLastInParent(proxyParent, proxyStart, proxyEnd);
//     qDebug() << "lastInParent" << lastInParent;
//
//     QModelIndex sourceLast = q->mapToSource(lastInParent);
//     processedUntil = sourceLast.row();
//
//     PendingRemoval removal;
//     removal.index = proxyParent;
//     removal.start = proxyStart;
//     removal.end = lastInParent.row();
//     removal.parentId = proxyParent.internalId();
//     removal.sourceIndex = q->mapToSource(proxyParent);
//     m_pendingRemovals.append(removal);
//
//     qDebug() << "beg rem 3";
//     q->beginRemoveRows(proxyParent, removal.start, removal.end);
//
//     QModelIndex proxyIndexBelow = getIndexBelow(lastInParent, q);
//
//     if (!proxyIndexBelow.isValid())
//       return;
//
//     proxyParent = proxyIndexBelow.parent();
//     proxyStart = proxyIndexBelow.row();
//   }
#endif
}

QModelIndex KReparentingProxyModelPrivate::findLastInParent(QModelIndex parent, int start, int end)
{
    Q_Q(KReparentingProxyModel);

    const int column = 0;

    if (start == end) {
        return q->index(start, column, parent);
    }

    int middle = start + (end - start / 2);

    QModelIndex sourceParent = q->mapToSource(parent);
    QModelIndex middleIndex = q->mapFromSource(q->sourceModel()->index(middle, column, sourceParent));

    if (middleIndex.parent() == parent) {
        return findLastInParent(parent, middle, end);
    } else {
        return findLastInParent(parent, start + ((middle - start) / 2), middle);
    }
}

//   qDebug() << affectedIndex << affectedIndex.data() << proxyParent;
//
//   QHash<QModelIndex, PendingRemoval> pendingRemovals;
//
//   int i = start;
//   while (i <= end)
//   {
//     affectedIndex = affectedIndex.sibling(i, column);
//
// //     affectedIndex = getIndexBelow(affectedIndex, q);
//     if (!affectedIndex.isValid())
//       break;
// //     Q_ASSERT(affectedIndex.isValid());
//
//     if (affectedIndex.parent() != proxyParent)
//     {
//       // affectedIndex.parent() must be left of proxyParent
//
//       PendingRemoval removal;
//       removal.index = proxyParent;
//       removal.start = start;
//       removal.end = i;
//       pendingRemovals.insert(proxyParent, removal);
//
//       Q_EMIT q->rowsAboutToBeRemoved(proxyParent, start, i);
//       proxyParent = affectedIndex.parent();
//
//       end -= (i - start + 1);
//       start = affectedIndex.row();
//       i = start;
//     }
//
//     ++i;
//   }

// Move younger siblings out of the way so that the rows can be removed easily
// No. It's easier to use verifyStructure afterward.

//   // Removing rows in the source model could require sending the children to their grandparents.
//
//   QHash<QModelIndex, QModelIndexList> mappings;
//   recreateMappings(parent, start, end);
//
//   QHashIterator<QModelIndex, QModelIndexList> it(mappings);
//   while (it.hasNext())
//   {
//     it.next();
//     QModelIndexList removedList = it.value();
//     PendingRemoval pendingRemoval;
//     pendingRemoval.index = it.key();
//     pendingRemoval.start = q->mapFromSource(removedList.at(0)).row();
//     pendingRemoval.end = pendingRemoval.start + removedList.size() - 1;
//     m_pendingRemovals.insert(parent, pendingRemoval);
//   }
// }

void KReparentingProxyModelPrivate::handleRemoval(const PendingRemoval &pendingRemoval)
{
    Q_UNUSED(pendingRemoval)
    //   Q_Q(KReparentingProxyModel);
    //   q->beginRemoveRows(pendingRemoval.index, pendingRemoval.start, pendingRemoval.end);
    //   m_childIndexes.remove(pendingRemoval.index);
    //   // Remove stuff from m_parents.
    //   q->endRemoveRows();
}

void KReparentingProxyModelPrivate::sourceRowsRemoved(const QModelIndex &parent, int, int)
{
    return endResetProxy();

    Q_Q(KReparentingProxyModel);

    // loop over pending removals and process each one. Then look after the last one
    // to move displaced rows to where they should be.

    int lastAffectedRow = m_pendingRemovals.last().end;
    QModelIndex lastAffectedIndex = m_pendingRemovals.last().index;

    QMutableListIterator<PendingRemoval> it(m_pendingRemovals);

    while (it.hasNext()) {
        PendingRemoval removal = it.next();
        m_pendingRemovalChildIndexes.remove(removal.sourceIndex);
        m_pendingRemovalParents.remove(parent.internalId());
        it.remove();

        Q_EMIT q->endRemoveRows();
    }
    //   qDebug() << "Remove done ##########";

    //   qDebug() << lastAffectedIndex << lastAffectedIndex.data() << lastAffectedRow;

    verifyStructure(lastAffectedIndex, lastAffectedRow - 1);
}

void KReparentingProxyModelPrivate::sourceRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex &, int)
{
    // This could be several individual moves in the proxy model, or it could be no moves at all.
    // We can get the top indexes of the moved list and move those.
    // because their children won't be moved anywhere different.

    // I could look at the indexes between start and end (proxied could be several blocks), and move them to dest.
    // Then verify structure.
    // This could lead to an illegal move.
    // If we have
    //
    // Source:        Proxy:
    // A              A
    // B              B
    // C              - C
    // D              - D
    // E              E
    //
    // then source can legally move B to between C and D, however, implemented naively the proxymodel would attempt an illegal move.
    // We must first reparent everything below destRow in the proxy to the parent of parent in this case, then perform the move, then
    // verifyStructure.
    //
    // Moving B C and D to below E would be a legal move in the proxy model.
    //
    // Children of moved indexes which are not themselves moved must be first sent to their grandparents.
    // So if B and C were moved in the source model above to below E, D would first be moved to its grandparent, then B would be moved below E,
    // then the structure would need to be verified.
    //
    // Proxy start state:     Intermediate state:   Intermediate or final state:   Possible alternative final state:
    //       A                      A                     A                              A
    //       B                      B                     E                              E
    //       - C                    - C                   D                              - D
    //       - D                    D                     B                              B
    //       E                      E                     - C                            - C

    // So, I could iterate from start to end in proxySourceParent and if the depth goes less than parent, emit a block move, then start again.

    QHash<QModelIndex, QModelIndexList> newMappings = recreateMappings(parent, start, end);
}

void KReparentingProxyModelPrivate::sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)
{
}

void KReparentingProxyModelPrivate::sourceLayoutAboutToBeChanged()
{
    Q_Q(KReparentingProxyModel);

    q->beginResetModel();
    return;
#if 0

    Q_EMIT q->layoutAboutToBeChanged();

    Q_FOREACH (QPersistentModelIndex proxyPersistentIndex, q->persistentIndexList()) {
        m_proxyIndexes << proxyPersistentIndex;
        m_layoutChangePersistentIndexes << QPersistentModelIndex(q->mapToSource(proxyPersistentIndex));
    }
#endif
}

void KReparentingProxyModelPrivate::sourceLayoutChanged()
{
    endResetProxy();
    return;
#if 0
    Q_Q(KReparentingProxyModel);

    for (int i = 0; i < m_proxyIndexes.size(); ++i) {
        q->changePersistentIndex(m_proxyIndexes.at(i), q->mapFromSource(m_layoutChangePersistentIndexes.at(i)));
    }

    m_layoutChangePersistentIndexes.clear();
    m_proxyIndexes.clear();

    Q_EMIT q->layoutChanged();
#endif
}

void KReparentingProxyModelPrivate::sourceModelAboutToBeReset()
{
    Q_Q(KReparentingProxyModel);
    q->beginResetModel();
}

void KReparentingProxyModelPrivate::endResetProxy()
{
    Q_Q(KReparentingProxyModel);

    m_parents.clear();
    m_childIndexes.clear();
    m_nextId = 0;
    m_pendingInsertions.clear();
    m_pendingRemovals.clear();
    m_pendingRemovalChildIndexes.clear();
    m_pendingRemovalParents.clear();
    //   qDebug() << q->sourceModel()->rowCount();
    QHash<QModelIndex, QModelIndexList> mappings =
        recreateMappings(QModelIndex(), 0, q->sourceModel()->rowCount() - 1, KReparentingProxyModelPrivate::MapDescendants);
    qDebug() << mappings;

    mergeDescendants(mappings, QModelIndex(), 0);
    q->endResetModel();
}

void KReparentingProxyModelPrivate::sourceModelReset()
{
    endResetProxy();
}

void KReparentingProxyModelPrivate::emitDataChangedSignals(const QModelIndex &startIndex, int maxChanged)
{
    Q_Q(KReparentingProxyModel);

    QModelIndex proxyParent = startIndex.parent();

    int numChanged = 1;

    QModelIndex lastAffectedSibling = startIndex;
    QModelIndex proxySibling = getIndexBelow(startIndex, q);

    Q_FOREVER {
        if (proxySibling.parent() != proxyParent || numChanged >= maxChanged) {
            break;
        }

        numChanged++;
        lastAffectedSibling = proxySibling;

        proxySibling = getIndexBelow(proxySibling);
    }

    Q_EMIT q->dataChanged(startIndex, lastAffectedSibling);
    if (numChanged < maxChanged) {
        emitDataChangedSignals(proxySibling, maxChanged - numChanged);
    }
}

void KReparentingProxyModelPrivate::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    Q_Q(KReparentingProxyModel);

    q->beginResetModel();
    endResetProxy();
    return;
#if 0
    QModelIndex parent = topLeft.parent();
    const int start = topLeft.row();
    const int end = bottomRight.row();
    const int column = 0;
    const int maxChanged = end - start + 1;

    // Create mappings to the end because changing data can affect structure of siblings.
    verifyStructure(parent, start);

    // mapFromSource and emit signals.

    QModelIndex proxyStartIndex = q->mapFromSource(q->sourceModel()->index(start, column, parent));

    emitDataChangedSignals(proxyStartIndex, maxChanged);
#endif
}

Qt::DropActions KReparentingProxyModel::supportedDropActions() const
{
    Q_ASSERT(sourceModel());
    return sourceModel()->supportedDropActions();
}

void KReparentingProxyModel::beginChangeRule()
{
    Q_D(KReparentingProxyModel);
    d->sourceModelAboutToBeReset();
    //   beginResetModel();
    //   d->m_childIndexes.clear();
    //   d->m_layoutChangePersistentIndexes.clear();
    //   d->m_nextId = 1;
    //   d->m_parents.clear();
    //   d->m_pendingInsertions.clear();
    //   d->m_pendingRemovalChildIndexes.clear();
    //   d->m_pendingRemovalParents.clear();
    //   d->m_pendingRemovals.clear();
    //   d->m_proxyIndexes.clear();
}

void KReparentingProxyModel::endChangeRule()
{
    Q_D(KReparentingProxyModel);
    d->endResetProxy();
    return;
}

#include "moc_kreparentingproxymodel.cpp"
