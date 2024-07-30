/*
    SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATCHCHECKINGWIDGET_H343434
#define MATCHCHECKINGWIDGET_H343434

#include <QWidget>

class QTreeView;
class QLineEdit;
class QRadioButton;

class DynamicTreeWidget;

class MatchCheckingWidget : public QWidget
{
    Q_OBJECT
public:
    MatchCheckingWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

private Q_SLOTS:
    void matchChanged(const QString &matchData);

private:
    QLineEdit *m_lineEdit;
    DynamicTreeWidget *m_dynamicTreeWidget;
    QTreeView *m_selectionTreeView;
    QRadioButton *m_dynamicTreeRadioButton;
    QRadioButton *m_selectionModelRadioButton;
};

#endif
