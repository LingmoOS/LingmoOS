// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "userinfo.h"

#include "constants.h"

#include <grp.h>
#include <memory>
#include <pwd.h>
#include <unistd.h>

#define DEFAULT_AVATAR ":/img/default_avatar.svg"
#define DEFAULT_BACKGROUND "/usr/share/backgrounds/default_background.jpg"

User::User(QObject *parent)
    : QObject(parent)
    , m_isAutomaticLogin(false)
    , m_isLogin(false)
    , m_isNoPasswordLogin(false)
    , m_isPasswordValid(true)
    , m_isUse24HourFormat(true)
    , m_expiredDayLeft(0)
    , m_expiredState(ExpiredNormal)
    , m_lastAuthType(0)
    , m_shortDateFormat(0)
    , m_shortTimeFormat(0)
    , m_weekdayFormat(0)
    , m_accountType(-1)
    , m_uid(INT_MAX)
    , m_avatar(DEFAULT_AVATAR)
    , m_greeterBackground(DEFAULT_BACKGROUND)
    , m_locale(qgetenv("LANG"))
    , m_name("...")
    , m_desktopBackgrounds(DEFAULT_BACKGROUND)
    , m_limitsInfo(new QMap<int, LimitsInfo>())
{
}

User::User(const User &user)
    : QObject(user.parent())
    , m_isAutomaticLogin(user.m_isAutomaticLogin)
    , m_isLogin(user.m_isLogin)
    , m_isNoPasswordLogin(user.m_isNoPasswordLogin)
    , m_isPasswordValid(user.m_isPasswordValid)
    , m_isUse24HourFormat(user.m_isUse24HourFormat)
    , m_expiredDayLeft(user.m_expiredDayLeft)
    , m_expiredState(user.m_expiredState)
    , m_lastAuthType(0)
    , m_shortDateFormat(user.m_shortDateFormat)
    , m_shortTimeFormat(user.m_shortTimeFormat)
    , m_weekdayFormat(user.m_weekdayFormat)
    , m_accountType(user.m_accountType)
    , m_uid(user.m_uid)
    , m_avatar(user.m_avatar)
    , m_fullName(user.m_fullName)
    , m_greeterBackground(user.m_greeterBackground)
    , m_keyboardLayout(user.m_keyboardLayout)
    , m_locale(user.m_locale)
    , m_name(user.m_name)
    , m_passwordHint(user.m_passwordHint)
    , m_desktopBackgrounds(user.m_desktopBackgrounds)
    , m_keyboardLayoutList(user.m_keyboardLayoutList)
    , m_limitsInfo(new QMap<int, LimitsInfo>(*user.m_limitsInfo))
{
}

User::~User()
{
    delete m_limitsInfo;
}

bool User::operator==(const User &user) const
{
    return user.uid() == m_uid && user.name() == m_name;
}

/**
 * @brief 更新登录状态
 *
 * @param isLogin
 */
void User::updateLoginState(const bool isLogin)
{
    if (isLogin == m_isLogin) {
        return;
    }
    m_isLogin = isLogin;
    emit loginStateChanged(isLogin);
}

/**
 * @brief 更新账户限制信息
 *
 * @param info
 */
void User::updateLimitsInfo(const QString &info)
{
    LimitsInfo limitsInfoTmp;
    const QJsonDocument limitsInfoDoc = QJsonDocument::fromJson(info.toUtf8());
    const QJsonArray limitsInfoArr = limitsInfoDoc.array();
    for (const QJsonValue &limitsInfoStr : limitsInfoArr) {
        const QJsonObject limitsInfoObj = limitsInfoStr.toObject();
        limitsInfoTmp.unlockSecs = limitsInfoObj["unlockSecs"].toVariant().toUInt();
        limitsInfoTmp.maxTries = limitsInfoObj["maxTries"].toVariant().toUInt();
        limitsInfoTmp.numFailures = limitsInfoObj["numFailures"].toVariant().toUInt();
        limitsInfoTmp.locked = limitsInfoObj["locked"].toBool();
        limitsInfoTmp.unlockTime = limitsInfoObj["unlockTime"].toString();
        m_limitsInfo->insert(limitsInfoObj["flag"].toInt(), limitsInfoTmp);
    }
    emit limitsInfoChanged(m_limitsInfo);
}

