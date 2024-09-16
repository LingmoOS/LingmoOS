// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHINTERFACE_H
#define AUTHINTERFACE_H

#include "public_func.h"
#include "constants.h"
#include "dbuslogin1manager.h"

#include "authenticate_interface.h"
#include "accounts_interface.h"
#include "logined_interface.h"
#include "powermanager_interface.h"
#include "freedesktopdbus_interface.h"
#include "login1sessionself_interface.h"

#include <QJsonArray>
#include <QObject>
#include <QGSettings>
#include <memory>

using AccountsInter = org::deepin::dde::Accounts1;
using LoginedInter = org::deepin::dde::Logined;
using Login1SessionSelf = org::freedesktop::login1::Session;
using PowerManagerInter = org::deepin::dde::PowerManager1;
using DBusObjectInter = org::freedesktop::DBus;

using Authenticate = org::deepin::dde::Authenticate1;

class User;
class SessionBaseModel;

namespace Auth {
class AuthInterface : public QObject {
    Q_OBJECT
public:
    explicit AuthInterface(SessionBaseModel *const model, QObject *parent = nullptr);

    virtual void switchToUser(std::shared_ptr<User> user) = 0;

    virtual void setKeyboardLayout(std::shared_ptr<User> user, const QString &layout);
    virtual void onUserListChanged(const QStringList &list);
    virtual void onUserAdded(const QString &user);
    virtual void onUserRemove(const QString &user);

    enum SwitchUser {
        Always = 0,
        Ondemand,
        Disabled
    };

protected:
    void initDBus();
    void initData();
    void onLastLogoutUserChanged(uint uid);
    void onLoginUserListChanged(const QString &list);

    bool checkHaveDisplay(const QJsonArray &array);
    bool isLogined(uint uid);
    void checkConfig();
    void checkPowerInfo();
    bool isDeepin();
    QVariant getGSettings(const QString& node, const QString& key);

    template <typename T>
    T valueByQSettings(const QString & group,
                       const QString & key,
                       const QVariant &failback) {
        return findValueByQSettings<T>(DDESESSIONCC::session_ui_configs,
                                       group,
                                       key,
                                       failback);
    }

protected:
    SessionBaseModel*  m_model;
    AccountsInter *    m_accountsInter;
    LoginedInter*      m_loginedInter;
    DBusLogin1Manager* m_login1Inter;
    Login1SessionSelf* m_login1SessionSelf = nullptr;
    PowerManagerInter* m_powerManagerInter;
    Authenticate*      m_authenticateInter;
    DBusObjectInter*   m_dbusInter;
    QGSettings*        m_gsettings = nullptr;
    uint               m_lastLogoutUid;
    uint               m_currentUserUid;
    std::list<uint>    m_loginUserList;
};
}  // namespace Auth

#endif  // AUTHINTERFACE_H
