/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef REPARENTINGPM_WIDGET_H
#define REPARENTINGPM_WIDGET_H

#include <QWidget>

#include "kreparentingproxymodel.h"

class DynamicTreeModel;

class ReparentingProxyModelWidget : public QWidget
{
    Q_OBJECT
public:
    ReparentingProxyModelWidget(QWidget *parent = nullptr);

private:
    DynamicTreeModel *m_rootModel;
};

#endif
