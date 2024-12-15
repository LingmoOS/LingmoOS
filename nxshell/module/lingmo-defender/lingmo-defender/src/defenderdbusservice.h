// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include "hmiscreen_adaptor.h"

class MainWindow;

class DefenderDBusService : public QObject
{
    Q_OBJECT
public:
    explicit DefenderDBusService(QObject *parent = nullptr);

Q_SIGNALS:

public Q_SLOTS:
    void Show();
    MainWindow *parent();
    void ShowModule(const QString &module);
    void ShowPage(const QString &module, const QString &page);
    void ExitApp();
    // 显示设置弹窗
    void showSettingDialog(const QString &sGroupKey);

    // 联网管控弹框
    void showFuncConnectNetControl(QString sPkgName);
    // 远程访问弹框
    void showFuncConnectRemControl(QString sPkgName);
    // 流量详情弹框
    void showFuncConnectDataUsage();

private:
    HmiscreenAdaptor *m_adaptor;
};
