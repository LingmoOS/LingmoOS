/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "proxyitemselectionwidget.h"

#include <QHBoxLayout>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QTreeView>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include "klinkitemselectionmodel.h"

#define SON(object) object->setObjectName(QStringLiteral(#object))

ProxyItemSelectionWidget::ProxyItemSelectionWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    QSplitter *splitter = new QSplitter(this);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(splitter);

    DynamicTreeModel *rootModel = new DynamicTreeModel(this);

    DynamicTreeWidget *dynamicTreeWidget = new DynamicTreeWidget(rootModel, splitter);

    dynamicTreeWidget->setInitialTree(
        QLatin1String("- 1"
                      "- 2"
                      "- - 3"
                      "- - - 4"
                      "- 5"
                      "- 6"
                      "- 7"));

    QSplitter *vSplitter = new QSplitter(Qt::Vertical, splitter);
    QSplitter *hSplitter1 = new QSplitter(vSplitter);
    QSplitter *hSplitter2 = new QSplitter(vSplitter);

    QSortFilterProxyModel *proxy1 = new QSortFilterProxyModel(this);
    SON(proxy1);
    QSortFilterProxyModel *proxy2 = new QSortFilterProxyModel(this);
    SON(proxy2);
    QSortFilterProxyModel *proxy3 = new QSortFilterProxyModel(this);
    SON(proxy3);
    QSortFilterProxyModel *proxy4 = new QSortFilterProxyModel(this);
    SON(proxy4);
    QSortFilterProxyModel *proxy5 = new QSortFilterProxyModel(this);
    SON(proxy5);

    QTreeView *view1 = new QTreeView(hSplitter1);
    QTreeView *view2 = new QTreeView(hSplitter1);
    QTreeView *view3 = new QTreeView(hSplitter2);
    QTreeView *view4 = new QTreeView(hSplitter2);

    proxy1->setSourceModel(rootModel);
    proxy2->setSourceModel(proxy1);
    proxy3->setSourceModel(proxy2);

    proxy4->setSourceModel(rootModel);
    proxy5->setSourceModel(proxy4);

    view1->setModel(proxy3);
    view2->setModel(proxy5);
    view3->setModel(proxy2);
    view4->setModel(proxy1);

    QItemSelectionModel *rootSelectionModel = dynamicTreeWidget->treeView()->selectionModel();

    KLinkItemSelectionModel *view1SelectionModel = new KLinkItemSelectionModel(view1->model(), rootSelectionModel, this);
    view1->setSelectionModel(view1SelectionModel);

    KLinkItemSelectionModel *view2SelectionModel = new KLinkItemSelectionModel(view2->model(), view1->selectionModel(), this);
    view2->setSelectionModel(view2SelectionModel);

    KLinkItemSelectionModel *view3SelectionModel = new KLinkItemSelectionModel(view3->model(), view4->selectionModel(), this);
    view3->setSelectionModel(view3SelectionModel);

    view1->expandAll();
    view2->expandAll();
    view3->expandAll();
    view4->expandAll();
}

#include "moc_proxyitemselectionwidget.cpp"
