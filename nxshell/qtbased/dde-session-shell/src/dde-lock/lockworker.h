// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCKWORKER_H
#define LOCKWORKER_H

#include "authinterface.h"
#include "dbushotzone.h"
#include "dbuslockservice.h"
#include "dbuslogin1manager.h"
#include "deepinauthframework.h"
#include "sessionbasemodel.h"
#include "huaweiswitchos_interface.h"
#include "userinfo.h"

#include "accounts_interface.h"
#include "logined_interface.h"
#include "accountsuser_interface.h"
#include "sessionmanager_interface.h"

#include <QObject>
#include <QWidget>

using AccountsInter = org::deepin::dde::Accounts1;
using UserInter = org::deepin::dde::accounts1::User;
using LoginedInter = org::deepin::dde::Logined;
using SessionManagerInter = org::deepin::dde::SessionManager1;
using HuaWeiSwitchOSInterface = com::huawei::switchos;

class SessionBaseModel;
class LockWorker : public Auth::AuthInterface
{
    Q_OBJECT
public:
    explicit LockWorker(SessionBaseModel *const model, QObject *parent = nullptr);

    void enableZoneDetected(bool disable);

    bool isLocked() const;

public slots:
    /* New authentication framework */
    void createAuthentication(const QString &account);
    void destroyAuthentication(const QString &account);
    void startAuthentication(const QString &account, const int authType);
    void endAuthentication(const QString &account, const int authType);
    void endAccountAuthentication(const QString &account, const int authType);
    void sendTokenToAuth(const QString &account, const int authType, const QString &token);

    void switchToUser(std::shared_ptr<User> user) override;
    void restartResetSessionTimer();
    void onAuthFinished();
    void onAuthStateChanged(const int type, const int state, const QString &message);
    void onFrameworkStateChanged(const int state);

    void disableGlobalShortcutsForWayland(const bool enable);

    void checkAccount(const QString &account);

private:
    void initConnections();
    void initData();
    void initConfiguration();

    void doPowerAction(const SessionBaseModel::PowerAction action);
    void setCurrentUser(const std::shared_ptr<User> user);
    void setLocked(const bool locked);

    // lock
    void handleServiceEvent(quint32 eventType, quint32 pid, const QString &username, const QString &message);
    void onUnlockFinished(bool unlocked);

private:
    bool m_isThumbAuth;
    bool m_canAuthenticate;
    DeepinAuthFramework *m_authFramework;
    DBusLockService *m_lockInter;
    DBusHotzone *m_hotZoneInter;
    SessionManagerInter *m_sessionManagerInter;
    HuaWeiSwitchOSInterface *m_switchosInterface;

    QMap<std::shared_ptr<User>, bool> m_lockUser;
    QTimer *m_resetSessionTimer;
    QTimer *m_limitsUpdateTimer;
    QString m_account;
    QDBusInterface *m_kglobalaccelInter;
    QDBusInterface *m_kwinInter;
};

#endif // LOCKWORKER_H
