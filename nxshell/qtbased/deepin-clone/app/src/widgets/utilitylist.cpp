// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "utilitylist.h"
#include "utilitylistitem.h"

UtilityList::UtilityList(QWidget *parent)
    : QListWidget(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionRectVisible(false);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

void UtilityList::addItem(UtilityListItem *item)
{
    QListWidgetItem *lwi = new QListWidgetItem(this);
    QListWidget::addItem(lwi);

    QSize size = item->sizeHint();

    size.setWidth(qMin(size.width(), width()));

    lwi->setSizeHint(size);
    setItemWidget(lwi, item);

    if (count() == 1)
        lwi->setSelected(true);
}

void UtilityList::addItem(const QIcon &icon, const QString &title, const QString &message, const QSize &iconSize)
{
    UtilityListItem *item = new UtilityListItem(this);

    item->setIcon(icon, iconSize);
    item->setTitle(title);
    item->setMessage(message);

    addItem(item);
}

void UtilityList::showEvent(QShowEvent *e)
{
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *item = this->item(i);
        const QWidget *widget = itemWidget(item);

        if (widget) {
            item->setSizeHint(QSize(widget->width(), widget->heightForWidth(width())));
        }
    }

    return QListWidget::showEvent(e);
}
