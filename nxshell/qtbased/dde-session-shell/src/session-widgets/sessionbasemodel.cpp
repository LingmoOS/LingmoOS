// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionbasemodel.h"

#include <DSysInfo>

#include <QDebug>
#include <QGSettings>

#define SessionManagerService "org.deepin.dde.SessionManager1"
#define SessionManagerPath "/org/deepin/dde/SessionManager1"

DCORE_USE_NAMESPACE

SessionBaseModel::SessionBaseModel(QObject *parent)
    : QObject(parent)
    , m_hasSwap(false)
    , m_visible(false)
    , m_isServerModel(false)
    , m_canSleep(false)
    , m_allowShowUserSwitchButton(false)
    , m_alwaysShowUserSwitchButton(false)
    , m_abortConfirm(false)
    , m_isLockNoPassword(false)
    , m_isBlackMode(false)
    , m_isHibernateMode(false)
    , m_isLock(false)
    , m_allowShowCustomUser(false)
    , m_SEOpen(false)
    , m_isUseWayland(QGuiApplication::platformName().startsWith("wayland", Qt::CaseInsensitive))
    , m_userListSize(0)
    , m_appType(AuthCommon::None)
    , m_currentUser(nullptr)
    , m_lastLogoutUser(nullptr)
    , m_powerAction(PowerAction::RequireNormal)
    , m_currentModeState(ModeStatus::NoStatus)
    , m_isCheckedInhibit(false)
    , m_authProperty {false, false, Unavailable, AuthCommon::None, AuthCommon::None, 0, "", "", ""}
    , m_users(new QMap<QString, std::shared_ptr<User>>())
    , m_loginedUsers(new QMap<QString, std::shared_ptr<User>>())
{
}

SessionBaseModel::~SessionBaseModel()
{
    delete m_users;
    delete m_loginedUsers;
}

std::shared_ptr<User> SessionBaseModel::findUserByUid(const uint uid) const
{
    for (auto user : qAsConst(*m_users)) {
        if (user->uid() == uid) {
            return user;
        }
    }
    return std::shared_ptr<User>(nullptr);
}

std::shared_ptr<User> SessionBaseModel::findUserByName(const QString &name) const
{
    if (name.isEmpty()) {
        return std::shared_ptr<User>(nullptr);
    }

    for (auto user : qAsConst(*m_users)) {
        if (user->name() == name) {
            return user;
        }
    }
    return std::shared_ptr<User>(nullptr);
}

void SessionBaseModel::setAppType(const AppType type)
{
    m_appType = type;
}

void SessionBaseModel::setSessionKey(const QString &sessionKey)
{
    qDebug("set session key: %s", qPrintable(sessionKey));

    if (m_sessionKey == sessionKey)
        return;

    m_sessionKey = sessionKey;

    emit onSessionKeyChanged(sessionKey);
}

void SessionBaseModel::setPowerAction(const PowerAction &powerAction)
{
    qDebug("set power action: %d", powerAction);

    if (powerAction == m_powerAction)
        return;

    m_powerAction = powerAction;

    emit onPowerActionChanged(powerAction);
}

void SessionBaseModel::setCurrentModeState(const ModeStatus &currentModeState)
{
    qDebug("set current mode state: %d", currentModeState);

    if (m_currentModeState == currentModeState)
        return;

    m_currentModeState = currentModeState;

    emit onStatusChanged(currentModeState);
}

void SessionBaseModel::setUserListSize(int users_size)
{
    if (m_userListSize == users_size)
        return;

    m_userListSize = users_size;

    emit onUserListSizeChanged(users_size);
}

void SessionBaseModel::setHasVirtualKB(bool hasVirtualKB)
{
    //锁屏显示时，加载初始化屏幕键盘onboard进程，锁屏完成后结束onboard进程
    if (hasVirtualKB && !qgetenv("XDG_SESSION_TYPE").contains("wayland")) {
        bool b = QProcess::execute("which", QStringList() << "onboard") == 0;
        emit hasVirtualKBChanged(b);
    } else {
        emit hasVirtualKBChanged(false);
    }
}

void SessionBaseModel::setHasSwap(bool hasSwap)
{
    if (m_hasSwap == hasSwap)
        return;

    m_hasSwap = hasSwap;

    emit onHasSwapChanged(hasSwap);
}

/**
 * @brief 设置界面显示状态
 *
 * @param visible
 */
void SessionBaseModel::setVisible(const bool visible)
{
    qDebug() << "set visible: " << visible;

    if (m_visible == visible) {
        return;
    }
    m_visible = visible;

    emit visibleChanged(m_visible);
}

