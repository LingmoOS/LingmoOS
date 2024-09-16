// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "greeterworker.h"

#include "authcommon.h"
#include "keyboardmonitor.h"
#include "userinfo.h"

#include "systempower_interface.h"

#include <DSysInfo>

#include <QGSettings>

#include <pwd.h>

#define LOCKSERVICE_PATH "/org/deepin/dde/LockService1"
#define LOCKSERVICE_NAME "org.deepin.dde.LockService1"
#define SECURITYENHANCE_PATH "/com/deepin/daemon/SecurityEnhance"
#define SECURITYENHANCE_NAME "com.deepin.daemon.SecurityEnhance"

using PowerInter = org::deepin::dde::Power1;
using namespace Auth;
using namespace AuthCommon;
DCORE_USE_NAMESPACE

const int NUM_LOCKED = 0;   // 禁用小键盘
const int NUM_UNLOCKED = 1; // 启用小键盘
const int NUM_LOCK_UNKNOWN = 2; // 未知状态

GreeterWorker::GreeterWorker(SessionBaseModel *const model, QObject *parent)
    : AuthInterface(model, parent)
    , m_greeter(new QLightDM::Greeter(this))
    , m_authFramework(new DeepinAuthFramework(this))
    , m_lockInter(new DBusLockService(LOCKSERVICE_NAME, LOCKSERVICE_PATH, QDBusConnection::systemBus(), this))
    , m_soundPlayerInter(new SoundThemePlayerInter("org.deepin.dde.SoundThemePlayer1", "/org/deepin/dde/SoundThemePlayer1", QDBusConnection::systemBus(), this))
    , m_resetSessionTimer(new QTimer(this))
    , m_limitsUpdateTimer(new QTimer(this))
    , m_retryAuth(false)
{
#ifndef QT_DEBUG
    if (!m_greeter->connectSync()) {
        qCritical() << "greeter connect fail !!!";
        exit(1);
    }
#endif

    checkDBusServer(m_accountsInter->isValid());

    initConnections();
    initData();
    initConfiguration();

    m_limitsUpdateTimer->setSingleShot(true);
    m_limitsUpdateTimer->setInterval(50);

    //认证超时重启
    m_resetSessionTimer->setInterval(15000);
    if (QGSettings::isSchemaInstalled("com.deepin.dde.session-shell")) {
        QGSettings gsetting("com.deepin.dde.session-shell", "/com/deepin/dde/session-shell/", this);
        if (gsetting.keys().contains("authResetTime")) {
            int resetTime = gsetting.get("auth-reset-time").toInt();
            if (resetTime > 0)
                m_resetSessionTimer->setInterval(resetTime);
        }
    }

    m_resetSessionTimer->setSingleShot(true);
    connect(m_resetSessionTimer, &QTimer::timeout, this, [ = ] {
        endAuthentication(m_account, AT_All);
        m_model->updateAuthState(AT_All, AS_Cancel, "Cancel");
        destroyAuthentication(m_account);
        createAuthentication(m_account);
    });
}

GreeterWorker::~GreeterWorker()
{
}