/**
 * @brief 设置上次成功的认证
 *
 * @param type
 */
void User::setLastAuthType(const int type)
{
    if (m_lastAuthType == type) {
        return;
    }
    m_lastAuthType = type;
}

bool User::checkUserIsNoPWGrp(const User *user) const
{
    if (user->type() == User::ADDomain) {
        return false;
    }

    // Caution: 32 here is unreliable, and there may be more
    // than this number of groups that the user joins.

    int ngroups = 32;
    gid_t groups[32];
    struct passwd *pw = nullptr;
    struct group *gr = nullptr;

    /* Fetch passwd structure (contains first group ID for user) */
    pw = getpwnam(user->name().toUtf8().data());
    if (pw == nullptr) {
        qDebug() << "fetch passwd structure failed, username: " << user->name();
        return false;
    }

    /* Retrieve group list */

    if (getgrouplist(user->name().toUtf8().data(), pw->pw_gid, groups, &ngroups) == -1) {
        fprintf(stderr, "getgrouplist() returned -1; ngroups = %d\n",
                ngroups);
        return false;
    }

    /* Display list of retrieved groups, along with group names */

    for (int i = 0; i < ngroups; i++) {
        gr = getgrgid(groups[i]);
        if (gr != nullptr && QString(gr->gr_name) == QString("nopasswdlogin")) {
            return true;
        }
    }

    return false;
}

QString User::toLocalFile(const QString &path) const
{
    QUrl url(path);

    if (url.isLocalFile()) {
        return url.path();
    } else {
        return url.url();
    }
}

QString User::userPwdName(const uid_t uid) const
{
    //服务器版root用户的uid为0,需要特殊处理
    if (uid < 1000 && uid != 0)
        return QString();

    struct passwd *pw = nullptr;
    /* Fetch passwd structure (contains first group ID for user) */
    pw = getpwuid(uid);

    return pw ? QString().fromLocal8Bit(pw->pw_name) : QString();
}

NativeUser::NativeUser(const QString &path, QObject *parent)
    : User(parent)
    , m_path(path)
    , m_userInter(new UserInter("org.deepin.dde.Accounts1", path, QDBusConnection::systemBus(), this))
{
    initConnections();
    initData();
    initConfiguration(DDESESSIONCC::CONFIG_FILE + m_name);
}

NativeUser::NativeUser(const uid_t &uid, QObject *parent)
    : User(parent)
    , m_path("/org/deepin/dde/Accounts1/User" + QString::number(uid))
    , m_userInter(new UserInter("org.deepin.dde.Accounts1", m_path, QDBusConnection::systemBus(), this))
{
    initConnections();
    initData();
    initConfiguration(DDESESSIONCC::CONFIG_FILE + m_name);
}

NativeUser::NativeUser(const NativeUser &user)
    : User(user)
    , m_path(user.path())
    , m_userInter(new UserInter("org.deepin.dde.Accounts1", m_path, QDBusConnection::systemBus(), this))
{
    initConnections();
}

