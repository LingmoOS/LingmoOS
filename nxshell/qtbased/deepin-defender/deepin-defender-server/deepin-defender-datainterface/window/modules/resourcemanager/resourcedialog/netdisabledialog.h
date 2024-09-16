// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETDISABLEDIALOG_H
#define NETDISABLEDIALOG_H

#include "hmiscreen_interface.h"

#include <DDialog>

DWIDGET_USE_NAMESPACE
using HmiScreen = com::deepin::defender::hmiscreen;
class NetDisableDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit NetDisableDialog(const QString &sDkgName, const QString &sAppName, DDialog *parent = nullptr);
    ~NetDisableDialog() override;

private:
    // 初始化界面
    void initUi();

Q_SIGNALS:
    // 发送联网管控显示弹框信号
    void requestShowNetControlWidget();

    // 弹框关闭信号
    void onDialogClose();

private:
    QString      m_sAppName;
    QString      m_sPkgName;

    QTimer      *m_timer;
    int          m_elapsedSeconds;

    HmiScreen    *m_hmiScreen;
};

#endif // NETDISABLEDIALOG_H
