// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/modules/common/gsettingkey.h"

#include <QObject>

class InvokerFactoryInterface;
class DBusInvokerInterface;
class SettingsInvokerInterface;

class LoginSafetyModel : public QObject
{
    Q_OBJECT
public:
    explicit LoginSafetyModel(InvokerFactoryInterface *invokeFactory, QObject *parent = nullptr);
    ~LoginSafetyModel();

    // 开启/关闭密码限制策略
    void setPwdLimitPolicyEnable(const bool &enable);
    // 获取登录安全等级
    int getPwdLimitLevel() const;
    // 设置登录安全等级
    void setPwdLimitLevel(const int level);
    // 获取密码到期提醒截止天数类型
    int getPwdChangeDeadlineType() const;
    // 设置密码到期提醒截止天数类型
    void setPwdChangeDeadlineType(const int &type);
    // 打开控制中心账号页
    void jumpToAccountSettingPage();
Q_SIGNALS:
    // 设置登录安全等级完成信号
    void setPwdLimitedLevelFinished(int level);

private:
    SettingsInvokerInterface *m_gSettingsInvokerInter;
    DBusInvokerInterface *m_defenderDataInvokerInter;
    DBusInvokerInterface *m_controlCenterInvokerInter;
};
