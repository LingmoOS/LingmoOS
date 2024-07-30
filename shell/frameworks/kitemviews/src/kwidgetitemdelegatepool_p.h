/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWIDGETITEMDELEGATEPOOL_P_H
#define KWIDGETITEMDELEGATEPOOL_P_H

#include <QHash>
#include <QList>
#include <QPersistentModelIndex>

class QWidget;
class QStyleOptionViewItem;
class KWidgetItemDelegate;
class KWidgetItemDelegatePoolPrivate;

/**
 * @internal
 */
class KWidgetItemDelegatePool
{
public:
    enum UpdateWidgetsEnum {
        UpdateWidgets = 0,
        NotUpdateWidgets,
    };

    /**
     * Creates a new ItemDelegatePool.
     *
     * @param delegate the ItemDelegate for this pool.
     */

    KWidgetItemDelegatePool(KWidgetItemDelegate *delegate);

    /**
     * Destroys an ItemDelegatePool.
     */
    ~KWidgetItemDelegatePool();

    KWidgetItemDelegatePool(const KWidgetItemDelegatePool &) = delete;
    KWidgetItemDelegatePool &operator=(const KWidgetItemDelegatePool &) = delete;

    /**
     * @brief Returns the widget associated to @p index and @p widget
     * @param index The index to search into.
     * @param option a QStyleOptionViewItem.
     * @return A QList of the pointers to the widgets found.
     * @internal
     */
    QList<QWidget *> findWidgets(const QPersistentModelIndex &index, const QStyleOptionViewItem &option, UpdateWidgetsEnum updateWidgets = UpdateWidgets) const;

    /**
     * @internal
     */
    QList<QWidget *> invalidIndexesWidgets() const;

    /**
     * @internal
     */
    void fullClear();

private:
    friend class KWidgetItemDelegate;
    friend class KWidgetItemDelegatePrivate;
    KWidgetItemDelegatePoolPrivate *const d;
};

class KWidgetItemDelegateEventListener;

/**
 * @internal
 */
class KWidgetItemDelegatePoolPrivate
{
public:
    KWidgetItemDelegatePoolPrivate(KWidgetItemDelegate *d);

    KWidgetItemDelegate *delegate;
    KWidgetItemDelegateEventListener *eventListener;

    QHash<QPersistentModelIndex, QList<QWidget *>> usedWidgets;
    QHash<QWidget *, QPersistentModelIndex> widgetInIndex;

    bool clearing = false;
};

#endif
