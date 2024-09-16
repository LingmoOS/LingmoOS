// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lockworker.h"

#include "authcommon.h"
#include "sessionbasemodel.h"
#include "userinfo.h"

#include <DSysInfo>

#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>

#include <grp.h>
#include <libintl.h>
#include <pwd.h>
#include <unistd.h>

#define DOMAIN_BASE_UID 10000

using namespace Auth;
using namespace AuthCommon;
DCORE_USE_NAMESPACE

LockWorker::LockWorker(SessionBaseModel *const model, QObject *parent)
    : AuthInterface(model, parent)
    , m_isThumbAuth(false)
    , m_canAuthenticate(false)
    , m_authFramework(new DeepinAuthFramework(this))
    , m_lockInter(new DBusLockService("org.deepin.dde.LockService1", "/org/deepin/dde/LockService1", QDBusConnection::systemBus(), this))
    , m_hotZoneInter(new DBusHotzone("org.deepin.dde.Zone1", "/org/deepin/dde/Zone1", QDBusConnection::sessionBus(), this))
    , m_sessionManagerInter(new SessionManagerInter("org.deepin.dde.SessionManager1", "/org/deepin/dde/SessionManager1", QDBusConnection::sessionBus(), this))
    , m_switchosInterface(new HuaWeiSwitchOSInterface("com.huawei", "/com/huawei/switchos", QDBusConnection::sessionBus(), this))
    , m_resetSessionTimer(new QTimer(this))
    , m_limitsUpdateTimer(new QTimer(this))
    , m_kglobalaccelInter(nullptr)
    , m_kwinInter(nullptr)
{
    initConnections();
    initData();
    initConfiguration();

    m_limitsUpdateTimer->setSingleShot(true);
    m_limitsUpdateTimer->setInterval(50);

    m_resetSessionTimer->setInterval(15000);

    if (QGSettings::isSchemaInstalled("com.deepin.dde.session-shell")) {
        m_gsettings = new QGSettings("com.deepin.dde.session-shell", "/com/deepin/dde/session-shell/", this);
        if(m_gsettings->keys().contains("authResetTime")){
            int resetTime = m_gsettings->get("auth-reset-time").toInt();
            if(resetTime > 0)
               m_resetSessionTimer->setInterval(resetTime);
        }
    }

    m_resetSessionTimer->setSingleShot(true);
    connect(m_resetSessionTimer, &QTimer::timeout, this, [ = ] {
        endAuthentication(m_account, AT_All);
        destroyAuthentication(m_account);
        createAuthentication(m_account);
    });
}

/**
 * @brief 初始化信号连接
 */
