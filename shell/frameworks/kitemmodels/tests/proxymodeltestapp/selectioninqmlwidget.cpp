/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2015 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "selectioninqmlwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QSplitter>
#include <QTreeView>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include "kselectionproxymodel.h"

SelectionInQmlWidget::SelectionInQmlWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    QSplitter *splitter = new QSplitter(this);
    layout->addWidget(splitter);

    m_rootModel = new DynamicTreeModel(this);

    new DynamicTreeWidget(m_rootModel, splitter);

    QTreeView *selectionTree = new QTreeView(splitter);
    selectionTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectionTree->setModel(m_rootModel);
    selectionTree->expandAll();

    qmlRegisterType<KSelectionProxyModel>("KF5ItemModels", 1, 0, "SelectionProxyModel");

    QQuickWidget *quickView = new QQuickWidget(splitter);

    quickView->engine()->rootContext()->setContextProperty(QStringLiteral("_model"), m_rootModel);
    quickView->engine()->rootContext()->setContextProperty(QStringLiteral("_selectionModel"), selectionTree->selectionModel());

    quickView->setSource(QUrl::fromLocalFile(QLatin1String(SRC_DIR "/selection.qml")));
}
