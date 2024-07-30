/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BREADCRUMBNAVIGATION_WIDGET_H
#define BREADCRUMBNAVIGATION_WIDGET_H

#include "klinkitemselectionmodel.h"
#include <kselectionproxymodel.h>

#include <QItemSelection>
#include <QLabel>
#include <QWidget>

class CurrentItemLabel : public QLabel
{
    Q_OBJECT
public:
    CurrentItemLabel(QAbstractItemModel *model, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

private Q_SLOTS:
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void modelReset();

private:
    QAbstractItemModel *m_model;
};

class KBreadcrumbNavigationProxyModel : public KSelectionProxyModel
{
    Q_OBJECT
public:
    KBreadcrumbNavigationProxyModel(QItemSelectionModel *selectionModel, QObject *parent = nullptr);

    void setShowHiddenAscendantData(bool showHiddenAscendantData);
    bool showHiddenAscendantData() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    bool m_showHiddenAscendantData;
};

class KNavigatingProxyModel : public KSelectionProxyModel
{
    Q_OBJECT
public:
    KNavigatingProxyModel(QItemSelectionModel *selectionModel, QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *sourceModel) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private Q_SLOTS:
    void modelReset();
    void updateNavigation();
    void navigationSelectionChanged(const QItemSelection &, const QItemSelection &);

private:
private:
    using KSelectionProxyModel::setFilterBehavior;

    QItemSelectionModel *m_selectionModel;
};

class KForwardingItemSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    enum Direction {
        Forward,
        Reverse,
    };
    KForwardingItemSelectionModel(QAbstractItemModel *model, QItemSelectionModel *selectionModel, QObject *parent = nullptr);
    KForwardingItemSelectionModel(QAbstractItemModel *model, QItemSelectionModel *selectionModel, Direction direction, QObject *parent = nullptr);

    void select(const QModelIndex &index, SelectionFlags command) override;
    void select(const QItemSelection &selection, SelectionFlags command) override;

private Q_SLOTS:
    void navigationSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QItemSelectionModel *m_selectionModel;
    Direction m_direction;
};

class BreadcrumbNavigationWidget : public QWidget
{
    Q_OBJECT
public:
    BreadcrumbNavigationWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
};

#endif
