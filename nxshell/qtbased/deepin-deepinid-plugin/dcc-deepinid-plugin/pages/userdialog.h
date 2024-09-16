// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <DDialog>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DLineEdit;
class DToolButton;
class DPasswordEdit;
class DSuggestButton;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class ReBindDlg;

class RegisterDlg:public DDialog
{
    Q_OBJECT
public:
    explicit RegisterDlg(QWidget *parent = nullptr);
Q_SIGNALS:
    void registerPasswd(const QString &strpwd);
private:
    DPasswordEdit *m_pwdEdit;
    DPasswordEdit *m_againEdit;
};

class VerifyDlg: public DDialog
{
    Q_OBJECT
public:
    explicit VerifyDlg(QWidget *parent = nullptr);

    void showAlert(const QString &alertMsg);
Q_SIGNALS:
    void forgetPasswd();

    void verifyPasswd(const QString &strpwd);
private:
    DPasswordEdit *m_pwdEdit;
};
//
class PhoneMailDlg: public DDialog
{
    Q_OBJECT
public:
    explicit PhoneMailDlg(QWidget *parent = nullptr);

    void setNumHolderText(const QString &holderText);

    void setInvalidTip(const QString &invalidTip);

    void initPhoneNumConstraint();

    void initMailNumConstraint();

    void showVerifyAlert();

    void setRebindInfo(const QString &account, const QString &key)
    {
        m_rebindAccount = account;
        m_rebindKey = key;
    }

    QString rebindAccount()
    {
        return m_rebindAccount;
    }

    QString rebindKey()
    {
        return m_rebindKey;
    }

    void verifyCoolDown(int seconds);

    void bindToLocal();
Q_SIGNALS:
    void sendVerify(const QString &strnum);

    void updatePhoneMail(const QString &strnum, const QString &verify);

    void rebindPhoneMail(const QString &strnum, const QString &verify, const QString &key);
private:
    DLineEdit *m_numEdit;
    DLineEdit *m_codeEdit;
    DSuggestButton *m_btnCode;
    QString m_invalidTip;
    QString m_rebindAccount;
    QString m_rebindKey;
    int m_coolDown;
};

class ResetPwdDlg:public DDialog
{
    Q_OBJECT
public:
    explicit ResetPwdDlg(QWidget *parent = nullptr);

    void showFailTip();
Q_SIGNALS:
    void resetPasswd(const QString &newpwd);
private:
    DPasswordEdit *m_pwdEdit;
    DPasswordEdit *m_againEdit;
};
//
class ReBindDlg:public DDialog
{
    Q_OBJECT
public:
    explicit ReBindDlg(QWidget *parent = nullptr);

    bool isChangeNum()
    {
        return m_changeNum;
    }

    void setTitleTip(const QString &strtitle);

    void setChangeTip(const QString &strChange);

    void setDialogMessage(const QString &msg);
private:
    bool m_changeNum;
};

#endif // USERDIALOG_H
