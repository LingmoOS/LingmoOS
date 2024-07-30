/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbreadcrumbselectionmodel.h"

class KBreadcrumbSelectionModelPrivate
{
    Q_DECLARE_PUBLIC(KBreadcrumbSelectionModel)
    KBreadcrumbSelectionModel *const q_ptr;

public:
    KBreadcrumbSelectionModelPrivate(KBreadcrumbSelectionModel *breadcrumbSelector,
                                     QItemSelectionModel *selectionModel,
                                     KBreadcrumbSelectionModel::BreadcrumbTarget direction);

    /**
      Returns a selection containing the breadcrumbs for @p index
    */
    QItemSelection getBreadcrumbSelection(const QModelIndex &index);

    /**
      Returns a selection containing the breadcrumbs for @p selection
    */
    QItemSelection getBreadcrumbSelection(const QItemSelection &selection);

    void sourceSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void syncBreadcrumbs();

    bool m_includeActualSelection = true;
    bool m_showHiddenAscendantData = false;
    bool m_ignoreCurrentChanged = false;
    int m_selectionDepth = -1;
    KBreadcrumbSelectionModel::BreadcrumbTarget m_direction = KBreadcrumbSelectionModel::MakeBreadcrumbSelectionInSelf;
    QItemSelectionModel *m_selectionModel = nullptr;
};

KBreadcrumbSelectionModelPrivate::KBreadcrumbSelectionModelPrivate(KBreadcrumbSelectionModel *breadcrumbSelector,
                                                                   QItemSelectionModel *selectionModel,
                                                                   KBreadcrumbSelectionModel::BreadcrumbTarget direction)
    : q_ptr(breadcrumbSelector)
    , m_direction(direction)
    , m_selectionModel(selectionModel)
{
    Q_Q(KBreadcrumbSelectionModel);

    if (direction != KBreadcrumbSelectionModel::MakeBreadcrumbSelectionInSelf) {
        q->connect(selectionModel, &QItemSelectionModel::selectionChanged, q, [this](const QItemSelection &selected, const QItemSelection &deselected) {
            sourceSelectionChanged(selected, deselected);
        });
    }

    q->connect(m_selectionModel->model(), &QAbstractItemModel::layoutChanged, q, [this]() {
        syncBreadcrumbs();
    });
    q->connect(m_selectionModel->model(), &QAbstractItemModel::modelReset, q, [this]() {
        syncBreadcrumbs();
    });
    q->connect(m_selectionModel->model(), &QAbstractItemModel::rowsMoved, q, [this]() {
        syncBreadcrumbs();
    });

    // Don't need to handle insert & remove because they can't change the breadcrumbs on their own.
}

KBreadcrumbSelectionModel::KBreadcrumbSelectionModel(QItemSelectionModel *selectionModel, QObject *parent)
    : QItemSelectionModel(const_cast<QAbstractItemModel *>(selectionModel->model()), parent)
    , d_ptr(new KBreadcrumbSelectionModelPrivate(this, selectionModel, MakeBreadcrumbSelectionInSelf))
{
}

KBreadcrumbSelectionModel::KBreadcrumbSelectionModel(QItemSelectionModel *selectionModel, BreadcrumbTarget direction, QObject *parent)
    : QItemSelectionModel(const_cast<QAbstractItemModel *>(selectionModel->model()), parent)
    , d_ptr(new KBreadcrumbSelectionModelPrivate(this, selectionModel, direction))
{
}

KBreadcrumbSelectionModel::~KBreadcrumbSelectionModel() = default;

bool KBreadcrumbSelectionModel::isActualSelectionIncluded() const
{
    Q_D(const KBreadcrumbSelectionModel);
    return d->m_includeActualSelection;
}

void KBreadcrumbSelectionModel::setActualSelectionIncluded(bool includeActualSelection)
{
    Q_D(KBreadcrumbSelectionModel);
    d->m_includeActualSelection = includeActualSelection;
}

int KBreadcrumbSelectionModel::breadcrumbLength() const
{
    Q_D(const KBreadcrumbSelectionModel);
    return d->m_selectionDepth;
}