void LockWorker::initConnections()
{
   // 监听用户的新增、删除、上次登出用户、用户列表变化
    connect(m_accountsInter, &AccountsInter::UserAdded, m_model, static_cast<void (SessionBaseModel::*)(const QString &)>(&SessionBaseModel::addUser));
    connect(m_accountsInter, &AccountsInter::UserDeleted, m_model, static_cast<void (SessionBaseModel::*)(const QString &)>(&SessionBaseModel::removeUser));
    // connect(m_accountsInter, &AccountsInter::UserListChanged, m_model, &SessionBaseModel::updateUserList);  // UserListChanged信号的处理， 改用UserAdded和UserDeleted信号替代
    connect(m_accountsInter, &AccountsInter::UserDeleted, this, [this](const QString &path) {
        if (path == m_model->currentUser()->path()) {
            m_model->updateCurrentUser(m_lockInter->CurrentUser());
        }
    });
    connect(m_loginedInter, &LoginedInter::LastLogoutUserChanged, m_model, static_cast<void (SessionBaseModel::*)(const uid_t)>(&SessionBaseModel::updateLastLogoutUser));
    connect(m_loginedInter, &LoginedInter::UserListChanged, m_model, &SessionBaseModel::updateLoginedUserList);

    /* org.deepin.dde.Authenticate1 */
    connect(m_authFramework, &DeepinAuthFramework::FrameworkStateChanged, this, &LockWorker::onFrameworkStateChanged);
    connect(m_authFramework, &DeepinAuthFramework::LimitsInfoChanged, this, [this](const QString &account) {
        qDebug() << "DeepinAuthFramework::LimitsInfoChanged:" << account;
        if (account == m_model->currentUser()->name()) {
            m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(account));
        }
    });
    connect(m_authFramework, &DeepinAuthFramework::SupportedEncryptsChanged, m_model, &SessionBaseModel::updateSupportedEncryptionType);
    connect(m_authFramework, &DeepinAuthFramework::SupportedMixAuthFlagsChanged, m_model, &SessionBaseModel::updateSupportedMixAuthFlags);
    connect(m_authFramework, &DeepinAuthFramework::FuzzyMFAChanged, m_model, &SessionBaseModel::updateFuzzyMFA);
    connect(m_authFramework, &DeepinAuthFramework::MFAFlagChanged, m_model, &SessionBaseModel::updateMFAFlag);
    connect(m_authFramework, &DeepinAuthFramework::PINLenChanged, m_model, &SessionBaseModel::updatePINLen);
    connect(m_authFramework, &DeepinAuthFramework::PromptChanged, m_model, &SessionBaseModel::updatePrompt);
    connect(m_authFramework, &DeepinAuthFramework::AuthStateChanged, this, &LockWorker::onAuthStateChanged);
    connect(m_authFramework, &DeepinAuthFramework::FactorsInfoChanged, m_model, &SessionBaseModel::updateFactorsInfo);

    /* org.deepin.dde.LockService1 */
    connect(m_lockInter, &DBusLockService::UserChanged, this, [ = ](const QString &json) {
        qInfo() << "DBusLockService::UserChanged:" << json;
        QTimer::singleShot(100, this, [ = ] {
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        });
        m_resetSessionTimer->stop();
    });
    connect(m_lockInter, &DBusLockService::Event, this, &LockWorker::handleServiceEvent);

    /* org.deepin.dde.SessionManager1 */
    connect(m_sessionManagerInter, &SessionManagerInter::Unlock, this, [ = ] {
        emit m_model->authFinished(true);
    });

    connect(m_sessionManagerInter, &SessionManagerInter::LockedChanged, this, [ this ](bool locked) {
        qDebug() << "SessionManagerInter::LockedChanged" << locked;
        if (locked && !m_model->visible()) {
            m_model->showLockScreen();
        }
    });

    /* org.freedesktop.login1.Session */
    connect(m_login1SessionSelf, &Login1SessionSelf::ActiveChanged, this, [ = ](bool active) {
        qInfo() << "DBusLockService::ActiveChanged:" << active;
        if (active) {
            createAuthentication(m_model->currentUser()->name());
        } else {
            endAuthentication(m_account, AT_All);
            destroyAuthentication(m_account);
        }
    });

    /* org.freedesktop.login1.Manager */
    connect(m_login1Inter, &DBusLogin1Manager::PrepareForSleep, this, [ = ](bool isSleep) {
        qInfo() << "DBusLogin1Manager::PrepareForSleep:" << isSleep;
        if (isSleep) {
            endAuthentication(m_account, AT_All);
            destroyAuthentication(m_account);
        } else {
            createAuthentication(m_model->currentUser()->name());
        }
        emit m_model->prepareForSleep(isSleep);
    });

    /* model */
    connect(m_model, &SessionBaseModel::authTypeChanged, this, [ = ](const int type) {
        qInfo() << "Auth type changed: " << type;
        if (type > 0 && m_model->getAuthProperty().MFAFlag) {
            startAuthentication(m_account, type);
        }
        // OPTMIZE: 为什么要特意用一个timer延时？
        m_limitsUpdateTimer->start();
    });
    connect(m_model, &SessionBaseModel::onPowerActionChanged, this, &LockWorker::doPowerAction);
    connect(m_model, &SessionBaseModel::visibleChanged, this, [ = ](bool visible) {
        if (visible) {
            if (m_model->currentModeState() != SessionBaseModel::ShutDownMode && m_model->currentModeState() != SessionBaseModel::UserMode) {
                createAuthentication(m_model->currentUser()->name());
            }
        } else {
            m_resetSessionTimer->stop();
            endAuthentication(m_account, AT_All);
            destroyAuthentication(m_model->currentUser()->name());
            setCurrentUser(m_model->currentUser());
            setLocked(false);
        }
    });
    connect(m_model, &SessionBaseModel::onStatusChanged, this, [ = ](SessionBaseModel::ModeStatus state) {
        if (state == SessionBaseModel::ModeStatus::PowerMode || state == SessionBaseModel::ModeStatus::ShutDownMode) {
            checkPowerInfo();
        }
    });

    /* others */
    connect(m_limitsUpdateTimer, &QTimer::timeout, this, [this] {
        if (m_authFramework->isDeepinAuthValid())
            m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(m_account));
    });

    connect(m_dbusInter, &DBusObjectInter::NameOwnerChanged, this, [ = ](const QString &name, const QString &oldOwner, const QString &newOwner) {
        Q_UNUSED(oldOwner)
        if (name == "org.deepin.dde.Authenticate1" && newOwner != "" && m_model->visible() && m_sessionManagerInter->locked()) {
            m_resetSessionTimer->stop();
            endAuthentication(m_account, AT_All);
            createAuthentication(m_model->currentUser()->name());
        }
    });

    connect(m_model, &SessionBaseModel::activeAuthChanged, this, [this] (const bool active) {
        if (!active || m_model->currentModeState() == SessionBaseModel::ModeStatus::PasswordMode)
            return;

        createAuthentication(m_model->currentUser()->name());
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
    });
}

