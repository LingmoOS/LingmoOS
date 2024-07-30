/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>
    SPDX-FileCopyrightText: 2016 Ableton AG <info@ableton.com>
    SPDX-FileContributor: Stephen Kelly <stephen.kelly@ableton.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kselectionproxymodel.h"

#include <QItemSelectionRange>
#include <QPointer>
#include <QStringList>

#include "kbihash_p.h"
#include "kmodelindexproxymapper.h"
#include "kvoidpointerfactory_p.h"

typedef KBiHash<QPersistentModelIndex, QModelIndex> SourceProxyIndexMapping;
typedef KBiHash<void *, QModelIndex> ParentMapping;
typedef KHash2Map<QPersistentModelIndex, int> SourceIndexProxyRowMapping;

/**
  Return true if @p idx is a descendant of one of the indexes in @p list.
  Note that this returns false if @p list contains @p idx.
*/
template<typename ModelIndex>
bool isDescendantOf(const QList<ModelIndex> &list, const QModelIndex &idx)
{
    if (!idx.isValid()) {
        return false;
    }

    if (list.contains(idx)) {
        return false;
    }

    QModelIndex parent = idx.parent();
    while (parent.isValid()) {
        if (list.contains(parent)) {
            return true;
        }
        parent = parent.parent();
    }
    return false;
}

static bool isDescendantOf(const QItemSelection &selection, const QModelIndex &descendant)
{
    if (!descendant.isValid()) {
        return false;
    }

    if (selection.contains(descendant)) {
        return false;
    }

    QModelIndex parent = descendant.parent();
    while (parent.isValid()) {
        if (selection.contains(parent)) {
            return true;
        }

        parent = parent.parent();
    }
    return false;
}

static bool isDescendantOf(const QItemSelectionRange &range, const QModelIndex &descendant)
{
    if (!descendant.isValid()) {
        return false;
    }

    if (range.contains(descendant)) {
        return false;
    }

    QModelIndex parent = descendant.parent();
    while (parent.isValid()) {
        if (range.contains(parent)) {
            return true;
        }

        parent = parent.parent();
    }
    return false;
}

static int _getRootListRow(const QList<QModelIndexList> &rootAncestors, const QModelIndex &index)
{
    QModelIndex commonParent = index;
    QModelIndex youngestAncestor;

    int firstCommonParent = -1;
    int bestParentRow = -1;
    while (commonParent.isValid()) {
        youngestAncestor = commonParent;
        commonParent = commonParent.parent();

        for (int i = 0; i < rootAncestors.size(); ++i) {
            const QModelIndexList ancestorList = rootAncestors.at(i);

            const int parentRow = ancestorList.indexOf(commonParent);

            if (parentRow < 0) {
                continue;
            }

            if (parentRow > bestParentRow) {
                firstCommonParent = i;
                bestParentRow = parentRow;
            }
        }

        if (firstCommonParent >= 0) {
            break;
        }
    }

    // If @p list is non-empty, the invalid QModelIndex() will at least be found in ancestorList.
    Q_ASSERT(firstCommonParent >= 0);

    const QModelIndexList firstAnsList = rootAncestors.at(firstCommonParent);

    const QModelIndex eldestSibling = firstAnsList.value(bestParentRow + 1);

    if (eldestSibling.isValid()) {
        // firstCommonParent is a sibling of one of the ancestors of @p index.
        // It is the first index to share a common parent with one of the ancestors of @p index.
        if (eldestSibling.row() >= youngestAncestor.row()) {
            return firstCommonParent;
        }
    }

    int siblingOffset = 1;

    // The same commonParent might be common to several root indexes.
    // If this is the last in the list, it's the only match. We instruct the model
    // to insert the new index after it ( + siblingOffset).
    if (rootAncestors.size() <= firstCommonParent + siblingOffset) {
        return firstCommonParent + siblingOffset;
    }

    // A
    // - B
    //   - C
    //   - D
    //   - E
    // F
    //
    // F is selected, then C then D. When inserting D into the model, the commonParent is B (the parent of C).
    // The next existing sibling of B is F (in the proxy model). bestParentRow will then refer to an index on
    // the level of a child of F (which doesn't exist - Boom!). If it doesn't exist, then we've already found
    // the place to insert D
    QModelIndexList ansList = rootAncestors.at(firstCommonParent + siblingOffset);
    if (ansList.size() <= bestParentRow) {
        return firstCommonParent + siblingOffset;
    }

    QModelIndex nextParent = ansList.at(bestParentRow);
    while (nextParent == commonParent) {
        if (ansList.size() < bestParentRow + 1)
        // If the list is longer, it means that at the end of it is a descendant of the new index.
        // We insert the ancestors items first in that case.
        {
            break;
        }

        const QModelIndex nextSibling = ansList.value(bestParentRow + 1);

        if (!nextSibling.isValid()) {
            continue;
        }

        if (youngestAncestor.row() <= nextSibling.row()) {
            break;
        }

        siblingOffset++;

        if (rootAncestors.size() <= firstCommonParent + siblingOffset) {
            break;
        }

        ansList = rootAncestors.at(firstCommonParent + siblingOffset);

        // In the scenario above, E is selected after D, causing this loop to be entered,
        // and requiring a similar result if the next sibling in the proxy model does not have children.
        if (ansList.size() <= bestParentRow) {
            break;
        }

        nextParent = ansList.at(bestParentRow);
    }

    return firstCommonParent + siblingOffset;
}

/**
  Determines the correct location to insert @p index into @p list.
*/
static int getRootListRow(const QList<QPersistentModelIndex> &list, const QModelIndex &index)
{
    if (!index.isValid()) {
        return -1;
    }

    if (list.isEmpty()) {
        return 0;
    }

    // What's going on?
    // Consider a tree like
    //
    // A
    // - B
    // - - C
    // - - - D
    // - E
    // - F
    // - - G
    // - - - H
    // - I
    // - - J
    // - K
    //
    // If D, E and J are already selected, and H is newly selected, we need to put H between E and J in the proxy model.
    // To figure that out, we create a list for each already selected index of its ancestors. Then,
    // we climb the ancestors of H until we reach an index with siblings which have a descendant
    // selected (F above has siblings B, E and I which have descendants which are already selected).
    // Those child indexes are traversed to find the right sibling to put F beside.
    //
    // i.e., new items are inserted in the expected location.

    QList<QModelIndexList> rootAncestors;
    for (const auto &root : list) {
        QModelIndexList ancestors;
        ancestors.append(root);
        QModelIndex parent = root.parent();
        while (parent.isValid()) {
            ancestors.prepend(parent);
            parent = parent.parent();
        }
        ancestors.prepend(QModelIndex());
        rootAncestors.append(ancestors);
    }
    return _getRootListRow(rootAncestors, index);
}

/**
  Returns a selection in which no descendants of selected indexes are also themselves selected.
  For example,
  @code
    A
    - B
    C
    D
  @endcode
  If A, B and D are selected in @p selection, the returned selection contains only A and D.
*/
static QItemSelection getRootRanges(const QItemSelection &_selection)
{
    QItemSelection rootSelection;
    QItemSelection selection = _selection;
    QList<QItemSelectionRange>::iterator it = selection.begin();
    while (it != selection.end()) {
        if (!it->topLeft().parent().isValid()) {
            rootSelection.append(*it);
            it = selection.erase(it);
        } else {
            ++it;
        }
    }

    it = selection.begin();
    while (it != selection.end()) {
        const QItemSelectionRange range = *it;
        it = selection.erase(it);

        if (isDescendantOf(rootSelection, range.topLeft()) || isDescendantOf(selection, range.topLeft())) {
            continue;
        }

        rootSelection << range;
    }
    return rootSelection;
}

/**
 */
struct RangeLessThan {
    bool operator()(const QItemSelectionRange &left, const QItemSelectionRange &right) const
    {
        if (right.model() == left.model()) {
            // parent has to be calculated, so we only do so once.
            const QModelIndex topLeftParent = left.parent();
            const QModelIndex otherTopLeftParent = right.parent();
            if (topLeftParent == otherTopLeftParent) {
                if (right.top() == left.top()) {
                    if (right.left() == left.left()) {
                        if (right.bottom() == left.bottom()) {
                            return left.right() < right.right();
                        }
                        return left.bottom() < right.bottom();
                    }
                    return left.left() < right.left();
                }
                return left.top() < right.top();
            }
            return topLeftParent < otherTopLeftParent;
        }
        return left.model() < right.model();
    }
};

static QItemSelection stableNormalizeSelection(const QItemSelection &selection)
{
    if (selection.size() <= 1) {
        return selection;
    }

    QItemSelection::const_iterator it = selection.begin();
    const QItemSelection::const_iterator end = selection.end();

    Q_ASSERT(it != end);
    QItemSelection::const_iterator scout = it + 1;

    QItemSelection result;
    while (scout != end) {
        Q_ASSERT(it != end);
        int bottom = it->bottom();
        while (scout != end && it->parent() == scout->parent() && bottom + 1 == scout->top()) {
            bottom = scout->bottom();
            ++scout;
        }
        if (bottom != it->bottom()) {
            const QModelIndex topLeft = it->topLeft();
            result << QItemSelectionRange(topLeft, topLeft.sibling(bottom, it->right()));
        }
        Q_ASSERT(it != scout);
        if (scout == end) {
            break;
        }
        if (it + 1 == scout) {
            result << *it;
        }
        it = scout;
        ++scout;
        if (scout == end) {
            result << *it;
        }
    }
    return result;
}

static QItemSelection kNormalizeSelection(QItemSelection selection)
{
    if (selection.size() <= 1) {
        return selection;
    }

    // KSelectionProxyModel has a strong assumption that
    // the views always select rows, so usually the
    // @p selection here contains ranges that span all
    // columns. However, if a QSortFilterProxyModel
    // is used too, it splits up the complete ranges into
    // one index per range. That confuses the data structures
    // held by this proxy (which keeps track of indexes in the
    // first column). As this proxy already assumes that if the
    // zeroth column is selected, then its entire row is selected,
    // we can safely remove the ranges which do not include
    // column 0 without a loss of functionality.
    QItemSelection::iterator i = selection.begin();
    while (i != selection.end()) {
        if (i->left() > 0) {
            i = selection.erase(i);
        } else {
            ++i;
        }
    }

    RangeLessThan lt;
    std::sort(selection.begin(), selection.end(), lt);
    return stableNormalizeSelection(selection);
}

class KSelectionProxyModelPrivate
{
public:
    KSelectionProxyModelPrivate(KSelectionProxyModel *model)
        : q_ptr(model)
        , m_indexMapper(nullptr)
        , m_startWithChildTrees(false)
        , m_omitChildren(false)
        , m_omitDescendants(false)
        , m_includeAllSelected(false)
        , m_rowsInserted(false)
        , m_rowsRemoved(false)
        , m_recreateFirstChildMappingOnRemoval(false)
        , m_rowsMoved(false)
        , m_resetting(false)
        , m_sourceModelResetting(false)
        , m_doubleResetting(false)
        , m_layoutChanging(false)
        , m_ignoreNextLayoutAboutToBeChanged(false)
        , m_ignoreNextLayoutChanged(false)
        , m_selectionModel(nullptr)
        , m_filterBehavior(KSelectionProxyModel::InvalidBehavior)
    {
    }

