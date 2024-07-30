/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KPAGEWIDGETMODEL_P_H
#define KPAGEWIDGETMODEL_P_H

#include "kpagemodel_p.h"
#include "kpagewidgetmodel.h"

class PageItem
{
public:
    explicit PageItem(KPageWidgetItem *pageItem, PageItem *parent = nullptr);
    ~PageItem();

    PageItem(const PageItem &) = delete;
    PageItem &operator=(const PageItem &) = delete;

    void appendChild(PageItem *child);
    void insertChild(int row, PageItem *child);
    void removeChild(int row);

    PageItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    PageItem *parent();

    KPageWidgetItem *pageWidgetItem() const;

    PageItem *findChild(const KPageWidgetItem *item);

    void dump(int indent = 0);

private:
    KPageWidgetItem *mPageWidgetItem;

    QList<PageItem *> mChildItems;
    PageItem *mParentItem;
};

class KPageWidgetModelPrivate : public KPageModelPrivate
{
    Q_DECLARE_PUBLIC(KPageWidgetModel)
protected:
    KPageWidgetModelPrivate()
        : rootItem(new PageItem(nullptr, nullptr))
    {
    }

    ~KPageWidgetModelPrivate() override
    {
        delete rootItem;
        rootItem = nullptr;
    }

    PageItem *rootItem;

    void _k_itemChanged()
    {
        Q_Q(KPageWidgetModel);
        KPageWidgetItem *item = qobject_cast<KPageWidgetItem *>(q->sender());
        if (!item) {
            return;
        }

        const QModelIndex index = q->index(item);
        if (!index.isValid()) {
            return;
        }

        Q_EMIT q->dataChanged(index, index);
    }

    void _k_itemToggled(bool checked)
    {
        Q_Q(KPageWidgetModel);
        KPageWidgetItem *item = qobject_cast<KPageWidgetItem *>(q->sender());
        if (!item) {
            return;
        }

        Q_EMIT q->toggled(item, checked);
    }
};

#endif // KPAGEWIDGETMODEL_P_H
