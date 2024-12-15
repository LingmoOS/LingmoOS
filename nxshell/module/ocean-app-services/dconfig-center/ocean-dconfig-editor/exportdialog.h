// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <DAbstractDialog>
#include <DDialog>
#include <DStandardItem>
#include <DSuggestButton>

class ValueHandler;
class QTreeView;

DWIDGET_USE_NAMESPACE

class ExportDialog : public DDialog {
    Q_OBJECT
public:
    explicit ExportDialog(QWidget *parent = nullptr);
    void loadData(const QString &language);

public Q_SLOTS:
    void treeItemChanged(QStandardItem *item);
    bool hasChildItemChecked();
    void saveFile();

private:
    void checkAllChild(QStandardItem *item, bool check);
    QList<QStringList> exportData();

private:
    QTreeView *m_exportView = nullptr;
    DSuggestButton *m_exportBtn = nullptr;
    QScopedPointer<ValueHandler> m_getter;
    QList<DStandardItem *> m_rootItems, m_childItems;
};

#endif // EXPORTDIALOG_H