    Q_DECLARE_PUBLIC(KSelectionProxyModel)
    KSelectionProxyModel *const q_ptr;

    // A unique id is generated for each parent. It is used for the internalPointer of its children in the proxy
    // This is used to store a unique id for QModelIndexes in the proxy which have children.
    // If an index newly gets children it is added to this hash. If its last child is removed it is removed from this map.
    // If this map contains an index, that index hasChildren(). This hash is populated when new rows are inserted in the
    // source model, or a new selection is made.
    mutable ParentMapping m_parentIds;
    // This mapping maps indexes with children in the source to indexes with children in the proxy.
    // The order of indexes in this list is not relevant.
    mutable SourceProxyIndexMapping m_mappedParents;

    KVoidPointerFactory<> m_voidPointerFactory;

    /**
      Keeping Persistent indexes from this model in this model breaks in certain situations
      such as after source insert, but before calling endInsertRows in this model. In such a state,
      the persistent indexes are not updated, but the methods assume they are already up-to-date.

      Instead of using persistentindexes for proxy indexes in m_mappedParents, we maintain them ourselves with this method.

      m_mappedParents and m_parentIds are affected.

      @p parent and @p start refer to the proxy model. Any rows >= @p start will be updated.
      @p offset is the amount that affected indexes will be changed.
    */
    void updateInternalIndexes(const QModelIndex &parent, int start, int offset);

    /**
     * Updates stored indexes in the proxy. Any proxy row >= @p start is changed by @p offset.
     *
     * This is only called to update indexes in the top level of the proxy. Most commonly that is
     *
     * m_mappedParents, m_parentIds and m_mappedFirstChildren are affected.
     */
    void updateInternalTopIndexes(int start, int offset);

    void updateFirstChildMapping(const QModelIndex &parent, int offset);

    bool isFlat() const
    {
        return m_omitChildren || (m_omitDescendants && m_startWithChildTrees);
    }

    /**
     * Tries to ensure that @p parent is a mapped parent in the proxy.
     * Returns true if parent is mappable in the model, and false otherwise.
     */
    bool ensureMappable(const QModelIndex &parent) const;
    bool parentIsMappable(const QModelIndex &parent) const
    {
        return parentAlreadyMapped(parent) || m_rootIndexList.contains(parent);
    }

    /**
     * Maps @p parent to source if it is already mapped, and otherwise returns an invalid QModelIndex.
     */
    QModelIndex mapFromSource(const QModelIndex &parent) const;

    /**
      Creates mappings in m_parentIds and m_mappedParents between the source and the proxy.

      This is not recursive
    */
    void createParentMappings(const QModelIndex &parent, int start, int end) const;
    void createFirstChildMapping(const QModelIndex &parent, int proxyRow) const;
    bool firstChildAlreadyMapped(const QModelIndex &firstChild) const;
    bool parentAlreadyMapped(const QModelIndex &parent) const;
    void removeFirstChildMappings(int start, int end);
    void removeParentMappings(const QModelIndex &parent, int start, int end);

    /**
      Given a QModelIndex in the proxy, return the corresponding QModelIndex in the source.

      This method works only if the index has children in the proxy model which already has a mapping from the source.

      This means that if the proxy is a flat list, this method will always return QModelIndex(). Additionally, it means that m_mappedParents is not populated
      automatically and must be populated manually.

      No new mapping is created by this method.
    */
    QModelIndex mapParentToSource(const QModelIndex &proxyParent) const;

    /**
      Given a QModelIndex in the source model, return the corresponding QModelIndex in the proxy.

      This method works only if the index has children in the proxy model which already has a mapping from the source.

      No new mapping is created by this method.
    */
    QModelIndex mapParentFromSource(const QModelIndex &sourceParent) const;

    QModelIndex mapTopLevelToSource(int row, int column) const;
    QModelIndex mapTopLevelFromSource(const QModelIndex &sourceIndex) const;
    QModelIndex createTopLevelIndex(int row, int column) const;
    int topLevelRowCount() const;

    void *parentId(const QModelIndex &proxyParent) const
    {
        return m_parentIds.rightToLeft(proxyParent);
    }
    QModelIndex parentForId(void *id) const
    {
        return m_parentIds.leftToRight(id);
    }

    // Only populated if m_startWithChildTrees.

    mutable SourceIndexProxyRowMapping m_mappedFirstChildren;

    // Source list is the selection in the source model.
    QList<QPersistentModelIndex> m_rootIndexList;

    KModelIndexProxyMapper *m_indexMapper;

    QPair<int, int> beginRemoveRows(const QModelIndex &parent, int start, int end) const;
    QPair<int, int> beginInsertRows(const QModelIndex &parent, int start, int end) const;
    void endRemoveRows(const QModelIndex &sourceParent, int proxyStart, int proxyEnd);
    void endInsertRows(const QModelIndex &parent, int start, int end);

    void sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow);
    void sourceRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow);
    void sourceModelAboutToBeReset();
    void sourceModelReset();
    void sourceLayoutAboutToBeChanged();
    void sourceLayoutChanged();
    void emitContinuousRanges(const QModelIndex &sourceFirst, const QModelIndex &sourceLast, const QModelIndex &proxyFirst, const QModelIndex &proxyLast);
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void removeSelectionFromProxy(const QItemSelection &selection);

    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void sourceModelDestroyed();

    void resetInternalData();

    bool rootWillBeRemoved(const QItemSelection &selection, const QModelIndex &root);

    /**
      When items are inserted or removed in the m_startWithChildTrees configuration,
      this method helps find the startRow for use emitting the signals from the proxy.
    */
    int getProxyInitialRow(const QModelIndex &parent) const;

    /**
      If m_startWithChildTrees is true, this method returns the row in the proxy model to insert newIndex
      items.

      This is a special case because the items above rootListRow in the list are not in the model, but
      their children are. Those children must be counted.

      If m_startWithChildTrees is false, this method returns @p rootListRow.
    */
    int getTargetRow(int rootListRow);

    /**
      Inserts the indexes in @p list into the proxy model.
    */
    void insertSelectionIntoProxy(const QItemSelection &selection);

    bool m_startWithChildTrees;
    bool m_omitChildren;
    bool m_omitDescendants;
    bool m_includeAllSelected;
    bool m_rowsInserted;
    bool m_rowsRemoved;
    bool m_recreateFirstChildMappingOnRemoval;
    QPair<int, int> m_proxyRemoveRows;
    bool m_rowsMoved;
    bool m_resetting;
    bool m_sourceModelResetting;
    bool m_doubleResetting;
    bool m_layoutChanging;
    bool m_ignoreNextLayoutAboutToBeChanged;
    bool m_ignoreNextLayoutChanged;
    QPointer<QItemSelectionModel> m_selectionModel;

    KSelectionProxyModel::FilterBehavior m_filterBehavior;

    QList<QPersistentModelIndex> m_layoutChangePersistentIndexes;
    QModelIndexList m_proxyIndexes;

    struct PendingSelectionChange {
        PendingSelectionChange()
        {
        }
        PendingSelectionChange(const QItemSelection &selected_, const QItemSelection &deselected_)
            : selected(selected_)
            , deselected(deselected_)
        {
        }
        QItemSelection selected;
        QItemSelection deselected;
    };
    QList<PendingSelectionChange> m_pendingSelectionChanges;
    QMetaObject::Connection selectionModelModelAboutToBeResetConnection;
    QMetaObject::Connection selectionModelModelResetConnection;
};

void KSelectionProxyModelPrivate::emitContinuousRanges(const QModelIndex &sourceFirst,
                                                       const QModelIndex &sourceLast,
                                                       const QModelIndex &proxyFirst,
                                                       const QModelIndex &proxyLast)
{
    Q_Q(KSelectionProxyModel);

    Q_ASSERT(sourceFirst.model() == q->sourceModel());
    Q_ASSERT(sourceLast.model() == q->sourceModel());
    Q_ASSERT(proxyFirst.model() == q);
    Q_ASSERT(proxyLast.model() == q);

    const int proxyRangeSize = proxyLast.row() - proxyFirst.row();
    const int sourceRangeSize = sourceLast.row() - sourceFirst.row();

    if (proxyRangeSize == sourceRangeSize) {
        Q_EMIT q->dataChanged(proxyFirst, proxyLast);
        return;
    }

    // TODO: Loop to skip descendant ranges.
    //     int lastRow;
    //
    //     const QModelIndex sourceHalfWay = sourceFirst.sibling(sourceFirst.row() + (sourceRangeSize / 2));
    //     const QModelIndex proxyHalfWay = proxyFirst.sibling(proxyFirst.row() + (proxyRangeSize / 2));
    //     const QModelIndex mappedSourceHalfway = q->mapToSource(proxyHalfWay);
    //
    //     const int halfProxyRange = mappedSourceHalfway.row() - proxyFirst.row();
    //     const int halfSourceRange = sourceHalfWay.row() - sourceFirst.row();
    //
    //     if (proxyRangeSize == sourceRangeSize)
    //     {
    //         Q_EMIT q->dataChanged(proxyFirst, proxyLast.sibling(proxyFirst.row() + proxyRangeSize, proxyLast.column()));
    //         return;
    //     }

    Q_EMIT q->dataChanged(proxyFirst, proxyLast);
}

