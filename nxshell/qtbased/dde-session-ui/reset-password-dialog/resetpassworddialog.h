// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESETPASSWORDDIALOG_H
#define RESETPASSWORDDIALOG_H

#include <QLocalSocket>
#include <QStackedLayout>

#include <DDialog>

#include "org_deepin_dde_accounts1_user.h"
#include "org_deepin_dde_accounts1.h"

DWIDGET_USE_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DButtonBox;
DWIDGET_END_NAMESPACE

class UnionIDWidget;
class SecurityQuestionsWidget;
class ResetPasswordWorker;

class ResetPasswordDialog : public DDialog
{
    Q_OBJECT

public:
    explicit ResetPasswordDialog(QRect screenGeometry, const QString &userName, const QString &appName, const int &fd);
    ~ResetPasswordDialog() {}

    QRect screenGeometry() const;
    void setScreenGeometry(const QRect &screenGeometry);

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *o, QEvent *e);

private:
    void initWidget(const QString &userName);
    void initData();
    void startCount();
    void quit();
    const QString getPassword();

Q_SIGNALS:
    void requestSecurityQuestions();

private:
    void onCancelBtnClicked();
    void onResetPasswordBtnClicked();
    void updatePosition();

public Q_SLOTS:
    void onGetSecurityQuestionsReplied(const QList<int> securityQuestions);

private Q_SLOTS:
    void onReadFromServerChanged(int);

private:
    DButtonBox *m_buttonBox = nullptr;
    UnionIDWidget *m_UnionIDWidget;
    SecurityQuestionsWidget *m_SecurityQuestionsWidget;
    bool m_isSecurityQuestionsExist;
    QRect m_screenGeometry;
    org::deepin::dde::Accounts1 *m_accountInter;
    org::deepin::dde::accounts1::User *m_userInter;
    QFile filein;
    QString m_userPath;
    QString m_userName;
    QString m_appName;
    int m_fd;
    QTimer *m_monitorTimer;
    QLocalSocket *m_client;
    DDialog m_tipDialog;
    bool m_isClose;
    QStackedLayout *m_stackedLayout;
    ResetPasswordWorker *m_resetPasswordWorker;
    QList<int> m_securityQuestions;
};

class Manager : public QObject
{
    Q_OBJECT
public:
   explicit Manager(const QString &userName, const QString &appName, const int &fd);
    ~Manager() {}

    void start();

private Q_SLOTS:
    void setupDialog();
    void showDialog();

private:
    ResetPasswordDialog *m_dialog;
    QString m_usrName;
    QString m_appName;
    int m_fd;
};

#endif // ResetPasswordDialog