void NativeUser::initConnections()
{
    connect(m_userInter, &UserInter::AutomaticLoginChanged, this, &NativeUser::updateAutomaticLogin);
    connect(m_userInter, &UserInter::FullNameChanged, this, &NativeUser::updateFullName);
    connect(m_userInter, &UserInter::GreeterBackgroundChanged, this, &NativeUser::updateGreeterBackground);
    connect(m_userInter, &UserInter::HistoryLayoutChanged, this, &NativeUser::updateKeyboardLayoutList);
    connect(m_userInter, &UserInter::IconFileChanged, this, &NativeUser::updateAvatar);
    connect(m_userInter, &UserInter::LayoutChanged, this, &NativeUser::updateKeyboardLayout);
    connect(m_userInter, &UserInter::LocaleChanged, this, &NativeUser::updateLocale);
    connect(m_userInter, &UserInter::NoPasswdLoginChanged, this, &NativeUser::updateNoPasswordLogin);
    connect(m_userInter, &UserInter::PasswordHintChanged, this, &NativeUser::updatePasswordHint);
    connect(m_userInter, &UserInter::PasswordStatusChanged, this, &NativeUser::updatePasswordStatus);
    connect(m_userInter, &UserInter::PasswordHintChanged, this, &NativeUser::updatePasswordHint);
    connect(m_userInter, &UserInter::ShortDateFormatChanged, this, &NativeUser::updateShortDateFormat);
    connect(m_userInter, &UserInter::ShortTimeFormatChanged, this, &NativeUser::updateShortTimeFormat);
    connect(m_userInter, &UserInter::WeekdayFormatChanged, this, &NativeUser::updateWeekdayFormat);
    connect(m_userInter, &UserInter::AccountTypeChanged, this, &NativeUser::updateAccountType);
    connect(m_userInter, &UserInter::UidChanged, this, &NativeUser::updateUid);
    connect(m_userInter, &UserInter::UserNameChanged, this, &NativeUser::updateName);
    connect(m_userInter, &UserInter::Use24HourFormatChanged, this, &NativeUser::updateUse24HourFormat);
    connect(m_userInter, &UserInter::PasswordLastChangeChanged, this, &NativeUser::updatePasswordExpiredInfo);
    connect(m_userInter, &UserInter::MaxPasswordAgeChanged, this, &NativeUser::updatePasswordExpiredInfo);
}

void NativeUser::initData()
{
    m_isAutomaticLogin = m_userInter->automaticLogin();
    m_isNoPasswordLogin = m_userInter->noPasswdLogin();
    m_isPasswordValid = (m_userInter->passwordStatus() == "P");
    m_isUse24HourFormat = m_userInter->use24HourFormat();

    m_expiredState = m_userInter->PasswordExpiredInfo(m_expiredDayLeft).value();
    m_shortDateFormat = m_userInter->shortDateFormat();
    m_shortTimeFormat = m_userInter->shortTimeFormat();
    m_weekdayFormat = m_userInter->weekdayFormat();
    m_accountType = m_userInter->accountType();

    const QString avatarPath = toLocalFile(m_userInter->iconFile());
    if (!avatarPath.isEmpty() && QFile(avatarPath).exists() && QFile(avatarPath).size() && QImageReader(avatarPath).canRead()) {
        m_avatar = avatarPath;
    }
    m_fullName = m_userInter->fullName();
    const QString backgroundPath = toLocalFile(m_userInter->greeterBackground());
    if (!backgroundPath.isEmpty() && QFile(backgroundPath).exists() && QFile(backgroundPath).size() && QImageReader(backgroundPath).canRead()) {
        m_greeterBackground = backgroundPath;
    }
    m_keyboardLayout = m_userInter->layout();
    m_locale = m_userInter->locale();
    m_name = m_userInter->userName();
    m_passwordHint = m_userInter->passwordHint();
    m_desktopBackgrounds = m_userInter->desktopBackgrounds();
    m_keyboardLayoutList = m_userInter->historyLayout();
    m_uid = m_userInter->uid().toUInt();
}

/**
 * @brief unnecessary
 *
 * @param config
 */
void NativeUser::initConfiguration(const QString &config)
{
    QSettings settings(config, QSettings::IniFormat);
    settings.beginGroup("User");

    //QSettings中分号为结束符，分号后面的数据不会被读取，因此使用QFile直接读取桌面背景图片路径
    QStringList desktopBackgrounds = readDesktopBackgroundPath(config);

    int index = settings.value("Workspace").toInt();
    //刚安装的系统中返回的Workspace为0
    index = index <= 0 ? 1 : index;
    if (desktopBackgrounds.size() >= index && index > 0) {
        // m_desktopBackgrounds = toLocalFile(desktopBackgrounds.at(index - 1));
    } else {
        qDebug() << "configAccountInfo get error index:" << index << ", backgrounds:" << desktopBackgrounds;
    }
}

