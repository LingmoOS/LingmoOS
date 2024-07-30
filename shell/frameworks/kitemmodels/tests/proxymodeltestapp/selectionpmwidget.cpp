/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "selectionpmwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QTreeView>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include "kselectionproxymodel.h"

SelectionProxyWidget::SelectionProxyWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    QSplitter *splitter = new QSplitter(this);
    layout->addWidget(splitter);

    m_rootModel = new DynamicTreeModel(this);

    DynamicTreeWidget *dynTreeWidget = new DynamicTreeWidget(m_rootModel, splitter);

    dynTreeWidget->setInitialTree(
        QLatin1String("- 1"
                      "- 2"
                      "- - 3"
                      "- - 3"
                      "- - - 4"
                      "- - - 4"
                      "- - - - 4"
                      "- - 4"
                      "- - 5"
                      "- - - 4"
                      "- - - - 4"
                      "- - 5"
                      "- 6"
                      "- 7"
                      "- - 8"
                      "- - - 9"
                      "- - - 10"
                      "- - - - 9"
                      "- - - - - 10"
                      "- - - - - - 9"
                      "- - - - - - 10"
                      "- - - - - - - 9"
                      "- - - - - - - - 10"
                      "- - - - - - - - 9"
                      "- - - - - - - 10"
                      "- - - - - 9"
                      "- - - - - 9"
                      "- - - - - 9"
                      "- - - - - 10"
                      "- - - - - - 9"
                      "- - - - - - 10"
                      "- - - - - 9"
                      "- - - - - 9"
                      "- - - - - 9"
                      "- - - - - 10"
                      "- - - - - - 9"
                      "- - - - - - 10"
                      "- - - - 10"
                      "- - 11"
                      "- - 12"
                      "- 13"
                      "- 14"
                      "- 15"
                      "- - 16"
                      "- - - 17"
                      "- - - 18"
                      "- 19"
                      "- 20"
                      "- 21"));

    QTreeView *selectionTree = createLabelledView(QStringLiteral("Selection"), splitter);
    selectionTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectionTree->setModel(m_rootModel);
    selectionTree->expandAll();

#define SUBTREES
#define SUBTREEROOTS
#define SUBTREESWITHOUTROOTS
#define EXACTSELECTION
#define CHILDRENOFEXACTSELECTION

#ifdef SUBTREES
    KSelectionProxyModel *selectedBranchesModel = new KSelectionProxyModel(selectionTree->selectionModel(), this);
    selectedBranchesModel->setSourceModel(m_rootModel);
    selectedBranchesModel->setFilterBehavior(KSelectionProxyModel::SubTrees);

    QTreeView *selectedBranchesView = createLabelledView(QStringLiteral("SubTrees"), splitter);
    selectedBranchesView->setModel(selectedBranchesModel);
#endif

#ifdef SUBTREEROOTS
    KSelectionProxyModel *selectedBranchesRootsModel = new KSelectionProxyModel(selectionTree->selectionModel(), this);
    selectedBranchesRootsModel->setSourceModel(m_rootModel);
    selectedBranchesRootsModel->setFilterBehavior(KSelectionProxyModel::SubTreeRoots);

    QTreeView *selectedBranchesRootsView = createLabelledView(QStringLiteral("SubTreeRoots"), splitter);
    selectedBranchesRootsView->setModel(selectedBranchesRootsModel);
#endif

#ifdef SUBTREESWITHOUTROOTS
    KSelectionProxyModel *selectedBranchesChildrenModel = new KSelectionProxyModel(selectionTree->selectionModel(), this);
    selectedBranchesChildrenModel->setSourceModel(m_rootModel);
    selectedBranchesChildrenModel->setFilterBehavior(KSelectionProxyModel::SubTreesWithoutRoots);

    QTreeView *selectedBranchesChildrenView = createLabelledView(QStringLiteral("SubTreesWithoutRoots"), splitter);
    selectedBranchesChildrenView->setModel(selectedBranchesChildrenModel);
#endif

#ifdef EXACTSELECTION
    KSelectionProxyModel *onlySelectedModel = new KSelectionProxyModel(selectionTree->selectionModel(), this);
    onlySelectedModel->setSourceModel(m_rootModel);
    onlySelectedModel->setFilterBehavior(KSelectionProxyModel::ExactSelection);

    QTreeView *onlySelectedView = createLabelledView(QStringLiteral("ExactSelection"), splitter);
    onlySelectedView->setModel(onlySelectedModel);
#endif

#ifdef CHILDRENOFEXACTSELECTION
    KSelectionProxyModel *onlySelectedChildrenModel = new KSelectionProxyModel(selectionTree->selectionModel(), this);
    onlySelectedChildrenModel->setSourceModel(m_rootModel);
    onlySelectedChildrenModel->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);

    QTreeView *onlySelectedChildrenView = createLabelledView(QStringLiteral("ChildrenOfExactSelection"), splitter);
    onlySelectedChildrenView->setModel(onlySelectedChildrenModel);
#endif
}

QTreeView *SelectionProxyWidget::createLabelledView(const QString &labelText, QWidget *parent)
{
    QWidget *labelledTreeWidget = new QWidget(parent);
    QVBoxLayout *layout = new QVBoxLayout(labelledTreeWidget);

    QLabel *label = new QLabel(labelText, labelledTreeWidget);
    QTreeView *treeview = new QTreeView(labelledTreeWidget);

    layout->addWidget(label);
    layout->addWidget(treeview);
    return treeview;
}
