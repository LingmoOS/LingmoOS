/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef STATESAVERWIDGET_H
#define STATESAVERWIDGET_H

#include <QWidget>

#include <KViewStateSaver>

class QTreeView;

class DynamicTreeWidget;

class DynamicTreeStateSaver : public KViewStateSaver
{
    Q_OBJECT
public:
    DynamicTreeStateSaver(QObject *parent = nullptr);

protected:
    /* reimp */ QModelIndex indexFromConfigString(const QAbstractItemModel *model, const QString &key) const;
    /* reimp */ QString indexToConfigString(const QModelIndex &index) const;
};

class StateSaverWidget : public QWidget
{
    Q_OBJECT
public:
    StateSaverWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~StateSaverWidget();

private Q_SLOTS:
    void saveState();
    void restoreState();

private:
    DynamicTreeWidget *m_dynamicTreeWidget;
    QTreeView *m_view;
};

#endif
