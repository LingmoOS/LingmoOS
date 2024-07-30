/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "descendantpmwidget.h"

#include <QSplitter>
#include <QTreeView>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include "kdescendantsproxymodel.h"
#include <QHBoxLayout>
#include <QLineEdit>

#include "modeleventlogger.h"

DescendantProxyModelWidget::DescendantProxyModelWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    QSplitter *vSplitter = new QSplitter(this);
    layout->addWidget(vSplitter);

    m_rootModel = new DynamicTreeModel(this);

    DynamicTreeWidget *dynTreeWidget = new DynamicTreeWidget(m_rootModel, vSplitter);

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

    m_eventLogger = new ModelEventLogger(m_rootModel, this);

    m_descProxyModel = new KDescendantsProxyModel(this);
    m_descProxyModel->setSourceModel(m_rootModel);

    //   KDescendantsProxyModel *descProxyModel2 = new KDescendantsProxyModel(this);
    //   descProxyModel2->setSourceModel(m_rootModel);
    //   descProxyModel2->setDisplayAncestorData(true);

    //   QTreeView *treeview = new QTreeView( vSplitter );
    //   treeview->setModel(m_rootModel);
    //   treeview->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_descView = new QTreeView(vSplitter);
    m_descView->setModel(m_descProxyModel);

    //   QTreeView *descView2 = new QTreeView( vSplitter );
    //   descView2->setModel(descProxyModel2);

    //   QWidget *w = new QWidget(vSplitter);
    //   QVBoxLayout *vLayout = new QVBoxLayout(w);
    //   QTreeView *matchView = new QTreeView(w);
    //   matchView->setModel(m_selectionProxyModel);
    //   m_lineEdit = new QLineEdit(w);
    //   connect(m_lineEdit, SIGNAL(textChanged(QString)), SLOT(doMatch(QString)));
    //   connect(m_descView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(refreshMatch()));

    //   vLayout->addWidget(m_lineEdit);
    //   vLayout->addWidget(matchView);
}

DescendantProxyModelWidget::~DescendantProxyModelWidget()
{
}

void DescendantProxyModelWidget::doMatch(const QString &matchData)
{
    Q_UNUSED(matchData);
#if 0
    m_itemSelectionModel->clearSelection();

    if (matchData.isEmpty()) {
        return;
    }

    QModelIndex start = m_descView->currentIndex();

    if (!start.isValid()) {
        start = m_descProxyModel->index(0, 0);
    }

    // TODO: get from user.
    int hits = -1;

    QModelIndexList matches = m_descProxyModel->match(start, Qt::DisplayRole, matchData, hits, Qt::MatchContains);

    Q_FOREACH (const QModelIndex &matchingIndex, matches) {
        m_itemSelectionModel->select(matchingIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
#endif
}

void DescendantProxyModelWidget::refreshMatch()
{
    doMatch(m_lineEdit->text());
}

#include "moc_descendantpmwidget.cpp"