void LockWorker::initData()
{
    /* org.deepin.dde.Accounts1 */
    m_model->updateUserList(m_accountsInter->userList());
    m_model->updateLastLogoutUser(m_loginedInter->lastLogoutUser());
    m_model->updateLoginedUserList(m_loginedInter->userList());

    /* com.deepin.udcp.iam */
    QDBusInterface ifc("com.deepin.udcp.iam", "/com/deepin/udcp/iam", "com.deepin.udcp.iam", QDBusConnection::systemBus(), this);
    const bool allowShowCustomUser = valueByQSettings<bool>("", "loginPromptInput", false) || ifc.property("Enable").toBool();
    m_model->setAllowShowCustomUser(allowShowCustomUser);

    /* init server user or custom user */
    if (DSysInfo::deepinType() == DSysInfo::DeepinServer || m_model->allowShowCustomUser()) {
        std::shared_ptr<User> user(new User());
        m_model->setIsServerModel(DSysInfo::deepinType() == DSysInfo::DeepinServer);
        m_model->addUser(user);
    }

    /* org.deepin.dde.LockService1 */
    std::shared_ptr<User> user_ptr = m_model->findUserByUid(getuid());
    if (user_ptr.get()) {
        m_model->updateCurrentUser(user_ptr);
        const QString &userJson = m_lockInter->CurrentUser();
        QJsonParseError jsonParseError;
        const QJsonDocument userDoc = QJsonDocument::fromJson(userJson.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError || userDoc.isEmpty()) {
            qWarning() << "Failed to obtain current user information from LockService!";
        } else {
            const QJsonObject userObj = userDoc.object();
            m_model->currentUser()->setLastAuthType(userObj["AuthType"].toInt());
        }
    } else {
        m_model->updateCurrentUser(m_lockInter->CurrentUser());
    }

    /* org.deepin.dde.Authenticate1 */
    m_model->updateFrameworkState(m_authFramework->GetFrameworkState());
    m_model->updateSupportedEncryptionType(m_authFramework->GetSupportedEncrypts());
    m_model->updateSupportedMixAuthFlags(m_authFramework->GetSupportedMixAuthFlags());
    m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(m_model->currentUser()->name()));
    if (m_model->isUseWayland()) {
        m_kglobalaccelInter = new QDBusInterface("org.kde.kglobalaccel","/kglobalaccel","org.kde.KGlobalAccel", QDBusConnection::sessionBus(), this);
        m_kwinInter = new QDBusInterface("org.kde.KWin","/KWin","org.kde.KWin", QDBusConnection::sessionBus(), this);
    }
}

void LockWorker::initConfiguration()
{
    m_model->setAlwaysShowUserSwitchButton(getGSettings("", "switchuser").toInt() == AuthInterface::Always);
    m_model->setAllowShowUserSwitchButton(getGSettings("", "switchuser").toInt() == AuthInterface::Ondemand);

    checkPowerInfo();
}