void GreeterWorker::initConnections()
{
    /* greeter */
    connect(m_greeter, &QLightDM::Greeter::showPrompt, this, &GreeterWorker::showPrompt);
    connect(m_greeter, &QLightDM::Greeter::showMessage, this, &GreeterWorker::showMessage);
    connect(m_greeter, &QLightDM::Greeter::authenticationComplete, this, &GreeterWorker::authenticationComplete);
    /* org.deepin.dde.Accounts1 */
    connect(m_accountsInter, &AccountsInter::UserAdded, m_model, static_cast<void (SessionBaseModel::*)(const QString &)>(&SessionBaseModel::addUser));
    connect(m_accountsInter, &AccountsInter::UserDeleted, m_model, static_cast<void (SessionBaseModel::*)(const QString &)>(&SessionBaseModel::removeUser));
    // connect(m_accountsInter, &AccountsInter::UserListChanged, m_model, &SessionBaseModel::updateUserList); // UserListChanged信号的处理， 改用UserAdded和UserDeleted信号替代
    connect(m_accountsInter, &AccountsInter::UserDeleted, this, [this](const QString &path) {
        if (path == m_model->currentUser()->path()) {
            m_model->updateCurrentUser(m_lockInter->CurrentUser());
            m_model->updateAuthState(AT_All, AS_Cancel, "Cancel");
            destroyAuthentication(m_account);
            if (!m_model->currentUser()->isNoPasswordLogin()) {
                createAuthentication(m_model->currentUser()->name());
            } else {
                m_model->setAuthType(AT_None);
            }
            m_soundPlayerInter->PrepareShutdownSound(static_cast<int>(m_model->currentUser()->uid()));
        }
    });
    connect(m_loginedInter, &LoginedInter::LastLogoutUserChanged, m_model, static_cast<void (SessionBaseModel::*)(const uid_t)>(&SessionBaseModel::updateLastLogoutUser));
    connect(m_loginedInter, &LoginedInter::UserListChanged, m_model, &SessionBaseModel::updateLoginedUserList);
    /* org.deepin.dde.Authenticate1 */
    connect(m_authFramework, &DeepinAuthFramework::FrameworkStateChanged, m_model, &SessionBaseModel::updateFrameworkState);
    connect(m_authFramework, &DeepinAuthFramework::LimitsInfoChanged, this, [this](const QString &account) {
        qDebug() << "GreeterWorker::initConnections LimitsInfoChanged:" << account;
        if (account == m_model->currentUser()->name()) {
            m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(account));
        }
    });
    connect(m_authFramework, &DeepinAuthFramework::SupportedEncryptsChanged, m_model, &SessionBaseModel::updateSupportedEncryptionType);
    connect(m_authFramework, &DeepinAuthFramework::SupportedMixAuthFlagsChanged, m_model, &SessionBaseModel::updateSupportedMixAuthFlags);
    /* org.deepin.dde.Authenticate1.Session */
    connect(m_authFramework, &DeepinAuthFramework::AuthStateChanged, this, &GreeterWorker::onAuthStateChanged);
    connect(m_authFramework, &DeepinAuthFramework::FactorsInfoChanged, m_model, &SessionBaseModel::updateFactorsInfo);
    connect(m_authFramework, &DeepinAuthFramework::FuzzyMFAChanged, m_model, &SessionBaseModel::updateFuzzyMFA);
    connect(m_authFramework, &DeepinAuthFramework::MFAFlagChanged, m_model, &SessionBaseModel::updateMFAFlag);
    connect(m_authFramework, &DeepinAuthFramework::PINLenChanged, m_model, &SessionBaseModel::updatePINLen);
    connect(m_authFramework, &DeepinAuthFramework::PromptChanged, m_model, &SessionBaseModel::updatePrompt);
    /* org.freedesktop.login1.Session */
    connect(m_login1SessionSelf, &Login1SessionSelf::ActiveChanged, this, [ = ](bool active) {
        qInfo() << "Login1SessionSelf::ActiveChanged:" << active;
        if (m_model->currentUser() == nullptr || m_model->currentUser()->name().isEmpty()) {
            return;
        }
        if (active) {
            if (!m_model->isServerModel() && !m_model->currentUser()->isNoPasswordLogin()) {
                createAuthentication(m_model->currentUser()->name());
            }
        } else {
            endAuthentication(m_account, AT_All);
            destroyAuthentication(m_account);
        }
    });
    /* org.freedesktop.login1.Manager */
    connect(m_login1Inter, &DBusLogin1Manager::PrepareForSleep, this, [ = ](bool isSleep) {
        qInfo() << "DBusLogin1Manager::PrepareForSleep:" << isSleep;
        // 登录界面待机或休眠时提供显示假黑屏，唤醒时显示正常界面
        m_model->setIsBlackMode(isSleep);

        if (isSleep) {
            endAuthentication(m_account, AT_All);
            destroyAuthentication(m_account);
        } else {
            createAuthentication(m_model->currentUser()->name());
        }
    });
    connect(m_login1Inter, &DBusLogin1Manager::SessionRemoved, this, [this] {
        qInfo() << "DBusLogin1Manager::SessionRemoved";
        if (m_model->updateCurrentUser(m_lockInter->CurrentUser())) {
            m_model->updateAuthState(AT_All, AS_Cancel, "Cancel");
            destroyAuthentication(m_account);
            if (!m_model->currentUser()->isNoPasswordLogin()) {
                createAuthentication(m_model->currentUser()->name());
            } else {
                m_model->setAuthType(AT_None);
            }
        }
        m_soundPlayerInter->PrepareShutdownSound(static_cast<int>(m_model->currentUser()->uid()));
    });
    /* org.deepin.dde.LockService1 */
    connect(m_lockInter, &DBusLockService::UserChanged, this, [ = ](const QString &json) {
        qInfo() << "DBusLockService::UserChanged:" << json;
        // 如果是已登录用户则返回，否则已登录用户和未登录用户来回切换时会造成用户信息错误
        std::shared_ptr<User> user_ptr = m_model->json2User(json);
        if (!user_ptr || user_ptr->isLogin())
            return;

        m_resetSessionTimer->stop();
        m_model->updateCurrentUser(user_ptr);
        const QString &account = user_ptr->name();
        if (user_ptr.get()->isNoPasswordLogin()) {
            emit m_model->authTypeChanged(AT_None);
            m_account = account;
        }
        QTimer::singleShot(100, this, [ = ] {
            if (m_model->appType() == AppType::Login) {
                // 管理员账户且密码过期的情况下，设置当前状态为ResetPasswdMode，从而方便直接跳转到重置密码界面
                bool showReset = m_model->currentUser()->expiredState() == User::ExpiredState::ExpiredAlready
                        && m_model->currentUser()->accountType() == User::AccountType::Admin;
                m_model->setCurrentModeState(showReset ? SessionBaseModel::ModeStatus::ResetPasswdMode : SessionBaseModel::ModeStatus::PasswordMode);
            } else {
                m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
            }
        });
    });
    /* model */
    connect(m_model, &SessionBaseModel::authTypeChanged, this, [ = ](const int type) {
        qInfo() << "Auth type changed: " << type;
        if (type > 0 && m_model->getAuthProperty().MFAFlag) {
            startAuthentication(m_account, m_model->getAuthProperty().AuthType);
        }
        m_limitsUpdateTimer->start();
    });
    connect(m_model, &SessionBaseModel::onPowerActionChanged, this, &GreeterWorker::doPowerAction);
    connect(m_model, &SessionBaseModel::currentUserChanged, this, &GreeterWorker::onCurrentUserChanged);
    connect(m_model, &SessionBaseModel::visibleChanged, this, [ = ](bool visible) {
        if (visible) {
            if (!m_model->isServerModel()
                && (!m_model->currentUser()->isNoPasswordLogin() || m_model->currentUser()->expiredState() == User::ExpiredAlready)) {
                createAuthentication(m_model->currentUser()->name());
            }
        } else {
            m_resetSessionTimer->stop();
        }
    });
    /* others */
    connect(KeyboardMonitor::instance(), &KeyboardMonitor::numlockStatusChanged, this, [ = ](bool on) {
        saveNumlockState(m_model->currentUser(), on);
    });
    connect(m_limitsUpdateTimer, &QTimer::timeout, this, [this] {
        if (m_authFramework->isDeepinAuthValid())
            m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(m_account));
    });

    // 等保服务开启/关闭时更新
    QDBusConnection::systemBus().connect(SECURITYENHANCE_NAME, SECURITYENHANCE_PATH, SECURITYENHANCE_NAME,
                                         "Receipt", this, SLOT(onReceiptChanged(bool)));
}