void KSelectionProxyModelPrivate::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_Q(KSelectionProxyModel);

    Q_ASSERT(topLeft.model() == q->sourceModel());
    Q_ASSERT(bottomRight.model() == q->sourceModel());

    const QModelIndex sourceRangeParent = topLeft.parent();
    if (!sourceRangeParent.isValid() && m_startWithChildTrees && !m_rootIndexList.contains(sourceRangeParent)) {
        return;
    }

    const QModelIndex proxyTopLeft = q->mapFromSource(topLeft);
    const QModelIndex proxyBottomRight = q->mapFromSource(bottomRight);

    const QModelIndex proxyRangeParent = proxyTopLeft.parent();

    if (!m_omitChildren && m_omitDescendants && m_startWithChildTrees && m_includeAllSelected) {
        // ChildrenOfExactSelection
        if (proxyTopLeft.isValid()) {
            emitContinuousRanges(topLeft, bottomRight, proxyTopLeft, proxyBottomRight);
        }
        return;
    }

    if ((m_omitChildren && !m_startWithChildTrees && m_includeAllSelected) || (!proxyRangeParent.isValid() && !m_startWithChildTrees)) {
        // Exact selection and SubTreeRoots and SubTrees in top level
        // Emit continuous ranges.
        QList<int> changedRows;
        for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
            const QModelIndex index = q->sourceModel()->index(row, topLeft.column(), topLeft.parent());
            const int idx = m_rootIndexList.indexOf(index);
            if (idx != -1) {
                changedRows.append(idx);
            }
        }
        if (changedRows.isEmpty()) {
            return;
        }
        int first = changedRows.first();
        int previous = first;
        QList<int>::const_iterator it = changedRows.constBegin();
        const QList<int>::const_iterator end = changedRows.constEnd();
        for (; it != end; ++it) {
            if (*it == previous + 1) {
                ++previous;
            } else {
                const QModelIndex _top = q->index(first, topLeft.column());
                const QModelIndex _bottom = q->index(previous, bottomRight.column());
                Q_EMIT q->dataChanged(_top, _bottom);
                previous = first = *it;
            }
        }
        if (first != previous) {
            const QModelIndex _top = q->index(first, topLeft.column());
            const QModelIndex _bottom = q->index(previous, bottomRight.column());
            Q_EMIT q->dataChanged(_top, _bottom);
        }
        return;
    }
    if (proxyRangeParent.isValid()) {
        if (m_omitChildren && !m_startWithChildTrees && !m_includeAllSelected)
        // SubTreeRoots
        {
            return;
        }
        if (!proxyTopLeft.isValid()) {
            return;
        }
        // SubTrees and SubTreesWithoutRoots
        Q_EMIT q->dataChanged(proxyTopLeft, proxyBottomRight);
        return;
    }

    if (m_startWithChildTrees && !m_omitChildren && !m_includeAllSelected && !m_omitDescendants) {
        // SubTreesWithoutRoots
        if (proxyTopLeft.isValid()) {
            Q_EMIT q->dataChanged(proxyTopLeft, proxyBottomRight);
        }
        return;
    }
}

void KSelectionProxyModelPrivate::sourceLayoutAboutToBeChanged()
{
    Q_Q(KSelectionProxyModel);

    if (m_ignoreNextLayoutAboutToBeChanged) {
        m_ignoreNextLayoutAboutToBeChanged = false;
        return;
    }

    if (m_rootIndexList.isEmpty()) {
        return;
    }

    Q_EMIT q->layoutAboutToBeChanged();

    QItemSelection selection;
    for (const auto &rootIndex : std::as_const(m_rootIndexList)) {
        // This will be optimized later.
        Q_EMIT q->rootIndexAboutToBeRemoved(rootIndex, KSelectionProxyModel::QPrivateSignal());
        selection.append(QItemSelectionRange(rootIndex, rootIndex));
    }

    selection = kNormalizeSelection(selection);
    Q_EMIT q->rootSelectionAboutToBeRemoved(selection, KSelectionProxyModel::QPrivateSignal());

    QPersistentModelIndex srcPersistentIndex;
    const auto lst = q->persistentIndexList();
    for (const QModelIndex &proxyPersistentIndex : lst) {
        m_proxyIndexes << proxyPersistentIndex;
        Q_ASSERT(proxyPersistentIndex.isValid());
        srcPersistentIndex = q->mapToSource(proxyPersistentIndex);
        Q_ASSERT(srcPersistentIndex.isValid());
        m_layoutChangePersistentIndexes << srcPersistentIndex;
    }

    m_rootIndexList.clear();
}

void KSelectionProxyModelPrivate::sourceLayoutChanged()
{
    Q_Q(KSelectionProxyModel);

    if (m_ignoreNextLayoutChanged) {
        m_ignoreNextLayoutChanged = false;
        return;
    }

    if (!m_selectionModel || !m_selectionModel->hasSelection()) {
        return;
    }

    // Handling this signal is slow.
    // The problem is that anything can happen between emissions of layoutAboutToBeChanged and layoutChanged.
    // We can't assume anything is the same about the structure anymore. items have been sorted, items which
    // were parents before are now not, items which were not parents before now are, items which used to be the
    // first child are now the Nth child and items which used to be the Nth child are now the first child.
    // We effectively can't update our mapping because we don't have enough information to update everything.
    // The only way we would have is if we take a persistent index of the entire source model
    // on sourceLayoutAboutToBeChanged and then examine it here. That would be far too expensive.
    // Instead we just have to clear the entire mapping and recreate it.

    m_rootIndexList.clear();
    m_mappedFirstChildren.clear();
    m_mappedParents.clear();
    m_parentIds.clear();

    m_resetting = true;
    m_layoutChanging = true;
    selectionChanged(m_selectionModel->selection(), QItemSelection());
    m_resetting = false;
    m_layoutChanging = false;

    for (int i = 0; i < m_proxyIndexes.size(); ++i) {
        q->changePersistentIndex(m_proxyIndexes.at(i), q->mapFromSource(m_layoutChangePersistentIndexes.at(i)));
    }

    m_layoutChangePersistentIndexes.clear();
    m_proxyIndexes.clear();

    Q_EMIT q->layoutChanged();
}

void KSelectionProxyModelPrivate::resetInternalData()
{
    m_rootIndexList.clear();
    m_layoutChangePersistentIndexes.clear();
    m_proxyIndexes.clear();
    m_mappedParents.clear();
    m_parentIds.clear();
    m_mappedFirstChildren.clear();
    m_voidPointerFactory.clear();
}

void KSelectionProxyModelPrivate::sourceModelDestroyed()
{
    // There is very little we can do here.
    resetInternalData();
    m_resetting = false;
    m_sourceModelResetting = false;
}

void KSelectionProxyModelPrivate::sourceModelAboutToBeReset()
{
    Q_Q(KSelectionProxyModel);

    // We might be resetting as a result of the selection source model resetting.
    // If so we don't want to emit
    // sourceModelAboutToBeReset
    // sourceModelAboutToBeReset
    // sourceModelReset
    // sourceModelReset
    // So we ensure that we just emit one.
    if (m_resetting) {
        // If both the source model and the selection source model are reset,
        // We want to begin our reset before the first one is reset and end
        // it after the second one is reset.
        m_doubleResetting = true;
        return;
    }

    q->beginResetModel();
    m_resetting = true;
    m_sourceModelResetting = true;
}

void KSelectionProxyModelPrivate::sourceModelReset()
{
    Q_Q(KSelectionProxyModel);

    if (m_doubleResetting) {
        m_doubleResetting = false;
        return;
    }

    resetInternalData();
    m_sourceModelResetting = false;
    m_resetting = false;
    selectionChanged(m_selectionModel->selection(), QItemSelection());
    q->endResetModel();
}

int KSelectionProxyModelPrivate::getProxyInitialRow(const QModelIndex &parent) const
{
    Q_ASSERT(m_rootIndexList.contains(parent));

    // The difficulty here is that parent and parent.parent() might both be in the m_rootIndexList.

    // - A
    // - B
    // - - C
    // - - D
    // - - - E

    // Consider that B and D are selected. The proxy model is:

    // - C
    // - D
    // - E

    // Then D gets a new child at 0. In that case we require adding F between D and E.

    // Consider instead that D gets removed. Then @p parent will be B.

    Q_Q(const KSelectionProxyModel);

    Q_ASSERT(parent.model() == q->sourceModel());

    int parentPosition = m_rootIndexList.indexOf(parent);

    QModelIndex parentAbove;

    // If parentPosition == 0, then parent.parent() is not also in the model. (ordering is preserved)
    while (parentPosition > 0) {
        parentPosition--;

        parentAbove = m_rootIndexList.at(parentPosition);
        Q_ASSERT(parentAbove.isValid());

        int rows = q->sourceModel()->rowCount(parentAbove);
        if (rows > 0) {
            QModelIndex sourceIndexAbove = q->sourceModel()->index(rows - 1, 0, parentAbove);
            Q_ASSERT(sourceIndexAbove.isValid());
            QModelIndex proxyChildAbove = mapFromSource(sourceIndexAbove);
            Q_ASSERT(proxyChildAbove.isValid());
            return proxyChildAbove.row() + 1;
        }
    }
    return 0;
}

void KSelectionProxyModelPrivate::updateFirstChildMapping(const QModelIndex &parent, int offset)
{
    Q_Q(KSelectionProxyModel);

    Q_ASSERT(parent.isValid() ? parent.model() == q->sourceModel() : true);

    static const int column = 0;
    static const int row = 0;

    const QPersistentModelIndex srcIndex = q->sourceModel()->index(row, column, parent);

    const QPersistentModelIndex previousFirstChild = q->sourceModel()->index(offset, column, parent);

    SourceIndexProxyRowMapping::left_iterator it = m_mappedFirstChildren.findLeft(previousFirstChild);
    if (it == m_mappedFirstChildren.leftEnd()) {
        return;
    }

    Q_ASSERT(srcIndex.isValid());
    const int proxyRow = it.value();
    Q_ASSERT(proxyRow >= 0);

    m_mappedFirstChildren.eraseLeft(it);

    // The proxy row in the mapping has already been updated by the offset in updateInternalTopIndexes
    // so we restore it by applying the reverse.
    m_mappedFirstChildren.insert(srcIndex, proxyRow - offset);
}

QPair<int, int> KSelectionProxyModelPrivate::beginInsertRows(const QModelIndex &parent, int start, int end) const
{
    const QModelIndex proxyParent = mapFromSource(parent);

    if (!proxyParent.isValid()) {
        if (!m_startWithChildTrees) {
            return qMakePair(-1, -1);
        }

        if (!m_rootIndexList.contains(parent)) {
            return qMakePair(-1, -1);
        }
    }

    if (!m_startWithChildTrees) {
        // SubTrees
        if (proxyParent.isValid()) {
            return qMakePair(start, end);
        }
        return qMakePair(-1, -1);
    }

    if (!m_includeAllSelected && proxyParent.isValid()) {
        // SubTreesWithoutRoots deeper than topLevel
        return qMakePair(start, end);
    }

    if (!m_rootIndexList.contains(parent)) {
        return qMakePair(-1, -1);
    }

    const int proxyStartRow = getProxyInitialRow(parent) + start;
    return qMakePair(proxyStartRow, proxyStartRow + (end - start));
}

void KSelectionProxyModelPrivate::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_Q(KSelectionProxyModel);

    Q_ASSERT(parent.isValid() ? parent.model() == q->sourceModel() : true);

    if (!m_selectionModel || !m_selectionModel->hasSelection()) {
        return;
    }

    if (m_omitChildren)
    // ExactSelection and SubTreeRoots
    {
        return;
    }

    // topLevel insertions can be ignored because topLevel items would need to be selected to affect the proxy.
    if (!parent.isValid()) {
        return;
    }

    QPair<int, int> pair = beginInsertRows(parent, start, end);
    if (pair.first == -1) {
        return;
    }

    const QModelIndex proxyParent = m_startWithChildTrees ? QModelIndex() : mapFromSource(parent);

    m_rowsInserted = true;
    q->beginInsertRows(proxyParent, pair.first, pair.second);
}

