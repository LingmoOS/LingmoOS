// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OEMDIALOG_H
#define OEMDIALOG_H

#include <DDialog>
#include <DStandardItem>
#include <DSuggestButton>
#include <QCloseEvent>

class ValueHandler;
class QTreeView;
class ConfigGetter;
DWIDGET_USE_NAMESPACE

class OEMDialog : public DDialog {
    Q_OBJECT
public:
    explicit OEMDialog(QWidget *parent = nullptr);
    void loadData(const QString &language);

public Q_SLOTS:
    void treeItemChanged(DStandardItem *item);
    void saveFiles();
    void displayChangedResult();
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    void createJsonFile(const QString &fileName, const QList<DStandardItem *> &items);
    QWidget* getItemWidget(ConfigGetter *getter, DStandardItem *item);
    void saveOverridesFiles(const QString &dirName);
    void saveCSVFile(const QString &dirName);
private:
    QTreeView *m_exportView = nullptr;
    QStandardItemModel *m_model = nullptr;
    DSuggestButton *m_exportBtn = nullptr;
    QScopedPointer<ValueHandler> m_getter;
    QMap<QString, QList<DStandardItem *>> m_overrides;
};

#endif // OEMDIALOG_H