void GreeterWorker::initData()
{
    if (isSecurityEnhanceOpen())
        m_model->setSEType(true);

    /* org.deepin.dde.Accounts */
    m_model->updateUserList(m_accountsInter->userList());
    m_model->updateLastLogoutUser(m_loginedInter->lastLogoutUser());
    m_model->updateLoginedUserList(m_loginedInter->userList());

    /* com.deepin.udcp.iam */
    QDBusInterface ifc("com.deepin.udcp.iam", "/com/deepin/udcp/iam", "com.deepin.udcp.iam", QDBusConnection::systemBus(), this);
    const bool allowShowCustomUser = valueByQSettings<bool>("", "loginPromptInput", false) || ifc.property("Enable").toBool();
    m_model->setAllowShowCustomUser(allowShowCustomUser);

    /* init current user */
    if (DSysInfo::deepinType() == DSysInfo::DeepinServer || m_model->allowShowCustomUser()) {
        std::shared_ptr<User> user(new User());
        m_model->setIsServerModel(DSysInfo::deepinType() == DSysInfo::DeepinServer);
        m_model->addUser(user);
        if (DSysInfo::deepinType() == DSysInfo::DeepinServer) {
            m_model->updateCurrentUser(user);
        } else {
            /* org.deepin.dde.LockService1 */
            m_model->updateCurrentUser(m_lockInter->CurrentUser());
        }
    } else {
        /* org.deepin.dde.LockService1 */
        m_model->updateCurrentUser(m_lockInter->CurrentUser());
    }
    m_soundPlayerInter->PrepareShutdownSound(static_cast<int>(m_model->currentUser()->uid()));

    /* org.deepin.dde.Authenticate1 */
    if (m_authFramework->isDeepinAuthValid()) {
        m_model->updateFrameworkState(m_authFramework->GetFrameworkState());
        m_model->updateSupportedEncryptionType(m_authFramework->GetSupportedEncrypts());
        m_model->updateSupportedMixAuthFlags(m_authFramework->GetSupportedMixAuthFlags());
        m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(m_model->currentUser()->name()));
    }
}

