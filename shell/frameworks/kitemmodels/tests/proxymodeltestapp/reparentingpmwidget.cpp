/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "reparentingpmwidget.h"

#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include "scriptablereparentingwidget.h"

ReparentingProxyModelWidget::ReparentingProxyModelWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QSplitter *vSplitter = new QSplitter(this);
    layout->addWidget(vSplitter);

    m_rootModel = new DynamicTreeModel(this);

    DynamicTreeWidget *dynamicTreeWidget = new DynamicTreeWidget(m_rootModel, vSplitter);
    dynamicTreeWidget->setInitialTree(
        QLatin1String("- 1"
                      "- 2"
                      "- - 3"
                      "- - - 4"
                      "- 5"
                      "- 6"
                      "- 7"));

    new ScriptableReparentingWidget(m_rootModel, vSplitter);
}

#include "moc_reparentingpmwidget.cpp"
