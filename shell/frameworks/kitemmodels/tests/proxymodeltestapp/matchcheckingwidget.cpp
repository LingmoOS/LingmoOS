/*
    SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matchcheckingwidget.h"

#include <QLineEdit>
#include <QRadioButton>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include <kselectionproxymodel.h>

#include <QDebug>

MatchCheckingWidget::MatchCheckingWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_lineEdit = new QLineEdit();

    connect(m_lineEdit, SIGNAL(textChanged(QString)), SLOT(matchChanged(QString)));

    m_dynamicTreeRadioButton = new QRadioButton(QStringLiteral("Dynamic Tree Model"), this);
    m_selectionModelRadioButton = new QRadioButton(QStringLiteral("Selection Model"), this);

    layout->addWidget(m_lineEdit);
    layout->addWidget(m_dynamicTreeRadioButton);
    layout->addWidget(m_selectionModelRadioButton);

    QSplitter *splitter = new QSplitter(this);
    layout->addWidget(splitter);
    DynamicTreeModel *dynamicTreeModel = new DynamicTreeModel(this);

    m_dynamicTreeWidget = new DynamicTreeWidget(dynamicTreeModel, this);

    splitter->addWidget(m_dynamicTreeWidget);

    KSelectionProxyModel *selectionProxyModel = new KSelectionProxyModel(m_dynamicTreeWidget->treeView()->selectionModel(), this);
    selectionProxyModel->setSourceModel(dynamicTreeModel);

    m_selectionTreeView = new QTreeView(this);
    m_selectionTreeView->setModel(selectionProxyModel);
    splitter->addWidget(m_selectionTreeView);
}

void MatchCheckingWidget::matchChanged(const QString &matchData)
{
    bool ok;
    int id = matchData.toInt(&ok);
    qDebug() << matchData << id << DynamicTreeModel::DynamicTreeModelId;
    if (!ok) {
        return;
    }

    QModelIndexList list;
    if (m_dynamicTreeRadioButton->isChecked()) {
        m_dynamicTreeWidget->treeView()->selectionModel()->clearSelection();
        list = m_dynamicTreeWidget->model()->match(m_dynamicTreeWidget->model()->index(0, 0), DynamicTreeModel::DynamicTreeModelId, id);
        qDebug() << list;
        for (const QModelIndex &idx : std::as_const(list)) {
            m_dynamicTreeWidget->treeView()->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent);
        }
    } else if (m_selectionModelRadioButton->isChecked()) {
        m_selectionTreeView->selectionModel()->clearSelection();
        list = m_selectionTreeView->model()->match(m_selectionTreeView->model()->index(0, 0), DynamicTreeModel::DynamicTreeModelId, id);
        qDebug() << list;
        for (const QModelIndex &idx : std::as_const(list)) {
            m_selectionTreeView->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent);
        }
    }
}

#include "moc_matchcheckingwidget.cpp"