void KSelectionProxyModelPrivate::endInsertRows(const QModelIndex &parent, int start, int end)
{
    Q_Q(const KSelectionProxyModel);
    const QModelIndex proxyParent = mapFromSource(parent);
    const int offset = end - start + 1;

    const bool isNewParent = (q->sourceModel()->rowCount(parent) == offset);

    if (m_startWithChildTrees && m_rootIndexList.contains(parent)) {
        const int proxyInitialRow = getProxyInitialRow(parent);
        Q_ASSERT(proxyInitialRow >= 0);
        const int proxyStartRow = proxyInitialRow + start;

        updateInternalTopIndexes(proxyStartRow, offset);
        if (isNewParent) {
            createFirstChildMapping(parent, proxyStartRow);
        } else if (start == 0)
        // We already have a first child mapping, but what we have mapped is not the first child anymore
        // so we need to update it.
        {
            updateFirstChildMapping(parent, end + 1);
        }
    } else {
        Q_ASSERT(proxyParent.isValid());
        if (!isNewParent) {
            updateInternalIndexes(proxyParent, start, offset);
        } else {
            createParentMappings(parent.parent(), parent.row(), parent.row());
        }
    }
    createParentMappings(parent, start, end);
}

void KSelectionProxyModelPrivate::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_Q(KSelectionProxyModel);

    Q_ASSERT(parent.isValid() ? parent.model() == q->sourceModel() : true);

    if (!m_rowsInserted) {
        return;
    }
    m_rowsInserted = false;
    endInsertRows(parent, start, end);
    q->endInsertRows();
    for (const PendingSelectionChange &pendingChange : std::as_const(m_pendingSelectionChanges)) {
        selectionChanged(pendingChange.selected, pendingChange.deselected);
    }
    m_pendingSelectionChanges.clear();
}

static bool rootWillBeRemovedFrom(const QModelIndex &ancestor, int start, int end, const QModelIndex &root)
{
    Q_ASSERT(root.isValid());

    auto parent = root;
    while (parent.isValid()) {
        auto prev = parent;
        parent = parent.parent();
        if (parent == ancestor) {
            return (prev.row() <= end && prev.row() >= start);
        }
    }
    return false;
}

bool KSelectionProxyModelPrivate::rootWillBeRemoved(const QItemSelection &selection, const QModelIndex &root)
{
    Q_ASSERT(root.isValid());

    for (auto &r : selection) {
        if (m_includeAllSelected) {
            if (r.parent() == root.parent() && root.row() <= r.bottom() && root.row() >= r.top()) {
                return true;
            }
        } else {
            if (rootWillBeRemovedFrom(r.parent(), r.top(), r.bottom(), root)) {
                return true;
            }
        }
    }
    return false;
}

QPair<int, int> KSelectionProxyModelPrivate::beginRemoveRows(const QModelIndex &parent, int start, int end) const
{
    Q_Q(const KSelectionProxyModel);

    if (!m_includeAllSelected && !m_omitChildren) {
        // SubTrees and SubTreesWithoutRoots
        const QModelIndex proxyParent = mapParentFromSource(parent);
        if (proxyParent.isValid()) {
            return qMakePair(start, end);
        }
    }

    if (m_startWithChildTrees && m_rootIndexList.contains(parent)) {
        const int proxyStartRow = getProxyInitialRow(parent) + start;
        const int proxyEndRow = proxyStartRow + (end - start);
        return qMakePair(proxyStartRow, proxyEndRow);
    }

    QList<QPersistentModelIndex>::const_iterator rootIt = m_rootIndexList.constBegin();
    const QList<QPersistentModelIndex>::const_iterator rootEnd = m_rootIndexList.constEnd();
    int proxyStartRemove = 0;

    for (; rootIt != rootEnd; ++rootIt) {
        if (rootWillBeRemovedFrom(parent, start, end, *rootIt)) {
            break;
        } else {
            if (m_startWithChildTrees) {
                proxyStartRemove += q->sourceModel()->rowCount(*rootIt);
            } else {
                ++proxyStartRemove;
            }
        }
    }

    if (rootIt == rootEnd) {
        return qMakePair(-1, -1);
    }

    int proxyEndRemove = proxyStartRemove;

    for (; rootIt != rootEnd; ++rootIt) {
        if (!rootWillBeRemovedFrom(parent, start, end, *rootIt)) {
            break;
        }
        if (m_startWithChildTrees) {
            proxyEndRemove += q->sourceModel()->rowCount(*rootIt);
        } else {
            ++proxyEndRemove;
        }
    }

    --proxyEndRemove;
    if (proxyEndRemove >= proxyStartRemove) {
        return qMakePair(proxyStartRemove, proxyEndRemove);
    }
    return qMakePair(-1, -1);
}

void KSelectionProxyModelPrivate::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(KSelectionProxyModel);

    Q_ASSERT(parent.isValid() ? parent.model() == q->sourceModel() : true);

    if (!m_selectionModel || !m_selectionModel->hasSelection()) {
        return;
    }

    QPair<int, int> pair = beginRemoveRows(parent, start, end);
    if (pair.first == -1) {
        return;
    }

    const QModelIndex proxyParent = mapParentFromSource(parent);

    m_rowsRemoved = true;
    m_proxyRemoveRows = pair;
    m_recreateFirstChildMappingOnRemoval = m_mappedFirstChildren.leftContains(q->sourceModel()->index(start, 0, parent));
    q->beginRemoveRows(proxyParent, pair.first, pair.second);
}

void KSelectionProxyModelPrivate::endRemoveRows(const QModelIndex &sourceParent, int proxyStart, int proxyEnd)
{
    const QModelIndex proxyParent = mapParentFromSource(sourceParent);

    // We need to make sure to remove entries from the mappings before updating internal indexes.

    // - A
    // - - B
    // - C
    // - - D

    // If A and C are selected, B and D are in the proxy. B maps to row 0 and D maps to row 1.
    // If B is then deleted leaving only D in the proxy, D needs to be updated to be a mapping
    // to row 0 instead of row 1. If that is done before removing the mapping for B, then the mapping
    // for D would overwrite the mapping for B and then the code for removing mappings would incorrectly
    // remove D.
    // So we first remove B and then update D.

    {
        SourceProxyIndexMapping::right_iterator it = m_mappedParents.rightBegin();

        while (it != m_mappedParents.rightEnd()) {
            if (!it.value().isValid()) {
                m_parentIds.removeRight(it.key());
                it = m_mappedParents.eraseRight(it);
            } else {
                ++it;
            }
        }
    }

    {
        // Depending on what is selected at the time, a single removal in the source could invalidate
        // many mapped first child items at once.

        // - A
        // - B
        // - - C
        // - - D
        // - - - E
        // - - - F
        // - - - - G
        // - - - - H

        // If D and F are selected, the proxy contains E, F, G, H. If B is then deleted E to H will
        // be removed, including both first child mappings at E and G.

        if (!proxyParent.isValid()) {
            removeFirstChildMappings(proxyStart, proxyEnd);
        }
    }

    if (proxyParent.isValid()) {
        updateInternalIndexes(proxyParent, proxyEnd + 1, -1 * (proxyEnd - proxyStart + 1));
    } else {
        updateInternalTopIndexes(proxyEnd + 1, -1 * (proxyEnd - proxyStart + 1));
    }

    QList<QPersistentModelIndex>::iterator rootIt = m_rootIndexList.begin();
    while (rootIt != m_rootIndexList.end()) {
        if (!rootIt->isValid()) {
            rootIt = m_rootIndexList.erase(rootIt);
        } else {
            ++rootIt;
        }
    }
}

void KSelectionProxyModelPrivate::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(KSelectionProxyModel);
    Q_UNUSED(end)
    Q_UNUSED(start)

    Q_ASSERT(parent.isValid() ? parent.model() == q->sourceModel() : true);

    if (!m_selectionModel) {
        return;
    }

    if (!m_rowsRemoved) {
        return;
    }
    m_rowsRemoved = false;

    Q_ASSERT(m_proxyRemoveRows.first >= 0);
    Q_ASSERT(m_proxyRemoveRows.second >= 0);
    endRemoveRows(parent, m_proxyRemoveRows.first, m_proxyRemoveRows.second);
    if (m_recreateFirstChildMappingOnRemoval && q->sourceModel()->hasChildren(parent))
    // The private endRemoveRows call might remove the first child mapping for parent, so
    // we create it again in that case.
    {
        createFirstChildMapping(parent, m_proxyRemoveRows.first);
    }
    m_recreateFirstChildMappingOnRemoval = false;

    m_proxyRemoveRows = qMakePair(-1, -1);
    q->endRemoveRows();
}

void KSelectionProxyModelPrivate::sourceRowsAboutToBeMoved(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow)
{
    Q_UNUSED(srcParent)
    Q_UNUSED(srcStart)
    Q_UNUSED(srcEnd)
    Q_UNUSED(destParent)
    Q_UNUSED(destRow)
    // we cheat and just act like the layout changed; this might benefit from some
    // optimization
    sourceLayoutAboutToBeChanged();
}

void KSelectionProxyModelPrivate::sourceRowsMoved(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow)
{
    Q_UNUSED(srcParent)
    Q_UNUSED(srcStart)
    Q_UNUSED(srcEnd)
    Q_UNUSED(destParent)
    Q_UNUSED(destRow)
    // we cheat and just act like the layout changed; this might benefit from some
    // optimization
    sourceLayoutChanged();
}

QModelIndex KSelectionProxyModelPrivate::mapParentToSource(const QModelIndex &proxyParent) const
{
    return m_mappedParents.rightToLeft(proxyParent);
}

QModelIndex KSelectionProxyModelPrivate::mapParentFromSource(const QModelIndex &sourceParent) const
{
    return m_mappedParents.leftToRight(sourceParent);
}

static bool
indexIsValid(bool startWithChildTrees, int row, const QList<QPersistentModelIndex> &rootIndexList, const SourceIndexProxyRowMapping &mappedFirstChildren)
{
    if (!startWithChildTrees) {
        Q_ASSERT(rootIndexList.size() > row);
    } else {
        Q_ASSERT(!mappedFirstChildren.isEmpty());

        SourceIndexProxyRowMapping::right_const_iterator result = std::prev(mappedFirstChildren.rightUpperBound(row));

        Q_ASSERT(result != mappedFirstChildren.rightEnd());
        const int proxyFirstRow = result.key();
        const QModelIndex sourceFirstChild = result.value();
        Q_ASSERT(proxyFirstRow >= 0);
        Q_ASSERT(sourceFirstChild.isValid());
        Q_ASSERT(sourceFirstChild.parent().isValid());
        Q_ASSERT(row <= proxyFirstRow + sourceFirstChild.model()->rowCount(sourceFirstChild.parent()));
    }
    return true;
}

