/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#ifndef MULTIAPPLYDIALOG_H
#define MULTIAPPLYDIALOG_H

#include <QDialog>
#include <QList>

class TypesListItem;
class TypesListProxyItem;
class QTreeWidget;
class QTreeWidgetItem;

class MultiApplyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultiApplyDialog(const TypesListItem *source, const QList<TypesListItem *> &sourceItemList, QWidget *parent = nullptr);
    ~MultiApplyDialog() override;

    const QList<TypesListItem *> selected() const;

private:
    void load(const TypesListItem *source, const QList<TypesListItem *> &sourceItemList);

private Q_SLOTS:
    void onItemChanged(QTreeWidgetItem *, int column);

private:
    QTreeWidget *m_mimeTree;
    QList<TypesListProxyItem *> m_itemList;
    QList<TypesListItem *> m_selected;
    TypesListProxyItem *m_source = nullptr;
};

#endif
