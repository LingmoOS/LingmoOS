// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SECURITYMANAGE_H
#define SECURITYMANAGE_H

#include "interface/namespace.h"
#include <QWidget>
#include <dtkwidget_global.h>
#include <QStandardItemModel>
#include "operation/syncmodel.h"
#include "operation/syncworker.h"
#include "userdialog.h"

DWIDGET_BEGIN_NAMESPACE
class DListView;
class DLabel;
class DLineEdit;
class DToolButton;
class DDialog;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class SingleItem;

class SecurityPage : public QWidget
{
    Q_OBJECT
    enum VerifyType:int
    {
        PhoneType = 1,
        MailType,
        BindAccountType,
        UnbindAccountType,
        PasswdType,
    };

public:
    explicit SecurityPage(QWidget *parent = nullptr);

    void setSyncModel(SyncModel *syncModel);

    void setSyncWorker(SyncWorker *worker);

    void onLogin();
Q_SIGNALS:
    void onUserLogout();
    void onChangeInfo();
private Q_SLOTS:
    void onBindSuccess();
    void onUserInfoChanged(const QVariantMap &infos);
private:
    void initUI();

    void initConnection();

    void initModelData();

    void initDialog();

    void initRegisterDialog(RegisterDlg *dlg);

    void initVerifyDialog(VerifyDlg *dlg);

    void initPhoneDialog(PhoneMailDlg *dlg);

    void initMailDialog(PhoneMailDlg *dlg);

    void initPhoneMailConnection(PhoneMailDlg *dlg);

    void initResetPwdDialog(ResetPwdDlg *dlg);

    bool verifyPasswd(VerifyType type);

    void openUserDialog(VerifyType type);
private:
    QString m_forgetUrl;
    QString m_wechatUrl;
    DDialog *m_unbindWeChatDlg;
    SyncModel *m_syncModel;
    SyncWorker *m_syncWorker;
    QString m_lastEnPwd;
    QWidget *m_phoneWidget;
    QWidget *m_accountWidget;
    QWidget *m_passwdWidget;
    SingleItem *m_itemPhone;
    SingleItem *m_itemMail;
    SingleItem *m_itemAccount;
    SingleItem *m_itemPasswd;
    QDBusInterface *m_clientService;
};

#endif // SECURITYMANAGE_H
