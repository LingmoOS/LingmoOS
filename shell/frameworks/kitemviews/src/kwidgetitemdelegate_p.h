/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/**
 * This class is necessary to be installed because of the templated method.
 * It is private in the sense of having clean the public header.
 * Do not forget that this file _has_ to be installed.
 */

#ifndef KWIDGETITEMDELEGATE_P_H
#define KWIDGETITEMDELEGATE_P_H

#include <QItemSelectionModel>

class KWidgetItemDelegate;

class KWidgetItemDelegatePrivate : public QObject
{
    Q_OBJECT

public:
    explicit KWidgetItemDelegatePrivate(KWidgetItemDelegate *q, QObject *parent = nullptr);
    ~KWidgetItemDelegatePrivate() override;

    void _k_slotRowsInserted(const QModelIndex &parent, int start, int end);
    void _k_slotRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void _k_slotRowsRemoved(const QModelIndex &parent, int start, int end);
    void _k_slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void _k_slotLayoutChanged();
    void _k_slotModelReset();
    void _k_slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void updateRowRange(const QModelIndex &parent, int start, int end, bool isRemoving);
    QStyleOptionViewItem optionView(const QModelIndex &index);

public Q_SLOTS:
    void initializeModel(const QModelIndex &parent = QModelIndex());

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

public:
    QAbstractItemView *itemView = nullptr;
    KWidgetItemDelegatePool *const widgetPool;
    QAbstractItemModel *model = nullptr;
    QItemSelectionModel *selectionModel = nullptr;
    bool viewDestroyed = false;

    KWidgetItemDelegate *const q;
};

#endif
