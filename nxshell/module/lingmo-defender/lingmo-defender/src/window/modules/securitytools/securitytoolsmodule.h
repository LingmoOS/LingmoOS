// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/interface/moduleinterface.h"
#include "window/modules/common/common.h"

class ModuleInterface;
class FrameProxyInterface;
class SecurityToolsWidget;
class StartupWidget;
class NetControlWidget;
class DataUsageModel;
class DataUsageWidget;
class UsbConnectionWork;
class UsbConnectionWidget;
// 登陆安全模块
class LoginSafetyModel;
class LoginSafetyWidget;

class SecurityToolsModule : public QObject, public ModuleInterface
{
    Q_OBJECT
public:
    explicit SecurityToolsModule(FrameProxyInterface *frame, QObject *parent = nullptr);
    ~SecurityToolsModule() override;

public:
    void initialize() override;
    void preInitialize() override;
    const QString name() const override;
    void active(int index) override;
    void deactive() override;

    // 删除安全工具子页面
    void deleteSunWidget();

    // 定位联网管控包名
    void setNetControlIndex(const QString &sPkgName);

public Q_SLOTS:
    void showDataUsageWidget();
    void showStartupWidget();
    void showNetControlWidget();
    void showUsbControlWidget();
    void showLoginSafetyWidget();

private:
    SecurityToolsWidget *m_securityToolsWidget;
    StartupWidget       *m_startupWidget;
    NetControlWidget    *m_netControlWidget;
    DataUsageModel      *m_dataUsageModel;
    DataUsageWidget     *m_dataUsageWidget;
    UsbConnectionWork   *m_usbConnectionWork;
    UsbConnectionWidget *m_usbConnectionWidget;
    // 登陆安全模块
    LoginSafetyModel    *m_loginSafetyModel;
    LoginSafetyWidget   *m_loginSafetyWidget;
};