void GreeterWorker::initConfiguration()
{
    m_model->setAlwaysShowUserSwitchButton(getGSettings("", "switchuser").toInt() == AuthInterface::Always);
    m_model->setAllowShowUserSwitchButton(getGSettings("", "switchuser").toInt() == AuthInterface::Ondemand);

    checkPowerInfo();

    if (QFile::exists("/etc/deepin/no_suspend")) {
        m_model->setCanSleep(false);
    }

    // 当这个配置不存在是，如果是不是笔记本就打开小键盘，否则就关闭小键盘 0关闭键盘 1打开键盘 2默认值（用来判断是不是有这个key）
    if (m_model->currentUser() != nullptr && getNumLockState(m_model->currentUser()->name()) == NUM_LOCK_UNKNOWN) {
        PowerInter powerInter("org.deepin.dde.Power1", "/org/deepin/dde/Power1", QDBusConnection::systemBus(), this);
        if (powerInter.hasBattery()) {
            saveNumlockState(m_model->currentUser(), false);
        } else {
            saveNumlockState(m_model->currentUser(), true);
        }
    }
    recoveryUserKBState(m_model->currentUser());
}

void GreeterWorker::doPowerAction(const SessionBaseModel::PowerAction action)
{
    switch (action) {
    case SessionBaseModel::PowerAction::RequireShutdown:
        m_login1Inter->PowerOff(true);
        break;
    case SessionBaseModel::PowerAction::RequireRestart:
        m_login1Inter->Reboot(true);
        break;
    // 在登录界面请求待机或者休眠时，通过显示假黑屏挡住输入密码界面，防止其闪现
    case SessionBaseModel::PowerAction::RequireSuspend:
        m_model->setIsBlackMode(true);
        if (m_model->currentModeState() != SessionBaseModel::ModeStatus::PasswordMode)
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        m_login1Inter->Suspend(true);
        break;
    case SessionBaseModel::PowerAction::RequireHibernate:
        m_model->setIsBlackMode(true);
        if (m_model->currentModeState() != SessionBaseModel::ModeStatus::PasswordMode)
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        m_login1Inter->Hibernate(true);
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
void GreeterWorker::setCurrentUser(const std::shared_ptr<User> user)
{
    QJsonObject json;
    json["AuthType"] = user->lastAuthType();
    json["Name"] = user->name();
    json["Type"] = user->type();
    json["Uid"] = static_cast<int>(user->uid());
    m_lockInter->SwitchToUser(QString(QJsonDocument(json).toJson(QJsonDocument::Compact))).waitForFinished();
}

void GreeterWorker::switchToUser(std::shared_ptr<User> user)
{
    if (*user == *m_model->currentUser()) {
        if (!m_model->currentUser()->isNoPasswordLogin()) {
            createAuthentication(user->name());
        }
        return;
    }
    qInfo() << "switch user from" << m_account << " to " << user->name() << user->uid() << user->isLogin();
    endAuthentication(m_account, AT_All);

    if (user->uid() == INT_MAX) {
        m_model->setAuthType(AT_None);
    }

    setCurrentUser(user);
    m_model->updateAuthState(AT_All, AS_Cancel, "Cancel");
    destroyAuthentication(m_account);
    m_model->updateCurrentUser(user);
    if (!user->isNoPasswordLogin()) {
        createAuthentication(user->name());
    } else {
        m_model->setAuthType(AT_None);
    }
    m_soundPlayerInter->PrepareShutdownSound(static_cast<int>(m_model->currentUser()->uid()));
}

bool GreeterWorker::isSecurityEnhanceOpen()
{
    QDBusInterface securityEnhanceInterface(SECURITYENHANCE_NAME,
                               SECURITYENHANCE_PATH,
                               SECURITYENHANCE_NAME,
                               QDBusConnection::systemBus());
    QDBusReply<QString> reply = securityEnhanceInterface.call("Status");
    if (!reply.isValid()) {
       qWarning() << "get security enhance status error: " << reply.error();
       return false;
    }
    return reply.value() == "open" || reply.value() == "opening";
}

/**
 * @brief 创建认证服务
 * 有用户时，通过dbus发过来的user信息创建认证服务，类服务器模式下通过用户输入的用户创建认证服务
 * @param account
 */
void GreeterWorker::createAuthentication(const QString &account)
{
    qInfo() << "GreeterWorker::createAuthentication:" << account;
    m_account = account;
    if (account.isEmpty() || account == "...") {
        m_model->setAuthType(AT_None);
        return;
    }

    m_retryAuth = false;
    std::shared_ptr<User> user_ptr = m_model->findUserByName(account);
    if (user_ptr) {
        user_ptr->updatePasswordExpiredInfo();
    }
    switch (m_model->getAuthProperty().FrameworkState) {
    case Available:
        m_authFramework->CreateAuthController(account, m_authFramework->GetSupportedMixAuthFlags(), Login);
        m_authFramework->SetAuthQuitFlag(account, DeepinAuthFramework::ManualQuit);
        if (!m_authFramework->SetPrivilegesEnable(account, QString("/usr/sbin/lightdm"))) {
            qWarning() << "Failed to set privileges!";
        }
        startGreeterAuth(account);
        break;
    default:
        startGreeterAuth(account);
        m_model->setAuthType(AT_PAM);
        break;
    }
}

/**
 * @brief 退出认证服务
 *
 * @param account
 */
void GreeterWorker::destroyAuthentication(const QString &account)
{
    qDebug() << "GreeterWorker::destroyAuthentication:" << account;
    switch (m_model->getAuthProperty().FrameworkState) {
    case Available:
        m_authFramework->DestroyAuthController(account);
        break;
    default:
        break;
    }
}

/**
 * @brief 开启认证服务    -- 作为接口提供给上层，屏蔽底层细节
 *
 * @param account   账户
 * @param authType  认证类型（可传入一种或多种）
 * @param timeout   设定超时时间（默认 -1）
 */
void GreeterWorker::startAuthentication(const QString &account, const int authType)
{
    qDebug() << "GreeterWorker::startAuthentication:" << account << authType;
    switch (m_model->getAuthProperty().FrameworkState) {
    case Available:
        m_authFramework->StartAuthentication(account, authType, -1);
        // 如果密码被锁定了，lightdm会停止验证，在开启验证的时候需要判断一下lightdm是否在验证中。
        if (!m_greeter->inAuthentication())
            startGreeterAuth(account);

        break;
    default:
        startGreeterAuth(account);
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
void GreeterWorker::sendTokenToAuth(const QString &account, const int authType, const QString &token)
{
    qDebug() << "GreeterWorker::sendTokenToAuth:" << account << authType;
    switch (m_model->getAuthProperty().FrameworkState) {
    case Available:
        if (AT_PAM == authType) {
            m_greeter->respond(token);
        } else {
            m_authFramework->SendTokenToAuth(account, authType, token);
        }
        if (authType == AT_Password) {
            m_password = token; // 用于解锁密钥环
        }
        break;
    default:
        m_greeter->respond(token);
        break;
    }
}

/**
 * @brief 结束本次认证，下次认证前需要先开启认证服务
 *
 * @param account   账户
 * @param authType  认证类型
 */
void GreeterWorker::endAuthentication(const QString &account, const int authType)
{
    qDebug() << "GreeterWorker::endAuthentication:" << account << authType;
    switch (m_model->getAuthProperty().FrameworkState) {
    case Available:
        if (authType == AT_All)
            m_authFramework->SetPrivilegesDisable(account);

        m_authFramework->EndAuthentication(account, authType);
        break;
    default:
        break;
    }
}

/**
 * @brief 检查用户输入的账户是否合理
 *
 * @param account
 */
void GreeterWorker::checkAccount(const QString &account)
{
    qDebug() << "GreeterWorker::checkAccount:" << account;
    if (m_greeter->authenticationUser() == account) {
        return;
    }

    std::shared_ptr<User> user_ptr = m_model->findUserByName(account);
    // 当用户登录成功后，判断用户输入帐户有效性逻辑改为后端去做处理
    const QString userPath = m_accountsInter->FindUserByName(account);
    if (userPath.startsWith("/")) {
        user_ptr = std::make_shared<NativeUser>(userPath);

        // 对于没有设置密码的账户,直接认定为错误账户
        if (!user_ptr->isPasswordValid()) {
            qWarning() << userPath;
            emit m_model->authFailedTipsMessage(tr("Wrong account"));
            m_model->setAuthType(AT_None);
            return;
        }
    } else if (!user_ptr) {
        // 判断账户第一次登录时的有效性
        std::string str = account.toStdString();
        passwd *pw = getpwnam(str.c_str());
        if (pw) {
            QString userName = pw->pw_name;
            QString userFullName = userName.leftRef(userName.indexOf(QString("@"))).toString();
            user_ptr = std::make_shared<ADDomainUser>(INT_MAX - 1);

            dynamic_cast<ADDomainUser *>(user_ptr.get())->setName(userName);
            dynamic_cast<ADDomainUser *>(user_ptr.get())->setFullName(userFullName);
        } else {
            qWarning() << userPath;
            emit m_model->authFailedTipsMessage(tr("Wrong account"));
            m_model->setAuthType(AT_None);
            return;
        }
    }

    m_model->updateCurrentUser(user_ptr);
    if (user_ptr->isNoPasswordLogin()) {
        if (user_ptr->expiredState() == User::ExpiredAlready) {
            changePasswd();
        }
        startGreeterAuth(user_ptr->name());
    } else {
        m_resetSessionTimer->stop();
        endAuthentication(m_account, AT_All);
        m_model->updateAuthState(AT_All, AS_Cancel, "Cancel");
        destroyAuthentication(m_account);
        createAuthentication(user_ptr->name());
    }
}

void GreeterWorker::checkDBusServer(bool isValid)
{
    if (isValid) {
        m_accountsInter->userList();
    } else {
        // FIXME: 我不希望这样做，但是QThread::msleep会导致无限递归
        QTimer::singleShot(300, this, [ = ] {
            qWarning() << "org.deepin.dde.Accounts1 is not start, rechecking!";
            checkDBusServer(m_accountsInter->isValid());
        });
    }
}

/**
 * @brief 显示提示信息
 *
 * @param text
 * @param type
 */
void GreeterWorker::showPrompt(const QString &text, const QLightDM::Greeter::PromptType type)
{
    qInfo() << "Greeter prompt:" << text << "type:" << type;
    switch (type) {
    case QLightDM::Greeter::PromptTypeSecret:
        m_retryAuth = true;
        m_model->updateAuthState(AT_PAM, AS_Prompt, text);
        break;
    case QLightDM::Greeter::PromptTypeQuestion:
        m_model->updateAuthState(AT_PAM, AS_Prompt, text);
        break;
    }
}

/**
 * @brief 显示认证成功/失败的信息
 *
 * @param text
 * @param type
 */
void GreeterWorker::showMessage(const QString &text, const QLightDM::Greeter::MessageType type)
{
    qInfo() << "Greeter message:" << text << "type:" << type;
    switch (type) {
    case QLightDM::Greeter::MessageTypeInfo:
        m_model->updateAuthState(AT_PAM, AS_Prompt, text);
        break;
    case QLightDM::Greeter::MessageTypeError:
        m_retryAuth = false;
        m_model->updateAuthState(AT_PAM, AS_Failure, text);
        break;
    }
}

/**
 * @brief 认证完成
 */
void GreeterWorker::authenticationComplete()
{
    const bool result = m_greeter->isAuthenticated();
    qInfo() << "Authentication result:" << result << "retryAuth" << m_retryAuth;

    if (!result) {
        if (m_retryAuth && !m_model->getAuthProperty().MFAFlag) {
            showMessage(tr("Wrong Password"), QLightDM::Greeter::MessageTypeError);
        }
        if (!m_model->currentUser()->limitsInfo(AT_Password).locked) {
            m_greeter->authenticate(m_model->currentUser()->name());
        }
        return;
    }

    emit m_model->authFinished(true);

    m_password.clear();

    switch (m_model->powerAction()) {
    case SessionBaseModel::PowerAction::RequireRestart:
        m_login1Inter->Reboot(true);
        return;
    case SessionBaseModel::PowerAction::RequireShutdown:
        m_login1Inter->PowerOff(true);
        return;
    default:
        break;
    }

    qInfo() << "start session = " << m_model->sessionKey();

    auto startSessionSync = [ = ]() {
        setCurrentUser(m_model->currentUser());
        m_greeter->startSessionSync(m_model->sessionKey());
    };

    emit requestUpdateBackground(m_model->currentUser()->greeterBackground());
    startSessionSync();
    endAuthentication(m_account, AT_All);
    destroyAuthentication(m_account);
}

void GreeterWorker::onAuthFinished()
{
    qInfo() << "GreeterWorker::onAuthFinished";
    if (m_greeter->inAuthentication()) {
        m_greeter->respond(m_authFramework->AuthSessionPath(m_account) + QString(";") + m_password);
        if (m_model->currentUser()->expiredState() == User::ExpiredAlready) {
            changePasswd();
        }
    } else {
        qWarning() << "The lightdm is not in authentication!";
    }
}

void GreeterWorker::onAuthStateChanged(const int type, const int state, const QString &message)
{
    qDebug() << "GreeterWorker::onAuthStateChanged:" << type << state << message;
    if (m_model->getAuthProperty().MFAFlag) {
        if (type == AT_All) {
            switch (state) {
            case AS_Success:
                m_model->updateAuthState(type, state, message);
                m_resetSessionTimer->stop();
                if (m_greeter->inAuthentication()) {
                    m_greeter->respond(m_authFramework->AuthSessionPath(m_account) + QString(";") + m_password);
                    if (m_model->currentUser()->expiredState() == User::ExpiredAlready) {
                        changePasswd();
                    }
                } else {
                    qWarning() << "The lightdm is not in authentication!";
                }
                break;
            case AS_Cancel:
                m_model->updateAuthState(type, state, message);
                destroyAuthentication(m_account);
                break;
            default:
                break;
            }
        } else {
            switch (state) {
            case AS_Success:
                if (m_model->currentModeState() != SessionBaseModel::ResetPasswdMode)
                    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
                m_resetSessionTimer->start();
                m_model->updateAuthState(type, state, message);
                break;
            case AS_Failure:
                if (m_model->currentModeState() != SessionBaseModel::ResetPasswdMode) {
                    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
                }
                m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(m_model->currentUser()->name()));
                endAuthentication(m_account, type);
                // 人脸和虹膜需要手动重启验证
                if (!m_model->currentUser()->limitsInfo(type).locked && type != AT_Face && type != AT_Iris) {
                    QTimer::singleShot(50, this, [this, type] {
                        startAuthentication(m_account, type);
                    });
                }
                QTimer::singleShot(50, this, [ = ] {
                    m_model->updateAuthState(type, state, message);
                });
                break;
            case AS_Locked:
                if (m_model->currentModeState() != SessionBaseModel::ResetPasswdMode)
                    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
                endAuthentication(m_account, type);
                // TODO: 信号时序问题,考虑优化,Bug 89056
                QTimer::singleShot(50, this, [ = ] {
                    m_model->updateAuthState(type, state, message);
                });
                break;
            case AS_Timeout:
            case AS_Error:
                m_model->updateAuthState(type, state, message);
                endAuthentication(m_account, type);
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
            && m_model->currentModeState() != SessionBaseModel::ResetPasswdMode)
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);

        m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(m_model->currentUser()->name()));
        m_model->updateAuthState(type, state, message);
        switch (state) {
        case AS_Success:
            if (type == AT_Face || type == AT_Iris)
                m_resetSessionTimer->start();
            break;
        case AS_Failure:
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
        default:
            break;
        }
    }
}

void GreeterWorker::onReceiptChanged(bool state)
{
    Q_UNUSED(state)

    m_model->setSEType(isSecurityEnhanceOpen());
}

void GreeterWorker::onCurrentUserChanged(const std::shared_ptr<User> &user)
{
    recoveryUserKBState(user);
    loadTranslation(user->locale());
}

void GreeterWorker::saveNumlockState(std::shared_ptr<User> user, bool on)
{
    QStringList list = getDConfigValue(getDefaultConfigFileName(), "numLockState", QStringList()).toStringList();
    const QString &userName = user->name();

    // 移除当前用户的记录
    for (const QString &numLockState : list) {
        QStringList tmpList = numLockState.split(":");
        if (tmpList.size() == 2 && tmpList.at(0) == userName) {
            list.removeAll(numLockState);
            break;
        }
    }

    // 插入当前用户的记录
    list.append(user->name() + ":" + (on ? "True" : "False"));
    setDConfigValue(getDefaultConfigFileName(), "numLockState", list);
}

int GreeterWorker::getNumLockState(const QString &userName)
{
    QStringList list = getDConfigValue(getDefaultConfigFileName(), "numLockState", QStringList()).toStringList();

    for (const QString &numLockState : list) {
        QStringList tmpList = numLockState.split(":");
        if (tmpList.size() == 2 && tmpList.at(0) == userName) {
            return "True" == tmpList.at(1) ? NUM_UNLOCKED : NUM_LOCKED;
        }
    }

    return NUM_LOCK_UNKNOWN;
}

void GreeterWorker::recoveryUserKBState(std::shared_ptr<User> user)
{
    //FIXME(lxz)
    //    PowerInter powerInter("org.deepin.dde.Power1", "/org/deepin/dde/Power1", QDBusConnection::systemBus(), this);
    //    const BatteryPresentInfo info = powerInter.batteryIsPresent();
    //    const bool defaultValue = !info.values().first();
    if (user.get() == nullptr)
        return;

    const bool enabled = getNumLockState(user->name()) == NUM_UNLOCKED;

    qWarning() << "restore numlock state to " << enabled;

    // Resync numlock light with numlock status
    bool cur_numlock = KeyboardMonitor::instance()->isNumlockOn();
    KeyboardMonitor::instance()->setNumlockStatus(!cur_numlock);
    KeyboardMonitor::instance()->setNumlockStatus(cur_numlock);

    KeyboardMonitor::instance()->setNumlockStatus(enabled);
}

void GreeterWorker::restartResetSessionTimer()
{
    if (m_model->visible() && m_resetSessionTimer->isActive()) {
        m_resetSessionTimer->start();
    }
}

void GreeterWorker::startGreeterAuth(const QString &account)
{
    m_greeter->authenticate(account);
}

void GreeterWorker::changePasswd()
{
    m_model->updateMFAFlag(false);
    m_model->setAuthType(AT_PAM);
}
