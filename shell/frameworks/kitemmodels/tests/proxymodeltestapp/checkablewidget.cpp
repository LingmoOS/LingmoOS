/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "checkablewidget.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeView>

#include "dynamictreemodel.h"
#include <kcheckableproxymodel.h>
#include <kselectionproxymodel.h>

CheckableWidget::CheckableWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    QSplitter *vSplitter = new QSplitter(this);
    layout->addWidget(vSplitter);

    DynamicTreeModel *rootModel = new DynamicTreeModel(this);

    ModelInsertCommand *insert = new ModelInsertCommand(rootModel, this);
    insert->setStartRow(0);
    insert->interpret(
        QLatin1String("- 1"
                      "- 1"
                      "- 1"
                      "- - 2"
                      "- - 2"
                      "- - 2"
                      "- - 2"
                      "- 1"
                      "- 1"
                      "- 1"
                      "- - 2"
                      "- - - 3"
                      "- - - - 4"
                      "- - - - 4"
                      "- - - 3"
                      "- - - 3"
                      "- - - 3"
                      "- - 2"
                      "- - 2"
                      "- - 2"
                      "- 1"
                      "- 1"));
    insert->doCommand();

    QItemSelectionModel *checkModel = new QItemSelectionModel(rootModel, this);
    KCheckableProxyModel *checkable = new KCheckableProxyModel(this);
    checkable->setSourceModel(rootModel);
    checkable->setSelectionModel(checkModel);

    QTreeView *tree1 = new QTreeView(vSplitter);
    tree1->setModel(checkable);
    tree1->expandAll();

    KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(checkModel, this);
    selectionProxy->setFilterBehavior(KSelectionProxyModel::ExactSelection);
    selectionProxy->setSourceModel(rootModel);

    QTreeView *tree2 = new QTreeView(vSplitter);
    tree2->setModel(selectionProxy);
}

#include "moc_checkablewidget.cpp"