void SessionBaseModel::setCanSleep(bool canSleep)
{
    if (m_canSleep == canSleep)
        return;

    m_canSleep = canSleep;

    emit canSleepChanged(canSleep);
}

void SessionBaseModel::setAllowShowUserSwitchButton(bool allowShowUserSwitchButton)
{
    if (m_allowShowUserSwitchButton == allowShowUserSwitchButton)
        return;

    m_allowShowUserSwitchButton = allowShowUserSwitchButton;

    emit allowShowUserSwitchButtonChanged(allowShowUserSwitchButton);
}

void SessionBaseModel::setAlwaysShowUserSwitchButton(bool alwaysShowUserSwitchButton)
{
    m_alwaysShowUserSwitchButton = alwaysShowUserSwitchButton;
}

void SessionBaseModel::setIsServerModel(bool server_model)
{
    if (m_isServerModel == server_model)
        return;

    m_isServerModel = server_model;
}

void SessionBaseModel::setAbortConfirm(bool abortConfirm)
{
    m_abortConfirm = abortConfirm;
    emit abortConfirmChanged(abortConfirm);
}

void SessionBaseModel::setIsBlackMode(bool is_black)
{
    if (m_isBlackMode == is_black)
        return;

    m_isBlackMode = is_black;
    emit blackModeChanged(is_black);
}

void SessionBaseModel::setIsHibernateModel(bool is_Hibernate)
{
    if (m_isHibernateMode == is_Hibernate)
        return;

    m_isHibernateMode = is_Hibernate;
    emit HibernateModeChanged(is_Hibernate);
}

void SessionBaseModel::setIsCheckedInhibit(bool checked)
{
    if (m_isCheckedInhibit == checked)
        return;

    m_isCheckedInhibit = checked;
}

/**
 * @brief 显示自定义用户
 *
 * @param allowShowCustomUser
 */
void SessionBaseModel::setAllowShowCustomUser(const bool allowShowCustomUser)
{
    if (allowShowCustomUser == m_allowShowCustomUser)
        return;

    m_allowShowCustomUser = allowShowCustomUser;
}

/**
 * @brief 认证类型
 *
 * @param type
 */
void SessionBaseModel::setAuthType(const int type)
{
    qDebug("set auth type: %d", type);

    if (type == m_authProperty.AuthType) {
        return;
    }
    if (m_currentUser->type() == User::Default) {
        m_authProperty.AuthType = type;
        emit authTypeChanged(AT_None);
    } else {
        m_authProperty.AuthType = type;
        emit authTypeChanged(type);
    }
}

/**
 * @brief 新增用户
 *
 * @param path 用户路径或 Uid
 */
void SessionBaseModel::addUser(const QString &path)
{
    qDebug("add user, path: %s", qPrintable(path));

    if (m_users->contains(path)) {
        return;
    }
    std::shared_ptr<NativeUser> user(new NativeUser(path));
    m_users->insert(path, user);
    emit userAdded(user);
    emit userListChanged(m_users->values());
}

/**
 * @brief 新增用户
 *
 * @param user
 */
void SessionBaseModel::addUser(const std::shared_ptr<User> user)
{
    qDebug("add user, path: %s, name: %s", qPrintable(user->path()), qPrintable(user->name()));

    const QList<std::shared_ptr<User>> userList = m_users->values();
    if (userList.contains(user)) {
        return;
    }
    const QString path = user->path().isEmpty() ? QString::number(user->uid()) : user->path();
    m_users->insert(path, user);
    emit userAdded(user);
}

/**
 * @brief 删除用户
 *
 * @param path 用户路径或 Uid
 */
void SessionBaseModel::removeUser(const QString &path)
{
    if (!m_users->contains(path)) {
        return;
    }
    const std::shared_ptr<User> user = m_users->value(path);
    m_users->remove(path);
    emit userRemoved(user);
    emit userListChanged(m_users->values());
}

/**
 * @brief 删除用户
 *
 * @param user
 */
void SessionBaseModel::removeUser(const std::shared_ptr<User> user)
{
    qDebug("remove user, name: %s, id: %d", qPrintable(user->name()), user->uid());

    const QList<std::shared_ptr<User>> userList = m_users->values();
    if (!userList.contains(user)) {
        return;
    }

    const QString path = user->path().isEmpty() ? QString::number(user->uid()) : user->path();
    m_users->remove(path);
    emit userRemoved(user);
}

/**
 * @brief json数据转成user对象
 *
 * @param userJson
 */
