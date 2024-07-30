/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2015 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef SELECTIONINQML_WIDGET_H
#define SELECTIONINQML_WIDGET_H

#include <QWidget>

class QTreeView;

class DynamicTreeModel;

class SelectionInQmlWidget : public QWidget
{
public:
    SelectionInQmlWidget(QWidget *parent = nullptr);

private:
    DynamicTreeModel *m_rootModel;
};

#endif
