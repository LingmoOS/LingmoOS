// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/gsettingkey.h"
#include "common/common.h"

#include <com_deepin_daemon_accounts.h>
#include <com_deepin_daemon_accounts_user.h>

#include <QObject>

// deepin-pw-check配置模板
const QString PwdConfStr = "[Password]\n"
                            "STRONG_PASSWORD = true\n"
                            "PASSWORD_MIN_LENGTH = %1\n"
                            "PASSWORD_MAX_LENGTH = 512\n"
                            "VALIDATE_POLICY = \"1234567890;abcdefghijklmnopqrstuvwxyz;ABCDEFGHIJKLMNOPQRSTUVWXYZ;~`!@#$%^&*()-_+=|\\{}[]:\"'<>,.?/\"\n"
                            "VALIDATE_REQUIRED = %2\n"
                            "PALINDROME_NUM = 0\n"
                            "WORD_CHECK = 0\n"
                            "MONOTONE_CHARACTER_NUM = 0\n"
                            "CONSECUTIVE_SAME_CHARACTER_NUM = 0\n"
                            "DICT_PATH = \n"
                            "FIRST_LETTER_UPPERCASE = false\n";

class QDBusInterface;
class QGSettings;
using AccountInter = com::deepin::daemon::Accounts;

class LoginSafetySrvModel : public QObject
{
    Q_OBJECT
public:
    explicit LoginSafetySrvModel(QObject *parent = nullptr);
    ~LoginSafetySrvModel();

    // 获取密码校验策略开启关闭状态
    bool getPwdLimitPolicyEnable();
    // 设置密码校验策略开启关闭状态
    void setPwdLimitPolicyEnable(const bool &enable);
    // 设置密码安全等级
    void setPwdLimitedLevel(const int &level);
    // 获取密码修改错误提示文字
    QString getPwdChangeError() const;

Q_SIGNALS:
    // 设置密码校验策略开启关闭状态完成信号
    void setPwdLimitPolicyEnableFinished(const bool &enable);
    // 设置密码安全等级完成信号
    void setPwdLimitedLevelFinished(const int &level);

private:
    // 距上次密码修改的天数
    qint64 daysToPasswdLastChanged();
    // 获取密码修改提醒截止天数
    qint64 getPwdChangeDeadline();
    // 显示密码修改提醒
    void showNeedChangePwdSysNotify(qint64 days);
    // 检查密码修改的天数
    void checkPwdLastChangeDays();

private:
    QGSettings *m_gSettings;
    QDBusInterface *m_pwdConfInter; // deepin-pw-check接口
    AccountInter *m_accountInter; // 账号接口
};