std::shared_ptr<User> SessionBaseModel::json2User(const QString &userJson)
{
    qDebug("user json data: %s", qPrintable(userJson));

    std::shared_ptr<User> user_ptr;
    QJsonParseError jsonParseError;
    const QJsonDocument userDoc = QJsonDocument::fromJson(userJson.toUtf8(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError || userDoc.isEmpty()) {
        qWarning("failed to obtain current user information from LockService!");
        return user_ptr;
    }

    const QJsonObject userObj = userDoc.object();
    const QString &name = userObj["Name"].toString();
    user_ptr = findUserByName(name);
    if (name.isEmpty() || user_ptr == nullptr) {
        const uid_t uid = static_cast<uid_t>(userObj["Uid"].toInt());
        user_ptr = findUserByUid(uid);
    }
    if (user_ptr)
        user_ptr->setLastAuthType(userObj["AuthType"].toInt());

    return user_ptr;
}

/**
 * @brief 保存当前用户信息
 *
 * @param userJson
 * @return 是否更新了用户
 */
bool SessionBaseModel::updateCurrentUser(const QString &userJson)
{
    qDebug("update current user, data: %s", qPrintable(userJson));

    std::shared_ptr<User> user_ptr = json2User(userJson);
    if (!user_ptr)
        user_ptr = m_lastLogoutUser ? m_lastLogoutUser : m_users->first();

    return updateCurrentUser(user_ptr);
}

/**
 * @brief 保存当前用户信息
 *
 * @param user
 * @return 是否更新了用户
 */
bool SessionBaseModel::updateCurrentUser(const std::shared_ptr<User> user)
{
    if (!user) {
        qWarning() << "Failed to set current user!" << user.get();
        return false;
    }

    qDebug() << "SessionBaseModel::updateCurrentUser:" << user->name();

    if (m_currentUser && *m_currentUser == *user) {
        return false;
    }

    m_currentUser = user;
    emit currentUserChanged(user);

    return true;
}

/**
 * @brief 更新用户列表。当用户增加或减少时，会将新的用户列表通过信号发过来。
 *
 * @param list
 */
void SessionBaseModel::updateUserList(const QStringList &list)
{
    qDebug() << "update user list: " << list;

    QStringList listTmp = m_users->keys();
    for (const QString &path : list) {
        if (m_users->contains(path)) {
            listTmp.removeAll(path);
        } else {
            std::shared_ptr<NativeUser> user(new NativeUser(path));
            m_users->insert(path, user);
        }
    }
    for (const QString &path : qAsConst(listTmp)) {
        m_users->remove(path);
    }
    emit userListChanged(m_users->values());
}

/**
 * @brief 更新上一个登录用户
 *
 * @param uid
 */
void SessionBaseModel::updateLastLogoutUser(const uid_t uid)
{
    qDebug() << "SessionBaseModel::updateLastLogoutUser:" << uid;
    QList<std::shared_ptr<User>> userList = m_users->values();
    auto it = std::find_if(userList.begin(), userList.end(), [uid](std::shared_ptr<User> &user) {
        return user->uid() == uid;
    });
    if (it != userList.end()) {
        updateLastLogoutUser(it.i->t());
    }
}

/**
 * @brief 设置上一个登录的用户
 *
 * @param lastLogoutUser
 */
void SessionBaseModel::updateLastLogoutUser(const std::shared_ptr<User> lastLogoutUser)
{
    if (!lastLogoutUser.get() || m_lastLogoutUser == lastLogoutUser) {
        return;
    }
    qInfo() << "last logout user:" << lastLogoutUser->name() << lastLogoutUser->uid();

    m_lastLogoutUser = lastLogoutUser;
}

/**
 * @brief 更新已登录用户列表
 *
 * @param list
 */
void SessionBaseModel::updateLoginedUserList(const QString &list)
{
    qDebug() << "update logined user list: " << list;

    QList<QString> loginedUsersTmp = m_loginedUsers->keys();
    QJsonParseError jsonParseError;
    const QJsonDocument loginedUserListDoc = QJsonDocument::fromJson(list.toUtf8(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError) {
        qWarning("The logined user list is wrong!");
        return;
    }
    const QJsonObject loginedUserListDocObj = loginedUserListDoc.object();
    for (const QString &loginedUserListDocKey : loginedUserListDocObj.keys()) {
        const QJsonArray loginedUserListArr = loginedUserListDocObj.value(loginedUserListDocKey).toArray();
        for (const QJsonValue &loginedUserStr : loginedUserListArr) {
            const QJsonObject loginedUserListObj = loginedUserStr.toObject();
            const int uid = loginedUserListObj["Uid"].toInt();
            const QString &desktop = loginedUserListObj["Desktop"].toString();
            if ((uid != 0 && uid < 1000) || desktop.isEmpty()) {
                // 排除非正常登录用户
                continue;
            }
            const QString path = QString("/org/deepin/dde/Accounts1/User") + QString::number(uid);
            if (!m_loginedUsers->contains(path)) {
                // 对于通过自定义窗口输入的账户(域账户), 此时账户还没添加进来，导致下面m_users->value(path)为空指针，调用会导致程序奔溃
                // 因此在登录时，对于新增的账户，调用addUser先将账户添加进来，然后再去更新对应账户的登录状态
                addUser(path);
                m_loginedUsers->insert(path, m_users->value(path));
                m_users->value(path)->updateLoginState(true);
            } else {
                loginedUsersTmp.removeAll(path);
            }
        }
    }
    for (const QString &path : qAsConst(loginedUsersTmp)) {
        m_loginedUsers->remove(path);
        m_users->value(path)->updateLoginState(false);
    }

    qInfo() << "Logined users: " << m_loginedUsers->keys();
    emit loginedUserListChanged(m_loginedUsers->values());
}

/**
 * @brief 更新账户限制信息
 *
 * @param info
 */
void SessionBaseModel::updateLimitedInfo(const QString &info)
{
    qDebug("update limit info: %s", qPrintable(info));

    m_currentUser->updateLimitsInfo(info);
}

/**
 * @brief 认证框架类型
 *
 * @param state
 */
void SessionBaseModel::updateFrameworkState(const int state)
{
    qDebug("update framework state: %d", state);

    if (state == m_authProperty.FrameworkState) {
        return;
    }
    m_authProperty.FrameworkState = state;
}

/**
 * @brief 支持的认证类型
 *
 * @param flags
 */
void SessionBaseModel::updateSupportedMixAuthFlags(const int flags)
{
    if (flags == m_authProperty.MixAuthFlags)
        return;

    m_authProperty.MixAuthFlags = flags;
}

/**
 * @brief 支持的加密类型
 *
 * @param type
 */
void SessionBaseModel::updateSupportedEncryptionType(const QString &type)
{
    if (type == m_authProperty.EncryptionType)
        return;

    m_authProperty.EncryptionType = type;
}

/**
 * @brief 模糊多因子信息，供 PAM 使用，暂时用上
 *
 * @param fuzzMFA
 */
void SessionBaseModel::updateFuzzyMFA(const bool fuzzMFA)
{
    if (fuzzMFA == m_authProperty.FuzzyMFA)
        return;

    m_authProperty.FuzzyMFA = fuzzMFA;
}

/**
 * @brief 多因子标志
 *
 * @param MFAFlag true: 使用多因子  false: 使用单因子
 */
void SessionBaseModel::updateMFAFlag(const bool MFAFlag)
{
    qDebug() << "update MFA flag: " << MFAFlag;

    if (MFAFlag == m_authProperty.MFAFlag)
        return;

    m_authProperty.MFAFlag = MFAFlag;
    emit MFAFlagChanged(MFAFlag);
}

/**
 * @brief PIN 最大长度
 * @param length
 */
void SessionBaseModel::updatePINLen(const int PINLen)
{
    if (PINLen == m_authProperty.PINLen)
        return;

    m_authProperty.PINLen = PINLen;
}

/**
 * @brief 总的提示语
 *
 * @param prompt 提示语
 */
void SessionBaseModel::updatePrompt(const QString &prompt)
{
    if (prompt == m_authProperty.Prompt)
        return;

    m_authProperty.Prompt = prompt;
}

/**
 * @brief 更新多因子信息
 *
 * @param info
 */
void SessionBaseModel::updateFactorsInfo(const MFAInfoList &infoList)
{
    qDebug() << "update factors info: " << infoList;
    m_authProperty.AuthType = AT_None;
    switch (m_authProperty.FrameworkState) {
    case Available:
        for (const MFAInfo &info : infoList) {
            m_authProperty.AuthType |= info.AuthType;
        }
        emit authTypeChanged(m_authProperty.AuthType);
        break;
    default:
        m_authProperty.AuthType = AT_PAM;
        emit authTypeChanged(AT_PAM);
        break;
    }
}

/**
 * @brief 更新认证状态
 *
 * @param currentAuthType
 * @param state
 * @param result
 */
void SessionBaseModel::updateAuthState(const int type, const int state, const QString &message)
{
    qDebug("update auth state, type: %d, state: %d, result: %s", type, state, qPrintable(message));

    switch (m_authProperty.FrameworkState) {
    case Available:
        emit authStateChanged(type, state, message);
        break;
    default:
        emit authStateChanged(AT_PAM, state, message);
        break;
    }
}
