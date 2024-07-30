/*
    SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MODELCOMMANDERWIDGET_H
#define MODELCOMMANDERWIDGET_H

#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

class DynamicTreeModel;
class ModelCommander;

class ModelCommanderWidget : public QWidget
{
    Q_OBJECT
public:
    ModelCommanderWidget(DynamicTreeModel *dynamicTreeModel, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

private Q_SLOTS:
    void initTest(QTreeWidgetItem *item);
    void executeTest(QTreeWidgetItem *item);
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void executeCurrentTest();
    void resetCurrentTest();

private:
    void init();

private:
    DynamicTreeModel *m_dynamicTreeModel;
    ModelCommander *m_modelCommander;
    QTreeWidget *m_treeWidget;
    QPushButton *m_executeButton;
};

#endif
