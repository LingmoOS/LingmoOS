/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "proxymodeltestwidget.h"

#include <QPushButton>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

#include "dynamictreemodel.h"
#include "kselectionproxymodel.h"
#include "modelcommander.h"
#if 0
#include "kdescendantsproxymodel.h"
#endif
#include "modelcommanderwidget.h"

ProxyModelTestWidget::ProxyModelTestWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QSplitter *splitter = new QSplitter(this);

    m_rootModel = new DynamicTreeModel(this);

    (void)new ModelCommanderWidget(m_rootModel, splitter);

    QTreeView *rootModelView = new QTreeView(splitter);
    rootModelView->setModel(m_rootModel);
    rootModelView->setSelectionMode(QTreeView::ExtendedSelection);

    KSelectionProxyModel *selProxyModel = new KSelectionProxyModel(rootModelView->selectionModel(), this);
    selProxyModel->setSourceModel(m_rootModel);
    selProxyModel->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);

    QTreeView *selModelView = new QTreeView(splitter);
    selModelView->setModel(selProxyModel);

#if 0
    KDescendantsProxyModel *descProxyModel = new KDescendantsProxyModel(this);
    descProxyModel->setSourceModel(m_rootModel);
    QTreeView *descProxyModelView = new QTreeView(splitter);
    descProxyModelView ->setModel(descProxyModel);
#endif

    // Your Proxy Here?

    layout->addWidget(splitter);
}

#include "moc_proxymodeltestwidget.cpp"