/**
 * @brief 处理认证状态
 *
 * @param type      认证类型
 * @param state     认证状态
 * @param message   认证消息
 */
void LockWorker::onAuthStateChanged(const int type, const int state, const QString &message)
{
    qDebug() << "LockWorker::onAuthStateChanged:" << type << state << message;

    if (m_model->getAuthProperty().MFAFlag) {
        if (type == AT_All) {
            switch (state) {
            case AS_Success:
                m_model->updateAuthState(type, state, message);
                destroyAuthentication(m_account);
                onUnlockFinished(true);
                m_resetSessionTimer->stop();
                break;
            case AS_Cancel:
                m_model->updateAuthState(type, state, message);
                destroyAuthentication(m_account);
                break;
            case AS_Recover:
                m_model->updateAuthState(type, state, message);
                createAuthentication(m_account);
                break;
            default:
                break;
            }
        } else {
            switch (state) {
            case AS_Success:
                if (m_model->currentModeState() != SessionBaseModel::ModeStatus::PasswordMode
                    && m_model->currentModeState() != SessionBaseModel::ModeStatus::ConfirmPasswordMode) {
                    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
                }
                m_resetSessionTimer->start();
                m_model->updateAuthState(type, state, message);
                break;
            case AS_Failure:
                if (m_model->currentModeState() != SessionBaseModel::ModeStatus::PasswordMode
                    && m_model->currentModeState() != SessionBaseModel::ModeStatus::ConfirmPasswordMode) {
                    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
                }
                m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(m_model->currentUser()->name()));
                endAuthentication(m_account, type);
                if (!m_model->currentUser()->limitsInfo(type).locked
                        && type != AT_Face && type != AT_Iris) {
                    QTimer::singleShot(50, this, [this, type] {
                        startAuthentication(m_account, type);
                    });
                }
                QTimer::singleShot(50, this, [this, type, state, message] {
                    m_model->updateAuthState(type, state, message);
                });
                break;
            case AS_Locked:
                if (m_model->currentModeState() != SessionBaseModel::ModeStatus::PasswordMode
                    && m_model->currentModeState() != SessionBaseModel::ModeStatus::ConfirmPasswordMode) {
                    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
                }
                endAuthentication(m_account, type);
                // TODO: 信号时序问题,考虑优化,Bug 89056
                QTimer::singleShot(50, this, [this, type, state, message] {
                    m_model->updateAuthState(type, state, message);
                });
                break;
            case AS_Timeout:
            case AS_Error:
                qWarning() << "Auth error, type: " << type << ", state: " << state << ", message: " << message;
                endAuthentication(m_account, type);
                m_model->updateAuthState(type, state, message);
                break;
            case AS_Unlocked:
                m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(m_model->currentUser()->name()));
                m_model->updateAuthState(type, state, message);
                break;
            default:
                m_model->updateAuthState(type, state, message);
                break;
            }
        }
    } else {
        if (m_model->currentModeState() != SessionBaseModel::ModeStatus::PasswordMode
            && (state == AS_Success || state == AS_Failure)
            && m_model->currentModeState() != SessionBaseModel::ModeStatus::ConfirmPasswordMode) {
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        }
        m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(m_model->currentUser()->name()));
        m_model->updateAuthState(type, state, message);
        switch (state) {
        case AS_Success:
            if ((type == AT_Face || type == AT_Iris) && m_model->visible())
                m_resetSessionTimer->start();

            break;
        case AS_Failure:
            // 单因失败会返回明确的失败类型，不关注type为-1的情况
            if (AT_All != type) {
                endAuthentication(m_account, type);
                // 人脸和虹膜需要手动重新开启验证
                if (!m_model->currentUser()->limitsInfo(type).locked && type != AT_Face && type != AT_Iris) {
                    QTimer::singleShot(50, this, [this, type] {
                        startAuthentication(m_account, type);
                    });
                }
            }
            break;
        case AS_Cancel:
            destroyAuthentication(m_account);
            break;
        case AS_Recover:
            createAuthentication(m_account);
            break;
        default:
            break;
        }
    }
}

