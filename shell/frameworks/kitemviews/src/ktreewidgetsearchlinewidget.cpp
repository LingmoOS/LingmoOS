/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Scott Wheeler <wheeler@kde.org>
    SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ktreewidgetsearchlinewidget.h"
#include "ktreewidgetsearchline.h"

#include <QHBoxLayout>
#include <QTreeWidget>

class KTreeWidgetSearchLineWidgetPrivate
{
public:
    QTreeWidget *treeWidget = nullptr;
    KTreeWidgetSearchLine *searchLine = nullptr;
};

KTreeWidgetSearchLineWidget::KTreeWidgetSearchLineWidget(QWidget *parent, QTreeWidget *treeWidget)
    : QWidget(parent)
    , d(new KTreeWidgetSearchLineWidgetPrivate)
{
    d->treeWidget = treeWidget;

    // can't call createWidgets directly because it calls virtual functions
    // that might not work if called directly from here due to how inheritance works
    QMetaObject::invokeMethod(this, "createWidgets", Qt::QueuedConnection);
}

KTreeWidgetSearchLineWidget::~KTreeWidgetSearchLineWidget() = default;

KTreeWidgetSearchLine *KTreeWidgetSearchLineWidget::createSearchLine(QTreeWidget *treeWidget) const
{
    return new KTreeWidgetSearchLine(const_cast<KTreeWidgetSearchLineWidget *>(this), treeWidget);
}

void KTreeWidgetSearchLineWidget::createWidgets()
{
    searchLine()->show();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->searchLine);
    setFocusProxy(searchLine());
}

KTreeWidgetSearchLine *KTreeWidgetSearchLineWidget::searchLine() const
{
    if (!d->searchLine) {
        d->searchLine = createSearchLine(d->treeWidget);
    }

    return d->searchLine;
}

#include "moc_ktreewidgetsearchlinewidget.cpp"