QModelIndex KSelectionProxyModelPrivate::createTopLevelIndex(int row, int column) const
{
    Q_Q(const KSelectionProxyModel);

    Q_ASSERT(indexIsValid(m_startWithChildTrees, row, m_rootIndexList, m_mappedFirstChildren));
    return q->createIndex(row, column);
}

QModelIndex KSelectionProxyModelPrivate::mapTopLevelFromSource(const QModelIndex &sourceIndex) const
{
    Q_Q(const KSelectionProxyModel);

    const QModelIndex sourceParent = sourceIndex.parent();
    const int row = m_rootIndexList.indexOf(sourceIndex);
    if (row == -1) {
        return QModelIndex();
    }

    if (!m_startWithChildTrees) {
        Q_ASSERT(m_rootIndexList.size() > row);
        return q->createIndex(row, sourceIndex.column());
    }
    if (!m_rootIndexList.contains(sourceParent)) {
        return QModelIndex();
    }

    const QModelIndex firstChild = q->sourceModel()->index(0, 0, sourceParent);
    const int firstProxyRow = m_mappedFirstChildren.leftToRight(firstChild);

    return q->createIndex(firstProxyRow + sourceIndex.row(), sourceIndex.column());
}

QModelIndex KSelectionProxyModelPrivate::mapFromSource(const QModelIndex &sourceIndex) const
{
    Q_Q(const KSelectionProxyModel);

    const QModelIndex maybeMapped = mapParentFromSource(sourceIndex);
    if (maybeMapped.isValid()) {
        //     Q_ASSERT((!d->m_startWithChildTrees && d->m_rootIndexList.contains(maybeMapped)) ? maybeMapped.row() < 0 : true );
        return maybeMapped;
    }
    const QModelIndex sourceParent = sourceIndex.parent();

    const QModelIndex proxyParent = mapParentFromSource(sourceParent);
    if (proxyParent.isValid()) {
        void *const parentId = m_parentIds.rightToLeft(proxyParent);
        static const int column = 0;
        return q->createIndex(sourceIndex.row(), column, parentId);
    }

    const QModelIndex firstChild = q->sourceModel()->index(0, 0, sourceParent);

    if (m_mappedFirstChildren.leftContains(firstChild)) {
        const int firstProxyRow = m_mappedFirstChildren.leftToRight(firstChild);
        return q->createIndex(firstProxyRow + sourceIndex.row(), sourceIndex.column());
    }
    return mapTopLevelFromSource(sourceIndex);
}

int KSelectionProxyModelPrivate::topLevelRowCount() const
{
    Q_Q(const KSelectionProxyModel);

    if (!m_startWithChildTrees) {
        return m_rootIndexList.size();
    }

    if (m_mappedFirstChildren.isEmpty()) {
        return 0;
    }

    const SourceIndexProxyRowMapping::right_const_iterator result = std::prev(m_mappedFirstChildren.rightConstEnd());

    const int proxyFirstRow = result.key();
    const QModelIndex sourceFirstChild = result.value();
    Q_ASSERT(sourceFirstChild.isValid());
    const QModelIndex sourceParent = sourceFirstChild.parent();
    Q_ASSERT(sourceParent.isValid());
    return q->sourceModel()->rowCount(sourceParent) + proxyFirstRow;
}

bool KSelectionProxyModelPrivate::ensureMappable(const QModelIndex &parent) const
{
    Q_Q(const KSelectionProxyModel);

    if (isFlat()) {
        return true;
    }

    if (parentIsMappable(parent)) {
        return true;
    }

    QModelIndex ancestor = parent.parent();
    QModelIndexList ancestorList;
    while (ancestor.isValid()) {
        if (parentIsMappable(ancestor)) {
            break;
        } else {
            ancestorList.prepend(ancestor);
        }

        ancestor = ancestor.parent();
    }

    if (!ancestor.isValid())
    // @p parent is not a descendant of m_rootIndexList.
    {
        return false;
    }

    // sourceIndex can be mapped to the proxy. We just need to create mappings for its ancestors first.
    for (int i = 0; i < ancestorList.size(); ++i) {
        const QModelIndex existingAncestor = mapParentFromSource(ancestor);
        Q_ASSERT(existingAncestor.isValid());

        void *const ansId = m_parentIds.rightToLeft(existingAncestor);
        const QModelIndex newSourceParent = ancestorList.at(i);
        const QModelIndex newProxyParent = q->createIndex(newSourceParent.row(), newSourceParent.column(), ansId);

        void *const newId = m_voidPointerFactory.createPointer();
        m_parentIds.insert(newId, newProxyParent);
        m_mappedParents.insert(QPersistentModelIndex(newSourceParent), newProxyParent);
        ancestor = newSourceParent;
    }
    return true;
}

void KSelectionProxyModelPrivate::updateInternalTopIndexes(int start, int offset)
{
    updateInternalIndexes(QModelIndex(), start, offset);

    QHash<QPersistentModelIndex, int> updates;
    {
        SourceIndexProxyRowMapping::right_iterator it = m_mappedFirstChildren.rightLowerBound(start);
        const SourceIndexProxyRowMapping::right_iterator end = m_mappedFirstChildren.rightEnd();

        for (; it != end; ++it) {
            updates.insert(*it, it.key() + offset);
        }
    }
    {
        QHash<QPersistentModelIndex, int>::const_iterator it = updates.constBegin();
        const QHash<QPersistentModelIndex, int>::const_iterator end = updates.constEnd();

        for (; it != end; ++it) {
            m_mappedFirstChildren.insert(it.key(), it.value());
        }
    }
}

void KSelectionProxyModelPrivate::updateInternalIndexes(const QModelIndex &parent, int start, int offset)
{
    Q_Q(KSelectionProxyModel);

    Q_ASSERT(start + offset >= 0);
    Q_ASSERT(parent.isValid() ? parent.model() == q : true);

    if (isFlat()) {
        return;
    }

    SourceProxyIndexMapping::left_iterator mappedParentIt = m_mappedParents.leftBegin();

    QHash<void *, QModelIndex> updatedParentIds;
    QHash<QPersistentModelIndex, QModelIndex> updatedParents;

    for (; mappedParentIt != m_mappedParents.leftEnd(); ++mappedParentIt) {
        const QModelIndex proxyIndex = mappedParentIt.value();
        Q_ASSERT(proxyIndex.isValid());

        if (proxyIndex.row() < start) {
            continue;
        }

        const QModelIndex proxyParent = proxyIndex.parent();

        if (parent.isValid()) {
            if (proxyParent != parent) {
                continue;
            }
        } else {
            if (proxyParent.isValid()) {
                continue;
            }
        }
        Q_ASSERT(m_parentIds.rightContains(proxyIndex));
        void *const key = m_parentIds.rightToLeft(proxyIndex);

        const QModelIndex newIndex = q->createIndex(proxyIndex.row() + offset, proxyIndex.column(), proxyIndex.internalPointer());

        Q_ASSERT(newIndex.isValid());

        updatedParentIds.insert(key, newIndex);
        updatedParents.insert(mappedParentIt.key(), newIndex);
    }

    {
        QHash<QPersistentModelIndex, QModelIndex>::const_iterator it = updatedParents.constBegin();
        const QHash<QPersistentModelIndex, QModelIndex>::const_iterator end = updatedParents.constEnd();
        for (; it != end; ++it) {
            m_mappedParents.insert(it.key(), it.value());
        }
    }

    {
        QHash<void *, QModelIndex>::const_iterator it = updatedParentIds.constBegin();
        const QHash<void *, QModelIndex>::const_iterator end = updatedParentIds.constEnd();
        for (; it != end; ++it) {
            m_parentIds.insert(it.key(), it.value());
        }
    }
}

bool KSelectionProxyModelPrivate::parentAlreadyMapped(const QModelIndex &parent) const
{
    Q_Q(const KSelectionProxyModel);
    Q_UNUSED(q) // except in Q_ASSERT
    Q_ASSERT(parent.model() == q->sourceModel());
    return m_mappedParents.leftContains(parent);
}

bool KSelectionProxyModelPrivate::firstChildAlreadyMapped(const QModelIndex &firstChild) const
{
    Q_Q(const KSelectionProxyModel);
    Q_UNUSED(q) // except in Q_ASSERT
    Q_ASSERT(firstChild.model() == q->sourceModel());
    return m_mappedFirstChildren.leftContains(firstChild);
}

void KSelectionProxyModelPrivate::createFirstChildMapping(const QModelIndex &parent, int proxyRow) const
{
    Q_Q(const KSelectionProxyModel);

    Q_ASSERT(parent.isValid() ? parent.model() == q->sourceModel() : true);

    static const int column = 0;
    static const int row = 0;

    const QPersistentModelIndex srcIndex = q->sourceModel()->index(row, column, parent);

    if (firstChildAlreadyMapped(srcIndex)) {
        return;
    }

    Q_ASSERT(srcIndex.isValid());
    m_mappedFirstChildren.insert(srcIndex, proxyRow);
}

void KSelectionProxyModelPrivate::createParentMappings(const QModelIndex &parent, int start, int end) const
{
    if (isFlat()) {
        return;
    }

    Q_Q(const KSelectionProxyModel);

    Q_ASSERT(parent.isValid() ? parent.model() == q->sourceModel() : true);

    static const int column = 0;

    for (int row = start; row <= end; ++row) {
        const QModelIndex srcIndex = q->sourceModel()->index(row, column, parent);
        Q_ASSERT(srcIndex.isValid());
        if (!q->sourceModel()->hasChildren(srcIndex) || parentAlreadyMapped(srcIndex)) {
            continue;
        }

        const QModelIndex proxyIndex = mapFromSource(srcIndex);
        if (!proxyIndex.isValid()) {
            return; // If one of them is not mapped, its siblings won't be either
        }

        void *const newId = m_voidPointerFactory.createPointer();
        m_parentIds.insert(newId, proxyIndex);
        Q_ASSERT(srcIndex.isValid());
        m_mappedParents.insert(QPersistentModelIndex(srcIndex), proxyIndex);
    }
}

void KSelectionProxyModelPrivate::removeFirstChildMappings(int start, int end)
{
    SourceIndexProxyRowMapping::right_iterator it = m_mappedFirstChildren.rightLowerBound(start);
    const SourceIndexProxyRowMapping::right_iterator endIt = m_mappedFirstChildren.rightUpperBound(end);
    while (it != endIt) {
        it = m_mappedFirstChildren.eraseRight(it);
    }
}

