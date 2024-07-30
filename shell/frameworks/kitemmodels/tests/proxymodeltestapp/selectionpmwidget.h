/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef SELECTION_PM_WIDGET_H
#define SELECTION_PM_WIDGET_H

#include <QWidget>

class QTreeView;

class DynamicTreeModel;

class SelectionProxyWidget : public QWidget
{
public:
    SelectionProxyWidget(QWidget *parent = nullptr);

protected:
    QTreeView *createLabelledView(const QString &labelText, QWidget *parent);

private:
    DynamicTreeModel *m_rootModel;
};

#endif
