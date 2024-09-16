// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settingdialog.h"

#include <DSearch>

#include <DMainWindow>
#include <DPushButton>
#include <DLineEdit>
#include <DListView>
#include <DSearchEdit>

#include <QStandardItemModel>

DSEARCH_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

public Q_SLOTS:
    void onSearchBtnClicked();
    void onActionTriggered(QAction *action);
    void onMatched();
    void onCompleted();
    void onStoped();

private:
    void initUI();
    void initMenu();
    void initConnect();

private:
    DSearch *searchObj { nullptr };

    DPushButton *searchBtn { nullptr };
    DSearchEdit *searchEdit { nullptr };
    DListView *resultView { nullptr };
    SettingDialog *settingDialog { nullptr };
    QStandardItemModel *model { nullptr };
};

#endif   // MAINWINDOW_H
