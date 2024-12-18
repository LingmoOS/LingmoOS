// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESTARTDEFENDERDIALOG_H
#define RESTARTDEFENDERDIALOG_H

#include "daemonservice_interface.h"
#include <DDialog>

// 心跳服务
using DaemonService = com::lingmo::defender::daemonservice;

DWIDGET_USE_NAMESPACE
class RestartDefenderDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit RestartDefenderDialog(DDialog *parent = nullptr);
    ~RestartDefenderDialog() override;

private:
    // 初始化界面
    void initUi();

Q_SIGNALS:
    // 发送重启信号
    void onRestartClose();
    // 弹框关闭信号
    void onDialogClose();

private Q_SLOTS:
    // 按钮点击槽
    void doButtonClicked(int index, const QString &text);

private:
    DaemonService *m_daemonservice; // 心跳接口
};

#endif // RESTARTDEFENDERDIALOG_H
