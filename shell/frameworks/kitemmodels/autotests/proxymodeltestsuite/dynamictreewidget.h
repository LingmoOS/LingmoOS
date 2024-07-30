/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DYNAMICTREEWIDGET_H
#define DYNAMICTREEWIDGET_H

#include <QWidget>

#include "proxymodeltestsuite_export.h"

class QModelIndex;

class QComboBox;
class QPlainTextEdit;
class QTreeView;
class QRadioButton;

class DynamicTreeModel;

class PROXYMODELTESTSUITE_EXPORT DynamicTreeWidget : public QWidget
{
    Q_OBJECT
public:
    DynamicTreeWidget(DynamicTreeModel *rootModel, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void setInitialTree(const QString &treeString);

    DynamicTreeModel *model() const
    {
        return m_dynamicTreeModel;
    }
    QTreeView *treeView() const
    {
        return m_treeView;
    }
    QPlainTextEdit *textEdit() const
    {
        return m_textEdit;
    }

private Q_SLOTS:
    void currentChanged(int index);
    void setTreePredefine(int index);
    void setInsertSubTreePredefine(int index);

    void removeSelected();
    void insertSelected();
    void resetModel();

private:
    void stringToModel(const QString &treeString);
    QString modelTreeToString(int depth, const QModelIndex &parent);

private:
    enum Tab {
        EditTab,
        ViewTab,
    };

    QString m_initialString;
    DynamicTreeModel *m_dynamicTreeModel;
    QTreeView *m_treeView;
    QPlainTextEdit *m_textEdit;

    QPlainTextEdit *m_insertPatternTextEdit;
    QRadioButton *m_insertChildren;
    QRadioButton *m_insertSiblingsAbove;
    QRadioButton *m_insertSiblingsBelow;
    QComboBox *m_insertSubTreePredefines;
    QComboBox *m_treePredefines;
};

#endif