void LockWorker::doPowerAction(const SessionBaseModel::PowerAction action)
{
    switch (action) {
    case SessionBaseModel::PowerAction::RequireSuspend:
    {
        m_model->setIsBlackMode(true);
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        int delayTime = 500;
        if(m_gsettings && m_gsettings->keys().contains("delaytime")){
            delayTime = m_gsettings->get("delaytime").toInt();
            qInfo() << "delayTime : " << delayTime;
        }
        if (delayTime < 0) {
            delayTime = 500;
        }
        QTimer::singleShot(delayTime, this, [ = ] {
            m_sessionManagerInter->RequestSuspend();
        });
    }
        break;
    case SessionBaseModel::PowerAction::RequireHibernate:
    {
        m_model->setIsBlackMode(true);
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        int delayTime = 500;
        if(m_gsettings && m_gsettings->keys().contains("delaytime")){
            delayTime = m_gsettings->get("delaytime").toInt();
            qInfo() << " delayTime : " << delayTime;
        }
        if (delayTime < 0) {
            delayTime = 500;
        }
        QTimer::singleShot(delayTime, this, [ = ] {
            m_sessionManagerInter->RequestHibernate();
        });
    }
        break;
    case SessionBaseModel::PowerAction::RequireRestart:
        if (!isLocked()) {
            m_sessionManagerInter->RequestReboot();
        } else {
            createAuthentication(m_account);
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ConfirmPasswordMode);
        }
        return;
    case SessionBaseModel::PowerAction::RequireShutdown:
        if (!isLocked()) {
            m_sessionManagerInter->RequestShutdown();
        } else {
            createAuthentication(m_account);
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ConfirmPasswordMode);
        }
        return;
    case SessionBaseModel::PowerAction::RequireLock:
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        createAuthentication(m_model->currentUser()->name());
        break;
    case SessionBaseModel::PowerAction::RequireLogout:
        m_sessionManagerInter->RequestLogout();
        return;
    case SessionBaseModel::PowerAction::RequireSwitchSystem:
        m_switchosInterface->setOsFlag(!m_switchosInterface->getOsFlag());
        QTimer::singleShot(200, this, [ = ] { m_sessionManagerInter->RequestReboot(); });
        break;
    case SessionBaseModel::PowerAction::RequireSwitchUser:
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::UserMode);
        break;
    default:
        break;
    }

    m_model->setPowerAction(SessionBaseModel::PowerAction::None);
}

/**
 * @brief 将当前用户的信息保存到 LockService 服务
 *
 * @param user
 */
void LockWorker::setCurrentUser(const std::shared_ptr<User> user)
{
    qInfo() << "Set current user: " << user->name();
    QJsonObject json;
    json["AuthType"] = user->lastAuthType();
    json["Name"] = user->name();
    json["Type"] = user->type();
    json["Uid"] = static_cast<int>(user->uid());
    m_lockInter->SwitchToUser(QString(QJsonDocument(json).toJson(QJsonDocument::Compact)));
}

void LockWorker::switchToUser(std::shared_ptr<User> user)
{
    qDebug() << "LockWorker::switchToUser:" << m_account << user->name();
    if (user->name() == m_account || *user == *m_model->currentUser()) {
        qInfo() << "switch to current user:" << user->name() << user->isLogin();
        if (!m_authFramework->authSessionExist(m_account)) {
            createAuthentication(user->name());
        } else {
            endAuthentication(m_account, AT_All);
            destroyAuthentication(m_account);
            createAuthentication(m_account);
        }
        return;
    } else {
        qInfo() << "switch user from" << m_account << "to" << user->name() << user->isLogin();
        endAuthentication(m_account, AT_All);
        setCurrentUser(user);
    }

    /**
     * 切换用户时，需要先将当前用户的界面切换为密码输入框，再切换为下一用户。
     * 当前用户界面还未更新完成，已经切换为下一用户界面了，导致切换回来时，闪现用户列表。
     * 故使用 QTimer 将切换用户的操作放在事件队列最后处理。
     */
    // TODO 在未登陆的情况下，是怎么切换到对应用户的
    QMetaObject::invokeMethod(this, [ user ] {
        QProcess::startDetached("dde-switchtogreeter", (user->isLogin() ? QStringList() << user->name() : QStringList()));
    }, Qt::QueuedConnection);
}

