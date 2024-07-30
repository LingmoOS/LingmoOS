/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "statesaverwidget.h"

#include <QApplication>
#include <QSplitter>
#include <QTreeView>

#include <KConfig>
#include <KConfigGroup>
#include <ksharedconfig.h>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"

QModelIndex DynamicTreeStateSaver::indexFromConfigString(const QAbstractItemModel *model, const QString &key) const
{
    QModelIndexList list = model->match(model->index(0, 0), DynamicTreeModel::DynamicTreeModelId, key.toInt(), 1, Qt::MatchRecursive);
    if (list.isEmpty()) {
        return QModelIndex();
    }
    return list.first();
}

QString DynamicTreeStateSaver::indexToConfigString(const QModelIndex &index) const
{
    return index.data(DynamicTreeModel::DynamicTreeModelId).toString();
}

DynamicTreeStateSaver::DynamicTreeStateSaver(QObject *parent)
    : KViewStateSaver(parent)
{
}

StateSaverWidget::StateSaverWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    QSplitter *splitter = new QSplitter(this);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(splitter);

    DynamicTreeModel *model = new DynamicTreeModel(this);

    DynamicTreeWidget *dynamicTreeWidget = new DynamicTreeWidget(model, splitter);

    m_view = new QTreeView(splitter);
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_view->setModel(model);

    connect(model, SIGNAL(modelAboutToBeReset()), SLOT(saveState()));
    connect(model, SIGNAL(modelReset()), SLOT(restoreState()));
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(saveState()));

    restoreState();
}

StateSaverWidget::~StateSaverWidget()
{
    saveState();
}

void StateSaverWidget::saveState()
{
    DynamicTreeStateSaver saver;
    saver.setView(m_view);

    KConfigGroup cfg(KSharedConfig::openConfig(), "ExampleViewState");
    saver.saveState(cfg);
    cfg.sync();
}

void StateSaverWidget::restoreState()
{
    DynamicTreeStateSaver *saver = new DynamicTreeStateSaver;
    saver->setView(m_view);
    KConfigGroup cfg(KSharedConfig::openConfig(), "ExampleViewState");
    saver->restoreState(cfg);
}

#include "moc_statesaverwidget.cpp"
