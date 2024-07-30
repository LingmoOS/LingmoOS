/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PROXYMODELTESTWIDGET_H
#define PROXYMODELTESTWIDGET_H

#include <QWidget>

class DynamicTreeModel;
class ModelCommander;
class QPushButton;

class ProxyModelTestWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProxyModelTestWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

private:
    DynamicTreeModel *m_rootModel;
};

#endif
