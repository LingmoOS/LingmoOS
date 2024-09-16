// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PWQUALITYMANAGER_H
#define PWQUALITYMANAGER_H

#include <QObject>

#include "deepin_pw_check.h"

class PwqualityManager : public QObject
{
    Q_OBJECT

public:
    typedef PW_ERROR_TYPE ERROR_TYPE;

    enum CheckType {
        Default,
        Grub2
    };
    static PwqualityManager* instance();

    ERROR_TYPE verifyPassword(const QString &user, const QString &password, CheckType checkType = Default);
    PASSWORD_LEVEL_TYPE newPassWdLevel(const QString &password) const;
    QString getErrorTips(ERROR_TYPE type, CheckType checkType = Default);

private:
    explicit PwqualityManager(QObject *parent = nullptr);
    PwqualityManager(const PwqualityManager &) = delete;
};

#endif // REMINDERDDIALOG_H
