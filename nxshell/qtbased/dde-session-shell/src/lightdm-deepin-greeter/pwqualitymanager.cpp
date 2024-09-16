// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pwqualitymanager.h"

#include <DSysInfo>

DCORE_USE_NAMESPACE

const bool IsServerSystem = (DSysInfo::UosServer ==  DSysInfo::uosType());

PwqualityManager::PwqualityManager(QObject *parent)
    : QObject(parent)
{
}


/**
* @brief PwqualityManager::instance 构造一个 单例
* @return 返回一个静态实例
*/
PwqualityManager *PwqualityManager::instance()
{
    static PwqualityManager pwquality;
    return &pwquality;
}

/**
* @brief PwqualityManager::verifyPassword 校验密码
* @param password 带检密码字符串
* @return 错误类型
*/
PwqualityManager::ERROR_TYPE PwqualityManager::verifyPassword(const QString &user, const QString &password, CheckType checkType)
{
    switch (checkType) {
    case PwqualityManager::Default: {
        ERROR_TYPE error = deepin_pw_check(user.toLocal8Bit().data(), password.toLocal8Bit().data(), LEVEL_STRICT_CHECK, nullptr);

        if (error == PW_ERR_PW_REPEAT) {
            error = PW_NO_ERR;
        }
        return error;
    }
    case PwqualityManager::Grub2: {
        // LEVEL_STRICT_CHECK?
        ERROR_TYPE error = deepin_pw_check_grub2(user.toLocal8Bit().data(), password.toLocal8Bit().data(), LEVEL_STANDARD_CHECK, nullptr);

        if (error == PW_ERR_PW_REPEAT) {
            error = PW_NO_ERR;
        }
        return error;
    }
    }

    return PW_NO_ERR;
}

/**
 * @brief PwqualityManager::GetNewPassWdLevel
 * @param password
 * @return 获取密码强度等级
 */
PASSWORD_LEVEL_TYPE PwqualityManager::newPassWdLevel(const QString &password) const
{
    return get_new_passwd_strength_level(password.toLocal8Bit().data());
}

QString PwqualityManager::getErrorTips(PwqualityManager::ERROR_TYPE type, CheckType checkType)
{
    const int palimdromeNum = (checkType == Default ? get_pw_palimdrome_num(LEVEL_STRICT_CHECK) : get_pw_palimdrome_num_grub2(LEVEL_STRICT_CHECK));
    const int monotoneNum = (checkType == Default ? get_pw_monotone_character_num(LEVEL_STRICT_CHECK) : get_pw_monotone_character_num_grub2(LEVEL_STRICT_CHECK));
    const int consecutiveNum = (checkType == Default ? get_pw_consecutive_same_character_num(LEVEL_STRICT_CHECK) : get_pw_consecutive_same_character_num_grub2(LEVEL_STRICT_CHECK));
    const int passwordMinLen = (checkType == Default ? get_pw_min_length(LEVEL_STRICT_CHECK) : get_pw_min_length_grub2(LEVEL_STRICT_CHECK));
    const int passwordMaxLen = (checkType == Default ? get_pw_max_length(LEVEL_STRICT_CHECK) : get_pw_max_length_grub2(LEVEL_STRICT_CHECK));

    //通用校验规则
    QMap<int, QString> PasswordHintsMap = {
        { PW_ERR_PASSWORD_EMPTY, tr("Password cannot be empty") },
        { PW_ERR_LENGTH_SHORT, tr("Password must have at least %1 characters").arg(passwordMinLen) },
        { PW_ERR_LENGTH_LONG, tr("Password must be no more than %1 characters").arg(passwordMaxLen) },
        { PW_ERR_CHARACTER_INVALID, tr("Password can only contain English letters (case-sensitive), numbers or special symbols (~`!@#$%^&*()-_+=|\\{}[]:\"'<>,.?/)") },
        { PW_ERR_PALINDROME, tr("No more than %1 palindrome characters please").arg(palimdromeNum) },
        { PW_ERR_PW_MONOTONE, tr("No more than %1 monotonic characters please").arg(monotoneNum) },
        { PW_ERR_PW_CONSECUTIVE_SAME, tr("No more than %1 repeating characters please").arg(consecutiveNum) },
    };

    //服务器版校验规则
    if (IsServerSystem) {
        PasswordHintsMap[PW_ERR_CHARACTER_INVALID] = tr("Password must contain uppercase letters, lowercase letters, numbers and symbols (~`!@#$%^&*()-_+=|\\{}[]:\"'<>,.?/)");
        PasswordHintsMap[PW_ERR_PALINDROME] = tr("Password must not contain more than 4 palindrome characters");
        PasswordHintsMap[PW_ERR_WORD] = tr("Do not use common words and combinations as password");
        PasswordHintsMap[PW_ERR_PW_MONOTONE] = tr("Create a strong password please");
        PasswordHintsMap[PW_ERR_PW_CONSECUTIVE_SAME] = tr("Create a strong password please");
        PasswordHintsMap[PW_ERR_PW_FIRST_UPPERM] = tr("Do not use common words and combinations as password");
    }

    //规则校验以外的情况统一返回密码不符合安全要求
    if (PasswordHintsMap.value(type).isEmpty()) {
        PasswordHintsMap[type] = tr("It does not meet password rules");
    }

    return PasswordHintsMap.value(type);
}
