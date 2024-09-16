// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QtCore/qglobal.h>
#include <QDBusContext>
#include <QHostInfo>
#include <QDBusInterface>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  #include <QNetworkConfigurationManager>
#else
  #include <QNetworkInformation>
#endif

#include <DMainWindow>
#include <QNetworkReply>
#include "login_view.h"
#include "ipc/dbuslogin1manager.h"


namespace ddc
{
enum AuthorizationState
{
    Notauthorized,      //未授权
    Authorized,         //已授权
    Expired,            //已过期
    TrialAuthorized,    //试用期已授权
    TrialExpired        //试用期已过期
};

enum ErrCode
{
    Err_no,
    Err_Authorize,
    Err_LoginPageLoad,
    Err_CloseLoginWindow,
    Err_CloseClient,
};

class LoginWindowPrivate;
class LoginWindow: public Dtk::Widget::DMainWindow,
                   protected QDBusContext
{
Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.deepinid.Client")
public:
    explicit LoginWindow(QWidget *parent = Q_NULLPTR);
    ~LoginWindow() Q_DECL_OVERRIDE;

    void setURL(const QString &url);
    void load();

Q_SIGNALS:
    void loadError();

public Q_SLOTS:
    void onLoadError();

public Q_SLOTS:
    Q_SCRIPTABLE void Register(const QString &clientID,
                               const QString &service,
                               const QString &path,
                               const QString &interface);

    Q_SCRIPTABLE void Authorize(const QString &clientID,
                                const QStringList &scopes,
                                const QString &callback,
                                const QString &state);

    Q_SCRIPTABLE void AuthTerm(const QString &clientID);

    Q_SCRIPTABLE void LoadPage(const QString &pageUrl);

protected Q_SLOTS:
    void onLookupHost(QHostInfo host);
    void syncAppearanceProperties(QString str, QMap<QString, QVariant> map, QStringList list);
    void onSystemDown(bool isReady);

protected:
    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;
    bool event(QEvent *event) Q_DECL_OVERRIDE;

    bool windowloadingEnd = true;
    bool pageLoadOK = true;
    LoginView *m_loginView;
    QDBusInterface *appearance_ifc_;
    DBusLogin1Manager *login1_Manager_ifc_ = nullptr;
    QString clientID_last;
    // 自更新需求暂时屏蔽
//    UpdateClient* updateClient = nullptr;

private:
    QScopedPointer<LoginWindowPrivate> dd_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(dd_ptr), LoginWindow)
};

}
