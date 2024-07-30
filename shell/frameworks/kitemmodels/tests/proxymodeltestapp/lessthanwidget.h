/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef LESSTHANWIDGET_H
#define LESSTHANWIDGET_H

#include <QItemSelectionModel>
#include <QWidget>

#include "dynamictreemodel.h"

class ColoredTreeModel : public DynamicTreeModel
{
    Q_OBJECT
public:
    ColoredTreeModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setSelectionModel(QItemSelectionModel *selectionModel);

protected Q_SLOTS:
    void recolor(const QModelIndex &parent = QModelIndex());

private:
    QItemSelectionModel *m_selectionModel;
    QColor m_lessThanColour;
    QColor m_greaterThanColour;
};

class LessThanWidget : public QWidget
{
    Q_OBJECT
public:
    LessThanWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void insertGrid(QList<int> address);

private:
    ColoredTreeModel *m_coloredTreeModel;
};

#endif
