// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETASKDIALOG_H
#define NETASKDIALOG_H

#include "datainterface_interface.h"
#include "monitornetflow_interface.h"

#include <DDialog>

DWIDGET_USE_NAMESPACE
using DataInterFaceServer = com::deepin::defender::datainterface;
using MonitorInterFaceServer = com::deepin::defender::MonitorNetFlow;

class NetAskDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit NetAskDialog(const QString &sDkgName, const QString &sAppName, const QString &sId, DDialog *parent = nullptr);
    ~NetAskDialog() override;

private:
    // 初始化界面
    void initUi();

Q_SIGNALS:
    // 弹框关闭信号
    void onDialogClose();

private:
    DataInterFaceServer *m_dataInterFaceServer; // 联网管控接口
    MonitorInterFaceServer *m_monitorInterFaceServer; // 流量监控dbus类

    const QString m_sAppName;
    const QString m_sPkgName;
    const QString m_sId;

    QTimer *m_timer;
    int m_elapsedSeconds;
    bool m_bButtonClickedFlag;
};

#endif // NETASKDIALOG_H
