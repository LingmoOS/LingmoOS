// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REMDISABLEDIALOG_H
#define REMDISABLEDIALOG_H

#include "hmiscreen_interface.h"

#include <DDialog>

using HmiScreen = com::deepin::defender::hmiscreen;

DWIDGET_USE_NAMESPACE
class RemDisableDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit RemDisableDialog(const QString &sDkgName, const QString &sAppName, DDialog *parent = nullptr);
    ~RemDisableDialog() override;

private:
    // 初始化界面
    void initUi();

Q_SIGNALS:
    // 弹框关闭信号
    void onDialogClose();

private:
    QString      m_sAppName;
    QString      m_sPkgName;

    QTimer      *m_timer;
    int          m_elapsedSeconds;

    HmiScreen   *m_hmiScreen;
};

#endif // REMDISABLEDIALOG_H
