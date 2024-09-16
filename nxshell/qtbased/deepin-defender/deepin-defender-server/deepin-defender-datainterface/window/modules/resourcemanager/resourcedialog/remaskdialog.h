// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REMASKDIALOG_H
#define REMASKDIALOG_H

#include "datainterface_interface.h"
#include "monitornetflow_interface.h"

#include <DDialog>

DWIDGET_USE_NAMESPACE
using DataInterFaceServer = com::deepin::defender::datainterface;
using MonitorInterFaceServer = com::deepin::defender::MonitorNetFlow;

class RemAskDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit RemAskDialog(const QString &sDkgName, const QString &sAppName, const QString &sId, DDialog *parent = nullptr);
    ~RemAskDialog() override;

private:
    // 初始化界面
    void initUi();

Q_SIGNALS:
    // 弹框关闭信号
    void onDialogClose();

private:
    DataInterFaceServer *m_dataInterFaceServer; // 联网管控接口
    MonitorInterFaceServer *m_monitorInterFaceServer; // 流量监控dbus类

    QString m_sAppName;
    QString m_sPkgName;
    QString m_sId;

    QTimer *m_timer;
    int m_elapsedSeconds;

    bool m_bButtonClickedFlag;
};

#endif // REMASKDIALOG_H
