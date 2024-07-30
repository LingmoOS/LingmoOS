/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SCRIPTABLEREPARENTINGWIDGET_H
#define SCRIPTABLEREPARENTINGWIDGET_H

#include <QJSValue>
#include <QWidget>

#include "kreparentingproxymodel.h"

class QComboBox;
class QTreeView;
class QPlainTextEdit;
class QJSEngine;

class ScriptableReparentingProxyModel : public KReparentingProxyModel
{
    Q_OBJECT
public:
    ScriptableReparentingProxyModel(QObject *parent = nullptr);

    bool isDescendantOf(const QModelIndex &ancestor, const QModelIndex &descendant) const override;

    void setImplementation(const QString &implementation);

private:
    QJSEngine *m_scriptEngine;
    mutable QJSValue m_implementationFunction;
};

class ScriptableReparentingWidget : public QWidget
{
    Q_OBJECT
public:
    ScriptableReparentingWidget(QAbstractItemModel *rootModel, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

private Q_SLOTS:
    void textChanged();
    void setExampleFunction(int index);

private:
    QComboBox *m_comboBox;
    ScriptableReparentingProxyModel *m_reparentingProxyModel;
    QTreeView *m_treeView;
    QPlainTextEdit *m_textEdit;
};

#endif // SCRIPTABLEREPARENTINGWIDGET_H
