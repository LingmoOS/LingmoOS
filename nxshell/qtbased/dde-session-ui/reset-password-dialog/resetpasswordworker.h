// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESETPASSWORDWORKER_H
#define RESETPASSWORDWORKER_H

#include "org_deepin_dde_accounts1_user.h"
#include "org_deepin_dde_accounts1.h"

using Accounts = org::deepin::dde::Accounts1;
using AccountsUser = org::deepin::dde::accounts1::User;

typedef QMap<int, QString> SecurityQuestionAnswers;

class ResetPasswordWorker : public QObject
{
    Q_OBJECT

public:
    explicit ResetPasswordWorker(const QString& userName, QObject *parent = 0);

Q_SIGNALS:
    void getSecurityQuestionsReplied(const QList<int> securityQuestions);
    void verifySecretQuestionsReplied(const QList<int> securityQuestions);
    void requestBindCheckUbidReplied(const QString& ubid);
    void requestBindCheckReplied(int result);
    void requestVerficationCodeCountReplied(int count);
    void requestVerficationCodeReplied(int result);
    void requestVerifyVerficationCodeReplied(int result);

public Q_SLOTS:
    void getSecurityQuestions();
    void setPasswordHint(const QString &passwordHint);
    void verifySecretQuestions(const QMap<int, QString> &securityQuestions);
    void asyncBindCheck();
    void asyncRequestVerficationCode(const QString &phoneEmail);
    void verifyVerficationCode(const QString &phoneEmail, const QString &code);

private:
    int bindCheck();
    int requestVerficationCode(const QString &phoneEmail);
    int parseError(const QString& errorMsg);

private:
    Accounts *m_accountInter;
    AccountsUser *m_userInter;
    QDBusInterface *m_userQInter;
    QDBusInterface *m_syncHelperInter;
    QString m_ubid;
    QString m_userPath;
};


#endif // RESETPASSWORDWORKER_H
