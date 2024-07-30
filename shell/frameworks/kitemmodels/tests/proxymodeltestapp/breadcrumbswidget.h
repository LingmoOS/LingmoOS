/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BREADCRUMBS_WIDGET_H
#define BREADCRUMBS_WIDGET_H

#include <QItemSelection>
#include <QWidget>
#include <kselectionproxymodel.h>

#include "klinkitemselectionmodel.h"

class MultiSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    MultiSelectionModel(QAbstractItemModel *model, QList<QItemSelectionModel *> selectionModels, QObject *parent = nullptr);

public:
    void select(const QModelIndex &index, SelectionFlags command) override;
    void select(const QItemSelection &selection, SelectionFlags command) override;

private:
    QList<QItemSelectionModel *> m_selectionModels;
};

class BreadcrumbsWidget : public QWidget
{
    Q_OBJECT
public:
    BreadcrumbsWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
};

#endif
