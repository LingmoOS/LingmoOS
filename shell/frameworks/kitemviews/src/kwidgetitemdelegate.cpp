/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007-2008 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwidgetitemdelegate.h"
#include "kwidgetitemdelegate_p.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QCursor>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>
#include <QTreeView>

#include "kwidgetitemdelegatepool_p.h"

Q_DECLARE_METATYPE(QList<QEvent::Type>)

/**
  KWidgetItemDelegatePrivate class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
KWidgetItemDelegatePrivate::KWidgetItemDelegatePrivate(KWidgetItemDelegate *q, QObject *parent)
    : QObject(parent)
    , widgetPool(new KWidgetItemDelegatePool(q))
    , q(q)
{
}

KWidgetItemDelegatePrivate::~KWidgetItemDelegatePrivate()
{
    if (!viewDestroyed) {
        widgetPool->fullClear();
    }
    delete widgetPool;
}

void KWidgetItemDelegatePrivate::_k_slotRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end);
    // We need to update the rows behind the inserted row as well because the widgets need to be
    // moved to their new position
    updateRowRange(parent, start, model->rowCount(parent), false);
}

void KWidgetItemDelegatePrivate::_k_slotRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    updateRowRange(parent, start, end, true);
}

void KWidgetItemDelegatePrivate::_k_slotRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end);
    // We need to update the rows that come behind the deleted rows because the widgets need to be
    // moved to the new position
    updateRowRange(parent, start, model->rowCount(parent), false);
}

void KWidgetItemDelegatePrivate::_k_slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        for (int j = topLeft.column(); j <= bottomRight.column(); ++j) {
            const QModelIndex index = model->index(i, j, topLeft.parent());
            widgetPool->findWidgets(index, optionView(index));
        }
    }
}

void KWidgetItemDelegatePrivate::_k_slotLayoutChanged()
{
    const auto lst = widgetPool->invalidIndexesWidgets();
    for (QWidget *widget : lst) {
        widget->setVisible(false);
    }
    QTimer::singleShot(0, this, SLOT(initializeModel()));
}

void KWidgetItemDelegatePrivate::_k_slotModelReset()
{
    widgetPool->fullClear();
    QTimer::singleShot(0, this, SLOT(initializeModel()));
}

void KWidgetItemDelegatePrivate::_k_slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    const auto lstSelected = selected.indexes();
    for (const QModelIndex &index : lstSelected) {
        widgetPool->findWidgets(index, optionView(index));
    }
    const auto lstDeselected = deselected.indexes();
    for (const QModelIndex &index : lstDeselected) {
        widgetPool->findWidgets(index, optionView(index));
    }
}

void KWidgetItemDelegatePrivate::updateRowRange(const QModelIndex &parent, int start, int end, bool isRemoving)
{
    int i = start;
    while (i <= end) {
        for (int j = 0; j < model->columnCount(parent); ++j) {
            const QModelIndex index = model->index(i, j, parent);
            const QList<QWidget *> widgetList =
                widgetPool->findWidgets(index,
                                        optionView(index),
                                        isRemoving ? KWidgetItemDelegatePool::NotUpdateWidgets : KWidgetItemDelegatePool::UpdateWidgets);
            if (isRemoving) {
                for (QWidget *widget : widgetList) {
                    const QModelIndex idx = widgetPool->d->widgetInIndex[widget];
                    widgetPool->d->usedWidgets.remove(idx);
                    widgetPool->d->widgetInIndex.remove(widget);
                    delete widget;
                }
            }
        }
        i++;
    }
}

inline QStyleOptionViewItem KWidgetItemDelegatePrivate::optionView(const QModelIndex &index)
{
    QStyleOptionViewItem optionView;
    optionView.initFrom(itemView->viewport());
    optionView.rect = itemView->visualRect(index);
    optionView.decorationSize = itemView->iconSize();
    return optionView;
}

void KWidgetItemDelegatePrivate::initializeModel(const QModelIndex &parent)
{
    if (!model) {
        return;
    }

    for (int i = 0; i < model->rowCount(parent); ++i) {
        for (int j = 0; j < model->columnCount(parent); ++j) {
            const QModelIndex index = model->index(i, j, parent);
            if (index.isValid()) {
                widgetPool->findWidgets(index, optionView(index));
            }
        }
        // Check if we need to go recursively through the children of parent (if any) to initialize
        // all possible indexes that are shown.
        const QModelIndex index = model->index(i, 0, parent);
        if (index.isValid() && model->hasChildren(index)) {
            initializeModel(index);
        }
    }
}
//@endcond

KWidgetItemDelegate::KWidgetItemDelegate(QAbstractItemView *itemView, QObject *parent)
    : QAbstractItemDelegate(parent)
    , d(new KWidgetItemDelegatePrivate(this))
{
    Q_ASSERT(itemView);

    itemView->setMouseTracking(true);
    itemView->viewport()->setAttribute(Qt::WA_Hover);

    d->itemView = itemView;

    itemView->viewport()->installEventFilter(d.get()); // mouse events
    itemView->installEventFilter(d.get()); // keyboard events

    if (qobject_cast<QTreeView *>(itemView)) {
        connect(itemView, SIGNAL(collapsed(QModelIndex)), d.get(), SLOT(initializeModel()));
        connect(itemView, SIGNAL(expanded(QModelIndex)), d.get(), SLOT(initializeModel()));
    }
}

KWidgetItemDelegate::~KWidgetItemDelegate() = default;

QAbstractItemView *KWidgetItemDelegate::itemView() const
{
    return d->itemView;
}

QPersistentModelIndex KWidgetItemDelegate::focusedIndex() const
{
    const QPersistentModelIndex idx = d->widgetPool->d->widgetInIndex.value(QApplication::focusWidget());
    if (idx.isValid()) {
        return idx;
    }
    // Use the mouse position, if the widget refused to take keyboard focus.
    const QPoint pos = d->itemView->viewport()->mapFromGlobal(QCursor::pos());
    return d->itemView->indexAt(pos);
}

//@cond PRIVATE
bool KWidgetItemDelegatePrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Destroy) {
        // we care for the view since it deletes the widgets (parentage).
        // if the view hasn't been deleted, it might be that just the
        // delegate is removed from it, in which case we need to remove the widgets
        // manually, otherwise they still get drawn.
        if (watched == itemView) {
            viewDestroyed = true;
        }
        return false;
    }

    Q_ASSERT(itemView);

    // clang-format off
    if (model != itemView->model()) {
        if (model) {
            disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), q, SLOT(_k_slotRowsInserted(QModelIndex,int,int)));
            disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), q, SLOT(_k_slotRowsAboutToBeRemoved(QModelIndex,int,int)));
            disconnect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)), q, SLOT(_k_slotRowsRemoved(QModelIndex,int,int)));
            disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), q, SLOT(_k_slotDataChanged(QModelIndex,QModelIndex)));
            disconnect(model, SIGNAL(layoutChanged()), q, SLOT(_k_slotLayoutChanged()));
            disconnect(model, SIGNAL(modelReset()), q, SLOT(_k_slotModelReset()));
        }
        model = itemView->model();
        connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), q, SLOT(_k_slotRowsInserted(QModelIndex,int,int)));
        connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), q, SLOT(_k_slotRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)), q, SLOT(_k_slotRowsRemoved(QModelIndex,int,int)));
        connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), q, SLOT(_k_slotDataChanged(QModelIndex,QModelIndex)));
        connect(model, SIGNAL(layoutChanged()), q, SLOT(_k_slotLayoutChanged()));
        connect(model, SIGNAL(modelReset()), q, SLOT(_k_slotModelReset()));
        QTimer::singleShot(0, this, SLOT(initializeModel()));
    }

    if (selectionModel != itemView->selectionModel()) {
        if (selectionModel) {
            disconnect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), q, SLOT(_k_slotSelectionChanged(QItemSelection,QItemSelection)));
        }
        selectionModel = itemView->selectionModel();
        connect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), q, SLOT(_k_slotSelectionChanged(QItemSelection,QItemSelection)));
        QTimer::singleShot(0, this, SLOT(initializeModel()));
    }
    // clang-format on

    switch (event->type()) {
    case QEvent::Polish:
    case QEvent::Resize:
        if (!qobject_cast<QAbstractItemView *>(watched)) {
            QTimer::singleShot(0, this, SLOT(initializeModel()));
        }
        break;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
        if (qobject_cast<QAbstractItemView *>(watched)) {
            const auto lst = selectionModel->selectedIndexes();
            for (const QModelIndex &index : lst) {
                if (index.isValid()) {
                    widgetPool->findWidgets(index, optionView(index));
                }
            }
        }
        break;
    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}
//@endcond

void KWidgetItemDelegate::setBlockedEventTypes(QWidget *widget, const QList<QEvent::Type> &types) const
{
    widget->setProperty("goya:blockedEventTypes", QVariant::fromValue(types));
}

QList<QEvent::Type> KWidgetItemDelegate::blockedEventTypes(QWidget *widget) const
{
    return widget->property("goya:blockedEventTypes").value<QList<QEvent::Type>>();
}

void KWidgetItemDelegate::resetModel()
{
    d->_k_slotModelReset();
}

#include "moc_kwidgetitemdelegate.cpp"
#include "moc_kwidgetitemdelegate_p.cpp"
