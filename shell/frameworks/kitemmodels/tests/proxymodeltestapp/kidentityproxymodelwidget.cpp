/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kidentityproxymodelwidget.h"

#include <QHBoxLayout>
#include <QIdentityProxyModel>
#include <QSplitter>
#include <QTreeView>
#include <dynamictreemodel.h>
#include <kbreadcrumbselectionmodel.h>

#include "dynamictreewidget.h"

#include "modeltest.h"

KIdentityProxyModelWidget::KIdentityProxyModelWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    QSplitter *splitter = new QSplitter(this);
    layout->addWidget(splitter);

    DynamicTreeModel *rootModel = new DynamicTreeModel(this);

    DynamicTreeWidget *treeWidget = new DynamicTreeWidget(rootModel, splitter);
    treeWidget->setInitialTree(
        QLatin1String(" - 1"
                      " - 2"
                      " - - 3"
                      " - - 4"
                      " - - 5"
                      " - 6"
                      " - 7"
                      " - - 8"
                      " - - - 9"
                      " - - - 10"
                      " - - 11")
        //     " - - 12"
        //     " - 13"
        //     " - 14"
        //     " - 15"
        //     " - - 16"
        //     " - - - 17"
        //     " - - - 18"
        //     " - 19"
        //     " - 20"
        //     " - 21"
    );

    QIdentityProxyModel *proxy = new QIdentityProxyModel(this);
    proxy->setSourceModel(rootModel);

    QTreeView *view1 = new QTreeView(splitter);
    view1->setModel(proxy);
    view1->expandAll();
}

#include "moc_kidentityproxymodelwidget.cpp"
