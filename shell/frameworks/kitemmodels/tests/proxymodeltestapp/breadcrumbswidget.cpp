/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "breadcrumbswidget.h"

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"

#include <QHBoxLayout>
#include <QListView>
#include <QSplitter>
#include <QTreeView>

#include "kbreadcrumbselectionmodel.h"
#include "kselectionproxymodel.h"

MultiSelectionModel::MultiSelectionModel(QAbstractItemModel *model, QList<QItemSelectionModel *> selectionModels, QObject *parent)
    : QItemSelectionModel(model, parent)
    , m_selectionModels(selectionModels)
{
}

void MultiSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    for (QItemSelectionModel *selectionModel : std::as_const(m_selectionModels)) {
        selectionModel->select(index, command);
    }
    QItemSelectionModel::select(index, command);
}

void MultiSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    for (QItemSelectionModel *selectionModel : std::as_const(m_selectionModels)) {
        selectionModel->select(selection, command);
    }
    QItemSelectionModel::select(selection, command);
}

BreadcrumbsWidget::BreadcrumbsWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    DynamicTreeModel *rootModel = new DynamicTreeModel(this);
    QSplitter *splitter = new QSplitter(this);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(splitter);

    DynamicTreeWidget *dynamicTree = new DynamicTreeWidget(rootModel, splitter);
    dynamicTree->treeView()->setSelectionMode(QAbstractItemView::SingleSelection);
    dynamicTree->setInitialTree(
        QLatin1String("- 1"
                      "- - 2"
                      "- - 2"
                      "- - - 3"
                      "- - - - 4"
                      "- - - - - 5"
                      "- - 2"
                      "- 6"
                      "- 6"
                      "- 6"
                      "- - 7"
                      "- - - 8"
                      "- - - 8"
                      "- - - - 9"
                      "- - - - - 10"
                      "- - - 8"
                      "- - - 8"
                      "- - 8"
                      "- 16"
                      "- - 17"
                      "- - - 18"
                      "- - - - 19"
                      "- - - - - 20"));

    QList<QItemSelectionModel *> selectionModelList;
    QItemSelectionModel *fullBreadcrumbSelectionModel = new QItemSelectionModel(rootModel, this);

    KBreadcrumbSelectionModel *fullBreadcrumbProxySelector = new KBreadcrumbSelectionModel(fullBreadcrumbSelectionModel, this);
    selectionModelList << fullBreadcrumbProxySelector;

    KSelectionProxyModel *fullBreadCrumbSelectionProxyModel = new KSelectionProxyModel(fullBreadcrumbSelectionModel, this);
    fullBreadCrumbSelectionProxyModel->setSourceModel(rootModel);
    fullBreadCrumbSelectionProxyModel->setFilterBehavior(KSelectionProxyModel::ExactSelection);

    QListView *fullBreadcrumbProxyView = new QListView(splitter);
    fullBreadcrumbProxyView->setModel(fullBreadCrumbSelectionProxyModel);

    QItemSelectionModel *breadcrumbOnlySelectionModel = new QItemSelectionModel(rootModel, this);

    KBreadcrumbSelectionModel *breadcrumbOnlyProxySelector = new KBreadcrumbSelectionModel(breadcrumbOnlySelectionModel, this);
    breadcrumbOnlyProxySelector->setActualSelectionIncluded(false);
    selectionModelList << breadcrumbOnlyProxySelector;

    KSelectionProxyModel *breadcrumbOnlySelectionProxyModel = new KSelectionProxyModel(breadcrumbOnlySelectionModel, this);
    breadcrumbOnlySelectionProxyModel->setSourceModel(rootModel);
    breadcrumbOnlySelectionProxyModel->setFilterBehavior(KSelectionProxyModel::ExactSelection);

    QListView *breadcrumbOnlyProxyView = new QListView(splitter);
    breadcrumbOnlyProxyView->setModel(breadcrumbOnlySelectionProxyModel);

    int selectionDepth = 2;

    QItemSelectionModel *thisAndAscendantsSelectionModel = new QItemSelectionModel(rootModel, this);

    KBreadcrumbSelectionModel *thisAndAscendantsProxySelector = new KBreadcrumbSelectionModel(thisAndAscendantsSelectionModel, this);
    thisAndAscendantsProxySelector->setBreadcrumbLength(selectionDepth);
    selectionModelList << thisAndAscendantsProxySelector;

    KSelectionProxyModel *thisAndAscendantsSelectionProxyModel = new KSelectionProxyModel(thisAndAscendantsSelectionModel, this);
    thisAndAscendantsSelectionProxyModel->setSourceModel(rootModel);
    thisAndAscendantsSelectionProxyModel->setFilterBehavior(KSelectionProxyModel::ExactSelection);

    QListView *thisAndAscendantsProxyView = new QListView(splitter);
    thisAndAscendantsProxyView->setModel(thisAndAscendantsSelectionProxyModel);

    QItemSelectionModel *ascendantsOnlySelectionModel = new QItemSelectionModel(rootModel, this);

    KBreadcrumbSelectionModel *ascendantsOnlyProxySelector = new KBreadcrumbSelectionModel(ascendantsOnlySelectionModel, this);
    ascendantsOnlyProxySelector->setActualSelectionIncluded(false);
    ascendantsOnlyProxySelector->setBreadcrumbLength(selectionDepth);
    selectionModelList << ascendantsOnlyProxySelector;

    KSelectionProxyModel *ascendantsOnlySelectionProxyModel = new KSelectionProxyModel(ascendantsOnlySelectionModel, this);
    ascendantsOnlySelectionProxyModel->setSourceModel(rootModel);
    ascendantsOnlySelectionProxyModel->setFilterBehavior(KSelectionProxyModel::ExactSelection);

    QListView *ascendantsOnlyProxyView = new QListView(splitter);
    ascendantsOnlyProxyView->setModel(ascendantsOnlySelectionProxyModel);

    MultiSelectionModel *multiSelectionModel = new MultiSelectionModel(rootModel, selectionModelList, this);
    dynamicTree->treeView()->setSelectionModel(multiSelectionModel);
}

#include "moc_breadcrumbswidget.cpp"