void KSelectionProxyModelPrivate::removeParentMappings(const QModelIndex &parent, int start, int end)
{
    Q_Q(KSelectionProxyModel);

    Q_ASSERT(parent.isValid() ? parent.model() == q : true);

    // collect all removals first, as executing them recursively will invalidate our iterators
    struct RemovalInfo {
        QPersistentModelIndex idx;
        QModelIndex sourceIdx;
    };
    std::vector<RemovalInfo> removals;
    removals.reserve(end - start + 1);
    for (auto it = m_mappedParents.rightBegin(); it != m_mappedParents.rightEnd(); ++it) {
        if (it.key().row() >= start && it.key().row() <= end) {
            const QModelIndex sourceParent = it.value();
            const QModelIndex proxyGrandParent = mapParentFromSource(sourceParent.parent());
            if (proxyGrandParent == parent) {
                removals.push_back({it.key(), it.value()});
            }
        }
    }

    // execute the removals
    const bool flatList = isFlat();
    for (const auto &r : removals) {
        if (!flatList) {
            removeParentMappings(r.idx, 0, q->sourceModel()->rowCount(r.sourceIdx) - 1);
        }
        m_parentIds.removeRight(r.idx);
        m_mappedParents.removeRight(r.idx);
    }
}

QModelIndex KSelectionProxyModelPrivate::mapTopLevelToSource(int row, int column) const
{
    if (!m_startWithChildTrees) {
        const QModelIndex idx = m_rootIndexList.at(row);
        return idx.sibling(idx.row(), column);
    }

    if (m_mappedFirstChildren.isEmpty()) {
        return QModelIndex();
    }

    SourceIndexProxyRowMapping::right_iterator result = std::prev(m_mappedFirstChildren.rightUpperBound(row));

    Q_ASSERT(result != m_mappedFirstChildren.rightEnd());

    const int proxyFirstRow = result.key();
    const QModelIndex sourceFirstChild = result.value();
    Q_ASSERT(sourceFirstChild.isValid());
    return sourceFirstChild.sibling(row - proxyFirstRow, column);
}

void KSelectionProxyModelPrivate::removeSelectionFromProxy(const QItemSelection &selection)
{
    Q_Q(KSelectionProxyModel);
    if (selection.isEmpty()) {
        return;
    }

    QList<QPersistentModelIndex>::iterator rootIt = m_rootIndexList.begin();
    const QList<QPersistentModelIndex>::iterator rootEnd = m_rootIndexList.end();
    int proxyStartRemove = 0;

    for (; rootIt != rootEnd; ++rootIt) {
        if (rootWillBeRemoved(selection, *rootIt)) {
            break;
        } else {
            if (m_startWithChildTrees) {
                auto rc = q->sourceModel()->rowCount(*rootIt);
                proxyStartRemove += rc;
            } else {
                ++proxyStartRemove;
            }
        }
    }

    if (rootIt == rootEnd) {
        return;
    }

    int proxyEndRemove = proxyStartRemove;

    QList<QPersistentModelIndex>::iterator rootRemoveStart = rootIt;

    for (; rootIt != rootEnd; ++rootIt) {
        if (!rootWillBeRemoved(selection, *rootIt)) {
            break;
        }
        q->rootIndexAboutToBeRemoved(*rootIt, KSelectionProxyModel::QPrivateSignal());
        if (m_startWithChildTrees) {
            auto rc = q->sourceModel()->rowCount(*rootIt);
            proxyEndRemove += rc;
        } else {
            ++proxyEndRemove;
        }
    }

    --proxyEndRemove;
    if (proxyEndRemove >= proxyStartRemove) {
        q->beginRemoveRows(QModelIndex(), proxyStartRemove, proxyEndRemove);

        rootIt = m_rootIndexList.erase(rootRemoveStart, rootIt);

        removeParentMappings(QModelIndex(), proxyStartRemove, proxyEndRemove);
        if (m_startWithChildTrees) {
            removeFirstChildMappings(proxyStartRemove, proxyEndRemove);
        }
        updateInternalTopIndexes(proxyEndRemove + 1, -1 * (proxyEndRemove - proxyStartRemove + 1));

        q->endRemoveRows();
    } else {
        rootIt = m_rootIndexList.erase(rootRemoveStart, rootIt);
    }
    if (rootIt != rootEnd) {
        removeSelectionFromProxy(selection);
    }
}

void KSelectionProxyModelPrivate::selectionChanged(const QItemSelection &_selected, const QItemSelection &_deselected)
{
    Q_Q(KSelectionProxyModel);

    if (!q->sourceModel() || (_selected.isEmpty() && _deselected.isEmpty())) {
        return;
    }

    if (m_sourceModelResetting) {
        return;
    }

    if (m_rowsInserted || m_rowsRemoved) {
        m_pendingSelectionChanges.append(PendingSelectionChange(_selected, _deselected));
        return;
    }

    // Any deselected indexes in the m_rootIndexList are removed. Then, any
    // indexes in the selected range which are not a descendant of one of the already selected indexes
    // are inserted into the model.
    //
    // All ranges from the selection model need to be split into individual rows. Ranges which are contiguous in
    // the selection model may not be contiguous in the source model if there's a sort filter proxy model in the chain.
    //
    // Some descendants of deselected indexes may still be selected. The ranges in m_selectionModel->selection()
    // are examined. If any of the ranges are descendants of one of the
    // indexes in deselected, they are added to the ranges to be inserted into the model.
    //
    // The new indexes are inserted in sorted order.

    const QItemSelection selected = kNormalizeSelection(m_indexMapper->mapSelectionRightToLeft(_selected));
    const QItemSelection deselected = kNormalizeSelection(m_indexMapper->mapSelectionRightToLeft(_deselected));

#if QT_VERSION < 0x040800
    // The QItemSelectionModel sometimes doesn't remove deselected items from its selection
    // Fixed in Qt 4.8 : http://qt.gitorious.org/qt/qt/merge_requests/2403
    QItemSelection reportedSelection = m_selectionModel->selection();
    reportedSelection.merge(deselected, QItemSelectionModel::Deselect);
    QItemSelection fullSelection = m_indexMapper->mapSelectionRightToLeft(reportedSelection);
#else
    QItemSelection fullSelection = m_indexMapper->mapSelectionRightToLeft(m_selectionModel->selection());
#endif

    fullSelection = kNormalizeSelection(fullSelection);

    QItemSelection newRootRanges;
    QItemSelection removedRootRanges;
    if (!m_includeAllSelected) {
        newRootRanges = getRootRanges(selected);

        QItemSelection existingSelection = fullSelection;
        // What was selected before the selection was made.
        existingSelection.merge(selected, QItemSelectionModel::Deselect);

        // This is similar to m_rootRanges, but that m_rootRanges at this point still contains the roots
        // of deselected and existingRootRanges does not.

        const QItemSelection existingRootRanges = getRootRanges(existingSelection);
        {
            QMutableListIterator<QItemSelectionRange> i(newRootRanges);
            while (i.hasNext()) {
                const QItemSelectionRange range = i.next();
                const QModelIndex topLeft = range.topLeft();
                if (isDescendantOf(existingRootRanges, topLeft)) {
                    i.remove();
                }
            }
        }

        QItemSelection exposedSelection;
        {
            QItemSelection deselectedRootRanges = getRootRanges(deselected);
            QListIterator<QItemSelectionRange> i(deselectedRootRanges);
            while (i.hasNext()) {
                const QItemSelectionRange range = i.next();
                const QModelIndex topLeft = range.topLeft();
                // Consider this:
                //
                // - A
                // - - B
                // - - - C
                // - - - - D
                //
                // B and D were selected, then B was deselected and C was selected in one go.
                if (!isDescendantOf(existingRootRanges, topLeft)) {
                    // B is topLeft and fullSelection contains D.
                    // B is not a descendant of D.

                    // range is not a descendant of the selection, but maybe the selection is a descendant of range.
                    // no need to check selected here. That's already in newRootRanges.
                    // existingRootRanges and newRootRanges do not overlap.
                    for (const QItemSelectionRange &selectedRange : existingRootRanges) {
                        const QModelIndex selectedRangeTopLeft = selectedRange.topLeft();
                        // existingSelection (and selectedRangeTopLeft) is D.
                        // D is a descendant of B, so when B was removed, D might have been exposed as a root.
                        if (isDescendantOf(range, selectedRangeTopLeft)
                            // But D is also a descendant of part of the new selection C, which is already set to be a new root
                            // so D would not be added to exposedSelection because C is in newRootRanges.
                            && !isDescendantOf(newRootRanges, selectedRangeTopLeft)) {
                            exposedSelection.append(selectedRange);
                        }
                    }
                    removedRootRanges << range;
                }
            }
        }

        QItemSelection obscuredRanges;
        {
            QListIterator<QItemSelectionRange> i(existingRootRanges);
            while (i.hasNext()) {
                const QItemSelectionRange range = i.next();
                if (isDescendantOf(newRootRanges, range.topLeft())) {
                    obscuredRanges << range;
                }
            }
        }
        removedRootRanges << getRootRanges(obscuredRanges);
        newRootRanges << getRootRanges(exposedSelection);

        removedRootRanges = kNormalizeSelection(removedRootRanges);
        newRootRanges = kNormalizeSelection(newRootRanges);
    } else {
        removedRootRanges = deselected;
        newRootRanges = selected;
    }

    removeSelectionFromProxy(removedRootRanges);

    if (!m_selectionModel->hasSelection()) {
        Q_ASSERT(m_rootIndexList.isEmpty());
        Q_ASSERT(m_mappedFirstChildren.isEmpty());
        Q_ASSERT(m_mappedParents.isEmpty());
        Q_ASSERT(m_parentIds.isEmpty());
    }

    insertSelectionIntoProxy(newRootRanges);
}

int KSelectionProxyModelPrivate::getTargetRow(int rootListRow)
{
    Q_Q(KSelectionProxyModel);
    if (!m_startWithChildTrees) {
        return rootListRow;
    }

    --rootListRow;
    while (rootListRow >= 0) {
        const QModelIndex idx = m_rootIndexList.at(rootListRow);
        Q_ASSERT(idx.isValid());
        const int rowCount = q->sourceModel()->rowCount(idx);
        if (rowCount > 0) {
            static const int column = 0;
            const QModelIndex srcIdx = q->sourceModel()->index(rowCount - 1, column, idx);
            const QModelIndex proxyLastChild = mapFromSource(srcIdx);
            return proxyLastChild.row() + 1;
        }
        --rootListRow;
    }
    return 0;
}