void LockWorker::enableZoneDetected(bool disable)
{
    m_hotZoneInter->EnableZoneDetected(disable);
}

/**
 * @brief 获取当前 Session 是否被锁定
 *
 * @return true
 * @return false
 */
bool LockWorker::isLocked() const
{
    return m_sessionManagerInter->locked();
}

/**
 * @brief 设置 Locked 的状态
 *
 * @param locked
 */
void LockWorker::setLocked(const bool locked)
{
#ifdef QT_DEBUG
    Q_UNUSED(locked)
#else
    m_sessionManagerInter->SetLocked(locked);
#endif
}

/**
 * @brief 创建认证服务
 * 有用户时，通过dbus发过来的user信息创建认证服务，类服务器模式下通过用户输入的用户创建认证服务
 * @param account
 */
void LockWorker::createAuthentication(const QString &account)
{
    qInfo() << "LockWorker::createAuthentication:" << account;
    QString userPath = m_accountsInter->FindUserByName(account);
    if (!userPath.startsWith("/")) {
        qWarning() << userPath;
        return;
    }

    // 同步密码过期的信息
    std::shared_ptr<User> user_ptr = m_model->findUserByName(account);
    if (user_ptr) {
        user_ptr->updatePasswordExpiredInfo();
        if (user_ptr->isNoPasswordLogin()) {
            qInfo() << "User is no password login";
            return;
        }
    }

    m_account = account;
    switch (m_model->getAuthProperty().FrameworkState) {
    case Available:
        m_authFramework->CreateAuthController(account, m_authFramework->GetSupportedMixAuthFlags(), Lock);
        break;
    default:
        m_authFramework->CreateAuthenticate(account);
        m_model->setAuthType(AT_PAM);
        break;
    }

    setLocked(true);
}

/**
 * @brief 退出认证服务
 *
 * @param account
 */
void LockWorker::destroyAuthentication(const QString &account)
{
    qInfo() << "LockWorker::destroyAuthentication:" << account;
    switch (m_model->getAuthProperty().FrameworkState) {
    case Available:
        m_authFramework->DestroyAuthController(account);
        break;
    default:
        m_authFramework->DestroyAuthenticate();
        break;
    }
}

/**
 * @brief 开启认证服务    -- 作为接口提供给上层，隐藏底层细节
 *
 * @param account   账户
 * @param authType  认证类型（可传入一种或多种）
 * @param timeout   设定超时时间（默认 -1）
 */
void LockWorker::startAuthentication(const QString &account, const int authType)
{
    qInfo() << "LockWorker::startAuthentication:" << account << authType;
    switch (m_model->getAuthProperty().FrameworkState) {
    case Available:
        m_authFramework->StartAuthentication(account, authType, -1);
        break;
    default:
        m_authFramework->CreateAuthenticate(account);
        break;
    }
}

/**
 * @brief 将密文发送给认证服务
 *
 * @param account   账户
 * @param authType  认证类型
 * @param token     密文
 */
void LockWorker::sendTokenToAuth(const QString &account, const int authType, const QString &token)
{
    qInfo() << "LockWorker::sendTokenToAuth:" << account << authType;
    switch (m_model->getAuthProperty().FrameworkState) {
    case Available:
        m_authFramework->SendTokenToAuth(account, authType, token);
        break;
    default:
        m_authFramework->SendToken(token);
        break;
    }
}

/**
 * @brief 结束本次认证，下次认证前需要先开启认证服务
 *
 * @param account   账户
 * @param authType  认证类型
 */
void LockWorker::endAuthentication(const QString &account, const int authType)
{
    qDebug() << "LockWorker::endAuthentication:" << account << authType;
    switch (m_model->getAuthProperty().FrameworkState) {
    case Available:
        m_authFramework->EndAuthentication(account, authType);
        break;
    default:
        break;
    }
}

void LockWorker::endAccountAuthentication(const QString &account, const int authType)
{
    endAuthentication(account, authType);
    if (authType == AT_All) {
        destroyAuthentication(account);
    }
}

void LockWorker::onFrameworkStateChanged(int state)
{
    onAuthStateChanged(AT_All, AS_Cancel, "Framework state changes.");
    m_model->updateFrameworkState(state);
    onAuthStateChanged(AT_All, AS_Recover, "Framework state changes.");
}