/**
 * @brief 设置键盘布局
 *
 * @param keyboard
 */
void NativeUser::setKeyboardLayout(const QString &keyboard)
{
    m_userInter->SetLayout(keyboard);
}

/**
 * @brief 更新头像
 *
 * @param path
 */
void NativeUser::updateAvatar(const QString &path)
{
    const QString pathTmp = toLocalFile(path);
    if (pathTmp == m_avatar) {
        return;
    }

    if (!pathTmp.isEmpty() && QFile(pathTmp).exists() && QFile(pathTmp).size() && QImageReader(pathTmp).canRead()) {
        m_avatar = pathTmp;
    } else {
        m_avatar = DEFAULT_AVATAR;
    }
    emit avatarChanged(pathTmp);
}

/**
 * @brief 更新账户自动登录状态
 *
 * @param autoLoginState
 */
void NativeUser::updateAutomaticLogin(const bool autoLoginState)
{
    if (autoLoginState == m_isAutomaticLogin) {
        return;
    }
    m_isAutomaticLogin = autoLoginState;
    emit autoLoginStateChanged(autoLoginState);
}

/**
 * @brief 更新用户全名
 *
 * @param name
 */
void NativeUser::updateFullName(const QString &fullName)
{
    if (fullName == m_fullName) {
        return;
    }
    m_fullName = fullName;
    emit displayNameChanged(m_fullName.isEmpty() ? m_name : m_fullName);
}

/**
 * @brief 更新登录背景
 *
 * @param path
 */
void NativeUser::updateGreeterBackground(const QString &path)
{
    const QString pathTmp = toLocalFile(path);
    if (pathTmp == m_greeterBackground) {
        return;
    }
    m_greeterBackground = pathTmp;
    emit greeterBackgroundChanged(pathTmp);
}

/**
 * @brief 更新键盘布局
 *
 * @param keyboard
 */
void NativeUser::updateKeyboardLayout(const QString &keyboardLayout)
{
    if (keyboardLayout == m_keyboardLayout) {
        return;
    }
    m_keyboardLayout = keyboardLayout;
    emit keyboardLayoutChanged(keyboardLayout);
}

/**
 * @brief 更新键盘布局列表
 *
 * @param list
 */
void NativeUser::updateKeyboardLayoutList(const QStringList &keyboardLayoutList)
{
    if (keyboardLayoutList == m_keyboardLayoutList) {
        return;
    }
    m_keyboardLayoutList = keyboardLayoutList;
    emit keyboardLayoutListChanged(keyboardLayoutList);
}

/**
 * @brief 更新语言环境
 *
 * @param locale
 */
void NativeUser::updateLocale(const QString &locale)
{
    if (locale == m_locale) {
        return;
    }
    m_locale = locale;
    emit localeChanged(locale);
}

/**
 * @brief 更新用户名
 *
 * @param name
 */
void NativeUser::updateName(const QString &name)
{
    if (name == m_name) {
        return;
    }
    m_name = name;
    emit displayNameChanged(m_fullName.isEmpty() ? name : m_fullName);
}

/**
 * @brief 更新无密码登录状态
 *
 * @param isNoPasswdLogin
 */
void NativeUser::updateNoPasswordLogin(const bool isNoPasswordLogin)
{
    if (isNoPasswordLogin == m_isNoPasswordLogin) {
        return;
    }
    m_isNoPasswordLogin = isNoPasswordLogin;
    emit noPasswordLoginChanged(isNoPasswordLogin);
}

/**
 * @brief 输入密码时的提示
 *
 * @param hint
 */

void NativeUser::updatePasswordHint(const QString &hint)
{
    if (hint == m_passwordHint) {
        return;
    }
    m_passwordHint = hint;
    emit passwordHintChanged(hint);
}