void KSelectionProxyModelPrivate::insertSelectionIntoProxy(const QItemSelection &selection)
{
    Q_Q(KSelectionProxyModel);

    if (selection.isEmpty()) {
        return;
    }

    const auto lst = selection.indexes();
    for (const QModelIndex &newIndex : lst) {
        Q_ASSERT(newIndex.model() == q->sourceModel());
        if (newIndex.column() > 0) {
            continue;
        }
        if (m_startWithChildTrees) {
            const int rootListRow = getRootListRow(m_rootIndexList, newIndex);
            Q_ASSERT(q->sourceModel() == newIndex.model());
            const int rowCount = q->sourceModel()->rowCount(newIndex);
            const int startRow = getTargetRow(rootListRow);

            if (rowCount == 0) {
                // Even if the newindex doesn't have any children to put into the model yet,
                // We still need to make sure its future children are inserted into the model.
                m_rootIndexList.insert(rootListRow, newIndex);
                if (!m_resetting || m_layoutChanging) {
                    Q_EMIT q->rootIndexAdded(newIndex, KSelectionProxyModel::QPrivateSignal());
                }
                continue;
            }
            if (!m_resetting) {
                q->beginInsertRows(QModelIndex(), startRow, startRow + rowCount - 1);
            }
            Q_ASSERT(newIndex.isValid());
            m_rootIndexList.insert(rootListRow, newIndex);
            if (!m_resetting || m_layoutChanging) {
                Q_EMIT q->rootIndexAdded(newIndex, KSelectionProxyModel::QPrivateSignal());
            }

            int _start = 0;
            for (int i = 0; i < rootListRow; ++i) {
                _start += q->sourceModel()->rowCount(m_rootIndexList.at(i));
            }

            updateInternalTopIndexes(_start, rowCount);
            createFirstChildMapping(newIndex, _start);
            createParentMappings(newIndex, 0, rowCount - 1);

            if (!m_resetting) {
                q->endInsertRows();
            }

        } else {
            const int row = getRootListRow(m_rootIndexList, newIndex);
            if (!m_resetting) {
                q->beginInsertRows(QModelIndex(), row, row);
            }

            Q_ASSERT(newIndex.isValid());
            m_rootIndexList.insert(row, newIndex);

            if (!m_resetting || m_layoutChanging) {
                Q_EMIT q->rootIndexAdded(newIndex, KSelectionProxyModel::QPrivateSignal());
            }
            Q_ASSERT(m_rootIndexList.size() > row);
            updateInternalIndexes(QModelIndex(), row, 1);
            createParentMappings(newIndex.parent(), newIndex.row(), newIndex.row());

            if (!m_resetting) {
                q->endInsertRows();
            }
        }
    }
    Q_EMIT q->rootSelectionAdded(selection, KSelectionProxyModel::QPrivateSignal());
}

KSelectionProxyModel::KSelectionProxyModel(QItemSelectionModel *selectionModel, QObject *parent)
    : QAbstractProxyModel(parent)
    , d_ptr(new KSelectionProxyModelPrivate(this))
{
    setSelectionModel(selectionModel);
}

KSelectionProxyModel::KSelectionProxyModel()
    : QAbstractProxyModel(nullptr)
    , d_ptr(new KSelectionProxyModelPrivate(this))
{
}

KSelectionProxyModel::~KSelectionProxyModel() = default;

void KSelectionProxyModel::setFilterBehavior(FilterBehavior behavior)
{
    Q_D(KSelectionProxyModel);

    Q_ASSERT(behavior != InvalidBehavior);
    if (behavior == InvalidBehavior) {
        return;
    }
    if (d->m_filterBehavior != behavior) {
        beginResetModel();

        d->m_filterBehavior = behavior;

        switch (behavior) {
        case InvalidBehavior: {
            Q_ASSERT(!"InvalidBehavior can't be used here");
            return;
        }
        case SubTrees: {
            d->m_omitChildren = false;
            d->m_omitDescendants = false;
            d->m_startWithChildTrees = false;
            d->m_includeAllSelected = false;
            break;
        }
        case SubTreeRoots: {
            d->m_omitChildren = true;
            d->m_startWithChildTrees = false;
            d->m_includeAllSelected = false;
            break;
        }
        case SubTreesWithoutRoots: {
            d->m_omitChildren = false;
            d->m_omitDescendants = false;
            d->m_startWithChildTrees = true;
            d->m_includeAllSelected = false;
            break;
        }
        case ExactSelection: {
            d->m_omitChildren = true;
            d->m_startWithChildTrees = false;
            d->m_includeAllSelected = true;
            break;
        }
        case ChildrenOfExactSelection: {
            d->m_omitChildren = false;
            d->m_omitDescendants = true;
            d->m_startWithChildTrees = true;
            d->m_includeAllSelected = true;
            break;
        }
        }
        Q_EMIT filterBehaviorChanged(QPrivateSignal());
        d->resetInternalData();
        if (d->m_selectionModel) {
            d->selectionChanged(d->m_selectionModel->selection(), QItemSelection());
        }

        endResetModel();
    }
}

KSelectionProxyModel::FilterBehavior KSelectionProxyModel::filterBehavior() const
{
    Q_D(const KSelectionProxyModel);
    return d->m_filterBehavior;
}

void KSelectionProxyModel::setSourceModel(QAbstractItemModel *_sourceModel)
{
    Q_D(KSelectionProxyModel);

    Q_ASSERT(_sourceModel != this);

    if (_sourceModel == sourceModel()) {
        return;
    }

    beginResetModel();
    d->m_resetting = true;

    if (auto *oldSourceModel = sourceModel()) {
        disconnect(oldSourceModel, nullptr, this, nullptr);
    }

    // Must be called before QAbstractProxyModel::setSourceModel because it emit some signals.
    d->resetInternalData();
    QAbstractProxyModel::setSourceModel(_sourceModel);
    if (_sourceModel) {
        if (d->m_selectionModel) {
            delete d->m_indexMapper;
            d->m_indexMapper = new KModelIndexProxyMapper(_sourceModel, d->m_selectionModel->model(), this);
            if (d->m_selectionModel->hasSelection()) {
                d->selectionChanged(d->m_selectionModel->selection(), QItemSelection());
            }
        }

        connect(_sourceModel, &QAbstractItemModel::rowsAboutToBeInserted, this, [d](const QModelIndex &parent, int start, int end) {
            d->sourceRowsAboutToBeInserted(parent, start, end);
        });

        connect(_sourceModel, &QAbstractItemModel::rowsInserted, this, [d](const QModelIndex &parent, int start, int end) {
            d->sourceRowsInserted(parent, start, end);
        });

        connect(_sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, [d](const QModelIndex &parent, int start, int end) {
            d->sourceRowsAboutToBeRemoved(parent, start, end);
        });

        connect(_sourceModel, &QAbstractItemModel::rowsRemoved, this, [d](const QModelIndex &parent, int start, int end) {
            d->sourceRowsRemoved(parent, start, end);
        });

        connect(_sourceModel,
                &QAbstractItemModel::rowsAboutToBeMoved,
                this,
                [d](const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow) {
                    d->sourceRowsAboutToBeMoved(parent, start, end, destParent, destRow);
                });

        connect(_sourceModel,
                &QAbstractItemModel::rowsMoved,
                this,
                [d](const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow) {
                    d->sourceRowsMoved(parent, start, end, destParent, destRow);
                });

        connect(_sourceModel, &QAbstractItemModel::modelAboutToBeReset, this, [d]() {
            d->sourceModelAboutToBeReset();
        });

        connect(_sourceModel, &QAbstractItemModel::modelReset, this, [d]() {
            d->sourceModelReset();
        });

        connect(_sourceModel, &QAbstractItemModel::dataChanged, this, [d](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
            d->sourceDataChanged(topLeft, bottomRight);
        });

        connect(_sourceModel, &QAbstractItemModel::layoutAboutToBeChanged, this, [d]() {
            d->sourceLayoutAboutToBeChanged();
        });

        connect(_sourceModel, &QAbstractItemModel::layoutChanged, this, [d]() {
            d->sourceLayoutChanged();
        });

        connect(_sourceModel, &QObject::destroyed, this, [d]() {
            d->sourceModelDestroyed();
        });
    }

    d->m_resetting = false;
    endResetModel();
}

QModelIndex KSelectionProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    Q_D(const KSelectionProxyModel);

    if (!proxyIndex.isValid() || !sourceModel() || d->m_rootIndexList.isEmpty()) {
        return QModelIndex();
    }

    Q_ASSERT(proxyIndex.model() == this);

    if (proxyIndex.internalPointer() == nullptr) {
        return d->mapTopLevelToSource(proxyIndex.row(), proxyIndex.column());
    }

    const QModelIndex proxyParent = d->parentForId(proxyIndex.internalPointer());
    Q_ASSERT(proxyParent.isValid());
    const QModelIndex sourceParent = d->mapParentToSource(proxyParent);
    Q_ASSERT(sourceParent.isValid());
    return sourceModel()->index(proxyIndex.row(), proxyIndex.column(), sourceParent);
}

QModelIndex KSelectionProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    Q_D(const KSelectionProxyModel);

    if (!sourceModel() || !sourceIndex.isValid() || d->m_rootIndexList.isEmpty()) {
        return QModelIndex();
    }

    Q_ASSERT(sourceIndex.model() == sourceModel());

    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }

    if (!d->ensureMappable(sourceIndex)) {
        return QModelIndex();
    }

    return d->mapFromSource(sourceIndex);
}

int KSelectionProxyModel::rowCount(const QModelIndex &index) const
{
    Q_D(const KSelectionProxyModel);

    if (!sourceModel() || index.column() > 0 || d->m_rootIndexList.isEmpty()) {
        return 0;
    }

    Q_ASSERT(index.isValid() ? index.model() == this : true);
    if (!index.isValid()) {
        return d->topLevelRowCount();
    }

    // index is valid
    if (d->isFlat()) {
        return 0;
    }

    QModelIndex sourceParent = d->mapParentToSource(index);

    if (!sourceParent.isValid() && sourceModel()->hasChildren(sourceParent)) {
        sourceParent = mapToSource(index.parent());
        d->createParentMappings(sourceParent, 0, sourceModel()->rowCount(sourceParent) - 1);
        sourceParent = d->mapParentToSource(index);
    }

    if (!sourceParent.isValid()) {
        return 0;
    }

    return sourceModel()->rowCount(sourceParent);
}

QModelIndex KSelectionProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const KSelectionProxyModel);

    if (!sourceModel() || d->m_rootIndexList.isEmpty() || !hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    Q_ASSERT(parent.isValid() ? parent.model() == this : true);

    if (!parent.isValid()) {
        return d->createTopLevelIndex(row, column);
    }

    void *const parentId = d->parentId(parent);
    Q_ASSERT(parentId);
    return createIndex(row, column, parentId);
}

QModelIndex KSelectionProxyModel::parent(const QModelIndex &index) const
{
    Q_D(const KSelectionProxyModel);

    if (!sourceModel() || !index.isValid() || d->m_rootIndexList.isEmpty()) {
        return QModelIndex();
    }

    Q_ASSERT(index.model() == this);

    return d->parentForId(index.internalPointer());
}

Qt::ItemFlags KSelectionProxyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || !sourceModel()) {
        return QAbstractProxyModel::flags(index);
    }

    Q_ASSERT(index.model() == this);

    const QModelIndex srcIndex = mapToSource(index);
    Q_ASSERT(srcIndex.isValid());
    return sourceModel()->flags(srcIndex);
}

QVariant KSelectionProxyModel::data(const QModelIndex &index, int role) const
{
    if (!sourceModel()) {
        return QVariant();
    }

    if (index.isValid()) {
        Q_ASSERT(index.model() == this);
        const QModelIndex idx = mapToSource(index);
        return idx.data(role);
    }
    return sourceModel()->data(index, role);
}