void LockWorker::handleServiceEvent(quint32 eventType, quint32 pid, const QString &username, const QString &message)
{
    Q_UNUSED(pid)
    if (!m_model->currentUser())
        return;

    if (username != m_model->currentUser()->name())
        return;

    // Don't show password prompt from standard pam modules since
    // we'll provide our own prompt or just not.
    const QString msg = message.simplified() == "Password:" ? "" : message;

    if (msg == "Verification timed out") {
        m_isThumbAuth = true;
        emit m_model->authFailedMessage(tr("Fingerprint verification timed out, please enter your password manually"));
        return;
    }

    switch (eventType) {
    case DBusLockService::PromptQuestion:
        qWarning() << "prompt question from pam: " << message;
        emit m_model->authFailedMessage(message);
        break;
    case DBusLockService::PromptSecret:
        qWarning() << "prompt secret from pam: " << message;
        if (m_isThumbAuth && !msg.isEmpty()) {
            emit m_model->authFailedMessage(msg);
        }
        break;
    case DBusLockService::ErrorMsg:
        qWarning() << "error message from pam: " << message;
        if (msg == "Failed to match fingerprint") {
            emit m_model->authFailedTipsMessage(tr("Failed to match fingerprint"));
            emit m_model->authFailedMessage("");
        }
        break;
    case DBusLockService::TextInfo:
        emit m_model->authFailedMessage(QString(dgettext("fprintd", message.toLatin1())));
        break;
    case DBusLockService::Failure:
        onUnlockFinished(false);
        break;
    case DBusLockService::Success:
        onUnlockFinished(true);
        break;
    default:
        break;
    }
}

void LockWorker::onAuthFinished()
{
    m_model->setVisible(false);
    onUnlockFinished(true);
    setCurrentUser(m_model->currentUser());
}

void LockWorker::onUnlockFinished(bool unlocked)
{
    qInfo() << "LockWorker::onUnlockFinished -- unlocked state : " << unlocked;

    //To Do: 最好的方案是修改同步后端认证信息的代码设计
    if (m_model->currentModeState() == SessionBaseModel::ModeStatus::UserMode)
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);

    //    if (!unlocked && m_authFramework->GetAuthType() == AuthFlag::Password) {
    //        qWarning() << "Authorization password failed!";
    //        emit m_model->authFailedTipsMessage(tr("Wrong Password"));
    //        return;
    //    }

    switch (m_model->powerAction()) {
    case SessionBaseModel::PowerAction::RequireRestart:
        if (unlocked) {
            m_sessionManagerInter->RequestReboot();
        }
        break;
    case SessionBaseModel::PowerAction::RequireShutdown:
        if (unlocked) {
            m_sessionManagerInter->RequestShutdown();
        }
        break;
    default:
        break;
    }

    setLocked(!unlocked);
    emit m_model->authFinished(unlocked);
}

void LockWorker::restartResetSessionTimer()
{
    if (m_model->visible() && m_resetSessionTimer->isActive()) {
        m_resetSessionTimer->start();
    }
}

void LockWorker::disableGlobalShortcutsForWayland(const bool enable)
{
    if (m_kwinInter == nullptr || m_kglobalaccelInter == nullptr) {
        return;
    }
    if (!m_kwinInter->isValid()) {
        qWarning() << "kwinInter is not valid";
        return;
    }
    QDBusReply<void> reply = m_kwinInter->call("disableGlobalShortcutsForClient", enable);
    if (!reply.isValid()) {
        qWarning() << "call disableGlobalShortcutsForClient failed" << reply.error();
    }
    if (!m_kglobalaccelInter->isValid()) {
        qWarning() << "kglobalaccelInter is not valid";
        return;
    }
    reply = m_kglobalaccelInter->call("setActiveByUniqueName", "Screenshot", true);
    if (!reply.isValid()) {
        qWarning() << "call setActiveByUniqueName failed" << reply.error();
    }
}

void LockWorker::checkAccount(const QString &account)
{
    Q_UNUSED(account)
    if (m_model->currentUser() && m_model->currentUser()->isNoPasswordLogin()) {
        qInfo() << "Current user has set 'no password login' : " << account;
        onAuthFinished();
    }
}