/**
 * @brief 更新账户的密码过期信息
 */
void NativeUser::updatePasswordExpiredInfo()
{
    m_expiredState = m_userInter->PasswordExpiredInfo(m_expiredDayLeft).value();

    emit passwordExpiredInfoChanged();
}

/**
 * @brief 更新密码状态，用户是否设置了密码
 *
 * @param state 有密码：P 无密码：NP
 */
void NativeUser::updatePasswordStatus(const QString &state)
{
    const bool isPasswordValidTmp = ((state == "P") ? true : false);
    if (isPasswordValidTmp == m_isPasswordValid) {
        return;
    }
    m_isPasswordValid = isPasswordValidTmp;
}

/**
 * @brief NativeUser::updateShortDateFormat
 *
 * @param format
 */
void NativeUser::updateShortDateFormat(const int format)
{
    if (format == m_shortDateFormat) {
        return;
    }
    m_shortDateFormat = format;
    emit shortDateFormatChanged(format);
}

/**
 * @brief 更新短时间格式
 *
 * @param format
 */
void NativeUser::updateShortTimeFormat(const int format)
{
    if (format == m_shortTimeFormat) {
        return;
    }
    m_shortTimeFormat = format;
    emit shortTimeFormatChanged(format);
}

/**
 * @brief 更新星期格式
 *
 * @param format
 */
void NativeUser::updateWeekdayFormat(const int format)
{
    if (format == m_weekdayFormat) {
        return;
    }
    m_weekdayFormat = format;
    emit weekdayFormatChanged(format);
}

void NativeUser::updateAccountType(const int type)
{
    if (m_accountType == type)
        return;

    m_accountType = type;
    emit accountTypeChanged(type);
}

/**
 * @brief 更新用户 uid
 *
 * @param uid
 */
void NativeUser::updateUid(const QString &uid)
{
    const uid_t uidTmp = uid.toUInt();
    if (uidTmp == m_uid) {
        return;
    }
    m_uid = uidTmp;
}

/**
 * @brief 更新24小时制
 *
 * @param is24HourFormat
 */
void NativeUser::updateUse24HourFormat(const bool is24HourFormat)
{
    if (is24HourFormat == m_isUse24HourFormat) {
        return;
    }
    m_isUse24HourFormat = is24HourFormat;
    emit use24HourFormatChanged(is24HourFormat);
}

QStringList NativeUser::readDesktopBackgroundPath(const QString &path)
{
    QStringList desktopBackgrounds;
    QFile file(path);
    if (file.exists()) {
        if (file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            QTextStream read(&file);
            while (!read.atEnd()) {
                QString line = read.readLine();
                if (line.contains("DesktopBackgrounds=", Qt::CaseInsensitive)) {
                    QString paths = line.section('=', 1);
                    desktopBackgrounds << paths.split(";");
                }
            }

            file.close();
        }
    }

    return desktopBackgrounds;
}

/**
 * @brief 域管账户。当域管账户在第一次认证时，用这个特殊的类保存相关信息，
 * 一旦登录一次后，account服务中就会保存域管账户相关的信息，后续就可以当作本地用户处理。
 * 这个类仅在域管账户第一次登录时使用，如果后续有其它类型的远程账户（第一次登录时系统中无用户信息），
 * 可以参考这个类的实现再抽象出一个 RemoteUser。
 * @param uid 用户的 uid
 * @param parent
 */
ADDomainUser::ADDomainUser(uid_t uid, QObject *parent)
    : User(parent)
{
    m_uid = uid;
}

void ADDomainUser::setFullName(const QString &fullName)
{
    if (fullName == m_fullName) {
        return;
    }
    m_fullName = fullName;
    emit displayNameChanged(m_fullName.isEmpty() ? m_name : m_fullName);
}

void ADDomainUser::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    emit displayNameChanged(m_fullName.isEmpty() ? name : m_fullName);
}
