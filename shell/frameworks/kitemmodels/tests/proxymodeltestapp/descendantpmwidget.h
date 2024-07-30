/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef DESCENDANTPM_WIDGET_H
#define DESCENDANTPM_WIDGET_H

#include <QWidget>

class DynamicTreeModel;
class QTreeView;
class QLineEdit;
class QItemSelectionModel;

class KDescendantsProxyModel;
class KSelectionProxyModel;

class ModelEventLogger;

class DescendantProxyModelWidget : public QWidget
{
    Q_OBJECT
public:
    DescendantProxyModelWidget(QWidget *parent = nullptr);
    ~DescendantProxyModelWidget() override;

protected Q_SLOTS:
    void doMatch(const QString &matchData);
    void refreshMatch();

private:
    DynamicTreeModel *m_rootModel;
    ModelEventLogger *m_eventLogger;
    KDescendantsProxyModel *m_descProxyModel;
    KSelectionProxyModel *m_selectionProxyModel;
    QItemSelectionModel *m_itemSelectionModel;
    QTreeView *m_descView;
    QLineEdit *m_lineEdit;
};

#endif