QVariant KSelectionProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!sourceModel()) {
        return QVariant();
    }
    return sourceModel()->headerData(section, orientation, role);
}

QMimeData *KSelectionProxyModel::mimeData(const QModelIndexList &indexes) const
{
    if (!sourceModel()) {
        return QAbstractProxyModel::mimeData(indexes);
    }
    QModelIndexList sourceIndexes;
    for (const QModelIndex &index : indexes) {
        sourceIndexes << mapToSource(index);
    }
    return sourceModel()->mimeData(sourceIndexes);
}

QStringList KSelectionProxyModel::mimeTypes() const
{
    if (!sourceModel()) {
        return QAbstractProxyModel::mimeTypes();
    }
    return sourceModel()->mimeTypes();
}

Qt::DropActions KSelectionProxyModel::supportedDropActions() const
{
    if (!sourceModel()) {
        return QAbstractProxyModel::supportedDropActions();
    }
    return sourceModel()->supportedDropActions();
}

bool KSelectionProxyModel::hasChildren(const QModelIndex &parent) const
{
    Q_D(const KSelectionProxyModel);

    if (d->m_rootIndexList.isEmpty() || !sourceModel()) {
        return false;
    }

    if (parent.isValid()) {
        Q_ASSERT(parent.model() == this);
        if (d->isFlat()) {
            return false;
        }
        return sourceModel()->hasChildren(mapToSource(parent));
    }

    if (!d->m_startWithChildTrees) {
        return true;
    }

    return !d->m_mappedFirstChildren.isEmpty();
}

int KSelectionProxyModel::columnCount(const QModelIndex &index) const
{
    if (!sourceModel() || index.column() > 0) {
        return 0;
    }

    return sourceModel()->columnCount(mapToSource(index));
}

QItemSelectionModel *KSelectionProxyModel::selectionModel() const
{
    Q_D(const KSelectionProxyModel);
    return d->m_selectionModel;
}

void KSelectionProxyModel::setSelectionModel(QItemSelectionModel *itemSelectionModel)
{
    Q_D(KSelectionProxyModel);
    if (d->m_selectionModel != itemSelectionModel) {
        if (d->m_selectionModel) {
            disconnect(d->m_selectionModel,
                       SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                       this,
                       SLOT(selectionChanged(QItemSelection, QItemSelection)));
        }

        d->m_selectionModel = itemSelectionModel;
        Q_EMIT selectionModelChanged(QPrivateSignal());

        if (d->m_selectionModel) {
            connect(d->m_selectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(selectionChanged(QItemSelection, QItemSelection)));

            auto handleSelectionModelModel = [&, d] {
                beginResetModel();
                if (d->selectionModelModelAboutToBeResetConnection) {
                    disconnect(d->selectionModelModelAboutToBeResetConnection);
                }
                if (d->selectionModelModelResetConnection) {
                    disconnect(d->selectionModelModelResetConnection);
                }
                if (d->m_selectionModel->model()) {
                    d->selectionModelModelAboutToBeResetConnection =
                        connect(d->m_selectionModel->model(), SIGNAL(modelAboutToBeReset()), this, SLOT(sourceModelAboutToBeReset()));
                    d->selectionModelModelResetConnection = connect(d->m_selectionModel->model(), SIGNAL(modelReset()), this, SLOT(sourceModelReset()));
                    d->m_rootIndexList.clear();
                    delete d->m_indexMapper;
                    d->m_indexMapper = new KModelIndexProxyMapper(sourceModel(), d->m_selectionModel->model(), this);
                }
                endResetModel();
            };
            connect(d->m_selectionModel.data(), &QItemSelectionModel::modelChanged, this, handleSelectionModelModel);
            handleSelectionModelModel();
        }

        if (sourceModel()) {
            delete d->m_indexMapper;
            d->m_indexMapper = new KModelIndexProxyMapper(sourceModel(), d->m_selectionModel->model(), this);
            if (d->m_selectionModel->hasSelection()) {
                d->selectionChanged(d->m_selectionModel->selection(), QItemSelection());
            }
        }
    }
}

bool KSelectionProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_D(const KSelectionProxyModel);
    if (!sourceModel() || d->m_rootIndexList.isEmpty()) {
        return false;
    }

    if ((row == -1) && (column == -1)) {
        return sourceModel()->dropMimeData(data, action, -1, -1, mapToSource(parent));
    }

    int source_destination_row = -1;
    int source_destination_column = -1;
    QModelIndex source_parent;

    if (row == rowCount(parent)) {
        source_parent = mapToSource(parent);
        source_destination_row = sourceModel()->rowCount(source_parent);
    } else {
        const QModelIndex proxy_index = index(row, column, parent);
        const QModelIndex source_index = mapToSource(proxy_index);
        source_destination_row = source_index.row();
        source_destination_column = source_index.column();
        source_parent = source_index.parent();
    }
    return sourceModel()->dropMimeData(data, action, source_destination_row, source_destination_column, source_parent);
}

QList<QPersistentModelIndex> KSelectionProxyModel::sourceRootIndexes() const
{
    Q_D(const KSelectionProxyModel);
    return d->m_rootIndexList;
}

QModelIndexList KSelectionProxyModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    if (role < Qt::UserRole) {
        return QAbstractProxyModel::match(start, role, value, hits, flags);
    }

    QModelIndexList list;
    QModelIndex proxyIndex;
    const auto lst = sourceModel()->match(mapToSource(start), role, value, hits, flags);
    for (const QModelIndex &idx : lst) {
        proxyIndex = mapFromSource(idx);
        if (proxyIndex.isValid()) {
            list << proxyIndex;
        }
    }
    return list;
}

QItemSelection KSelectionProxyModel::mapSelectionFromSource(const QItemSelection &selection) const
{
    Q_D(const KSelectionProxyModel);
    if (!d->m_startWithChildTrees && d->m_includeAllSelected) {
        // QAbstractProxyModel::mapSelectionFromSource puts invalid ranges in the result
        // without checking. We can't have that.
        QItemSelection proxySelection;
        for (const QItemSelectionRange &range : selection) {
            QModelIndex proxyTopLeft = mapFromSource(range.topLeft());
            if (!proxyTopLeft.isValid()) {
                continue;
            }
            QModelIndex proxyBottomRight = mapFromSource(range.bottomRight());
            Q_ASSERT(proxyBottomRight.isValid());
            proxySelection.append(QItemSelectionRange(proxyTopLeft, proxyBottomRight));
        }
        return proxySelection;
    }

    QItemSelection proxySelection;
    QItemSelection::const_iterator it = selection.constBegin();
    const QItemSelection::const_iterator end = selection.constEnd();
    for (; it != end; ++it) {
        const QModelIndex proxyTopLeft = mapFromSource(it->topLeft());
        if (!proxyTopLeft.isValid()) {
            continue;
        }

        if (it->height() == 1 && it->width() == 1) {
            proxySelection.append(QItemSelectionRange(proxyTopLeft, proxyTopLeft));
        } else {
            proxySelection.append(QItemSelectionRange(proxyTopLeft, d->mapFromSource(it->bottomRight())));
        }
    }
    return proxySelection;
}

QItemSelection KSelectionProxyModel::mapSelectionToSource(const QItemSelection &selection) const
{
    Q_D(const KSelectionProxyModel);

    if (selection.isEmpty()) {
        return selection;
    }

    if (!d->m_startWithChildTrees && d->m_includeAllSelected) {
        // QAbstractProxyModel::mapSelectionFromSource puts invalid ranges in the result
        // without checking. We can't have that.
        QItemSelection sourceSelection;
        for (const QItemSelectionRange &range : selection) {
            QModelIndex sourceTopLeft = mapToSource(range.topLeft());
            Q_ASSERT(sourceTopLeft.isValid());

            QModelIndex sourceBottomRight = mapToSource(range.bottomRight());
            Q_ASSERT(sourceBottomRight.isValid());
            sourceSelection.append(QItemSelectionRange(sourceTopLeft, sourceBottomRight));
        }
        return sourceSelection;
    }

    QItemSelection sourceSelection;
    QItemSelection extraSelection;
    QItemSelection::const_iterator it = selection.constBegin();
    const QItemSelection::const_iterator end = selection.constEnd();
    for (; it != end; ++it) {
        const QModelIndex sourceTopLeft = mapToSource(it->topLeft());
        if (it->height() == 1 && it->width() == 1) {
            sourceSelection.append(QItemSelectionRange(sourceTopLeft, sourceTopLeft));
        } else if (it->parent().isValid()) {
            sourceSelection.append(QItemSelectionRange(sourceTopLeft, mapToSource(it->bottomRight())));
        } else {
            // A contiguous selection in the proxy might not be contiguous in the source if it
            // is at the top level of the proxy.
            if (d->m_startWithChildTrees) {
                const QModelIndex sourceParent = mapFromSource(sourceTopLeft);
                Q_ASSERT(sourceParent.isValid());
                const int rowCount = sourceModel()->rowCount(sourceParent);
                if (rowCount < it->bottom()) {
                    Q_ASSERT(sourceTopLeft.isValid());
                    Q_ASSERT(it->bottomRight().isValid());
                    const QModelIndex sourceBottomRight = mapToSource(it->bottomRight());
                    Q_ASSERT(sourceBottomRight.isValid());
                    sourceSelection.append(QItemSelectionRange(sourceTopLeft, sourceBottomRight));
                    continue;
                }
                // Store the contiguous part...
                const QModelIndex sourceBottomRight = sourceModel()->index(rowCount - 1, it->right(), sourceParent);
                Q_ASSERT(sourceTopLeft.isValid());
                Q_ASSERT(sourceBottomRight.isValid());
                sourceSelection.append(QItemSelectionRange(sourceTopLeft, sourceBottomRight));
                // ... and the rest will be processed later.
                extraSelection.append(QItemSelectionRange(createIndex(it->top() - rowCount, it->right()), it->bottomRight()));
            } else {
                QItemSelection topSelection;
                const QModelIndex idx = createIndex(it->top(), it->right());
                const QModelIndex sourceIdx = mapToSource(idx);
                Q_ASSERT(sourceIdx.isValid());
                topSelection.append(QItemSelectionRange(sourceTopLeft, sourceIdx));
                for (int i = it->top() + 1; i <= it->bottom(); ++i) {
                    const QModelIndex left = mapToSource(createIndex(i, 0));
                    const QModelIndex right = mapToSource(createIndex(i, it->right()));
                    Q_ASSERT(left.isValid());
                    Q_ASSERT(right.isValid());
                    topSelection.append(QItemSelectionRange(left, right));
                }
                sourceSelection += kNormalizeSelection(topSelection);
            }
        }
    }
    sourceSelection << mapSelectionToSource(extraSelection);
    return sourceSelection;
}

#include "moc_kselectionproxymodel.cpp"
