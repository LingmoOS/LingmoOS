// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <DDialog>
#include <DPasswordEdit>

#include <QWindow>
#include <QComboBox>

#include <PolkitQt1/Identity>
#include <PolkitQt1/ActionDescription>

namespace PolkitQt1 {
class Details;
}

DWIDGET_USE_NAMESPACE

class QDialogButtonBox;
class QLabel;
class QLineEdit;

class AuthDialog : public DDialog
{
    Q_OBJECT
public:
    enum AuthMode {
        FingerPrint = 0,
        Password = 1
    };

    enum AuthStatus {
        None = -1,
        WaitingInput,
        Authenticating,
        Completed,
    };

    AuthDialog(const QString &message,
               const QString &iconName);
    ~AuthDialog();

    void setError(const QString &error, bool alertImmediately = false);
    void setRequest(const QString &request, bool requiresAdmin);
    void authenticationFailure(bool &isLock);
    void createUserCB(const PolkitQt1::Identity::List &identities);

    void setAuthInfo(const QString &info);

    void addOptions(QButtonGroup *bg);

    QString password() const;
    void lock();

    PolkitQt1::Identity selectedAdminUser() const;

    bool hasOpenSecurity();
    bool hasSecurityHighLever(QString userName);
    void setInAuth(AuthStatus authStatus);

signals:
    void adminUserSelected(PolkitQt1::Identity);

protected:
    bool event(QEvent *event) override;

private:
    void initUI();
    int getLockLimitTryNum();
    bool passwordIsExpired(PolkitQt1::Identity identity);

private slots:
    void on_userCB_currentIndexChanged(int index);

private:
    QString m_appname;
    QString m_message;
    QString m_iconName;

    QComboBox *m_adminsCombo;
    DPasswordEdit *m_passwordInput;

    int m_numTries;
    int m_lockLimitTryNum;

    void setupUI();
    void showErrorTip();

    QString m_errorMsg;
    AuthStatus m_authStatus;
};
#endif // AUTHDIALOG_H
