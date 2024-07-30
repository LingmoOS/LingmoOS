/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "breadcrumbdirectionwidget.h"

#include <dynamictreemodel.h>
#include <kbreadcrumbselectionmodel.h>

#include <QEvent>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeView>

BreadcrumbDirectionWidget::BreadcrumbDirectionWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    DynamicTreeModel *rootModel = new DynamicTreeModel(this);

    ModelInsertCommand ins(rootModel);
    ins.setStartRow(0);
    ins.interpret(
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
                      "- 20"
                      "- 21"));
    ins.doCommand();

    QHBoxLayout *layout = new QHBoxLayout(this);
    QSplitter *splitter1 = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter1);
    QSplitter *splitter2 = new QSplitter(splitter1);
    QSplitter *splitter3 = new QSplitter(splitter1);

    QTreeView *view1 = new QTreeView(splitter2);
    view1->setModel(rootModel);
    view1->expandAll();
    view1->viewport()->setBackgroundRole(QPalette::Button);
    QTreeView *view2 = new QTreeView(splitter2);
    view2->setModel(rootModel);
    view2->expandAll();
    view2->viewport()->installEventFilter(this);
    QTreeView *view3 = new QTreeView(splitter3);
    view3->setModel(rootModel);
    view3->expandAll();
    QTreeView *view4 = new QTreeView(splitter3);
    view4->setModel(rootModel);
    view4->expandAll();
    view4->viewport()->installEventFilter(this);
    view4->viewport()->setBackgroundRole(QPalette::Button);

    KBreadcrumbSelectionModel *breadcrumbSelection1 = new KBreadcrumbSelectionModel(view2->selectionModel(), this);
    view1->setSelectionModel(breadcrumbSelection1);

    KBreadcrumbSelectionModel *breadcrumbSelection2 =
        new KBreadcrumbSelectionModel(view3->selectionModel(), KBreadcrumbSelectionModel::MakeBreadcrumbSelectionInOther, this);
    view4->setSelectionModel(breadcrumbSelection2);
}

bool BreadcrumbDirectionWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick || e->type() == QEvent::MouseButtonRelease) {
        return true;
    }
    return QObject::eventFilter(o, e);
}

#include "moc_breadcrumbdirectionwidget.cpp"