void KBreadcrumbSelectionModel::setBreadcrumbLength(int breadcrumbLength)
{
    Q_D(KBreadcrumbSelectionModel);
    d->m_selectionDepth = breadcrumbLength;
}

QItemSelection KBreadcrumbSelectionModelPrivate::getBreadcrumbSelection(const QModelIndex &index)
{
    QItemSelection breadcrumbSelection;

    if (m_includeActualSelection) {
        breadcrumbSelection.append(QItemSelectionRange(index));
    }

    QModelIndex parent = index.parent();
    int sumBreadcrumbs = 0;
    bool includeAll = m_selectionDepth < 0;
    while (parent.isValid() && (includeAll || sumBreadcrumbs < m_selectionDepth)) {
        breadcrumbSelection.append(QItemSelectionRange(parent));
        parent = parent.parent();
    }
    return breadcrumbSelection;
}

QItemSelection KBreadcrumbSelectionModelPrivate::getBreadcrumbSelection(const QItemSelection &selection)
{
    QItemSelection breadcrumbSelection;

    if (m_includeActualSelection) {
        breadcrumbSelection = selection;
    }

    QItemSelection::const_iterator it = selection.constBegin();
    const QItemSelection::const_iterator end = selection.constEnd();

    for (; it != end; ++it) {
        QModelIndex parent = it->parent();

        if (breadcrumbSelection.contains(parent)) {
            continue;
        }

        int sumBreadcrumbs = 0;
        bool includeAll = m_selectionDepth < 0;

        while (parent.isValid() && (includeAll || sumBreadcrumbs < m_selectionDepth)) {
            breadcrumbSelection.append(QItemSelectionRange(parent));
            parent = parent.parent();

            if (breadcrumbSelection.contains(parent)) {
                break;
            }

            ++sumBreadcrumbs;
        }
    }
    return breadcrumbSelection;
}

void KBreadcrumbSelectionModelPrivate::sourceSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_Q(KBreadcrumbSelectionModel);
    const QItemSelection deselectedCrumbs = getBreadcrumbSelection(deselected);
    const QItemSelection selectedCrumbs = getBreadcrumbSelection(selected);

    QItemSelection removed = deselectedCrumbs;
    for (const QItemSelectionRange &range : selectedCrumbs) {
        removed.removeAll(range);
    }

    QItemSelection added = selectedCrumbs;
    for (const QItemSelectionRange &range : deselectedCrumbs) {
        added.removeAll(range);
    }

    if (!removed.isEmpty()) {
        q->QItemSelectionModel::select(removed, QItemSelectionModel::Deselect);
    }
    if (!added.isEmpty()) {
        q->QItemSelectionModel::select(added, QItemSelectionModel::Select);
    }
}

void KBreadcrumbSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    Q_D(KBreadcrumbSelectionModel);
    // When an item is removed, the current index is set to the top index in the model.
    // That causes a selectionChanged signal with a selection which we do not want.
    if (d->m_ignoreCurrentChanged) {
        d->m_ignoreCurrentChanged = false;
        return;
    }
    if (d->m_direction == MakeBreadcrumbSelectionInOther) {
        d->m_selectionModel->select(d->getBreadcrumbSelection(index), command);
        QItemSelectionModel::select(index, command);
    } else {
        d->m_selectionModel->select(index, command);
        QItemSelectionModel::select(d->getBreadcrumbSelection(index), command);
    }
}

void KBreadcrumbSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    Q_D(KBreadcrumbSelectionModel);
    QItemSelection bcc = d->getBreadcrumbSelection(selection);
    if (d->m_direction == MakeBreadcrumbSelectionInOther) {
        d->m_selectionModel->select(selection, command);
        QItemSelectionModel::select(bcc, command);
    } else {
        d->m_selectionModel->select(bcc, command);
        QItemSelectionModel::select(selection, command);
    }
}

void KBreadcrumbSelectionModelPrivate::syncBreadcrumbs()
{
    Q_Q(KBreadcrumbSelectionModel);
    q->select(m_selectionModel->selection(), QItemSelectionModel::ClearAndSelect);
}

#include "moc_kbreadcrumbselectionmodel.cpp"
