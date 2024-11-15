/****************************************************************************
 *
 * Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPLv3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include "useraccount.h"
#include "useraccount_p.h"

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

namespace QtAccountsService {

/*
 * UserAccountPrivate
 */

UserAccountPrivate::UserAccountPrivate(UserAccount *q)
    : bus(QDBusConnection::systemBus())
    , user(nullptr)
    , accountType(UserAccount::StandardAccountType)
    , locked(false)
    , automaticLogin(false)
    , passwordMode(UserAccount::NonePasswordMode)
    , q_ptr(q)
{
}

void UserAccountPrivate::initialize(const QDBusConnection &connection, const QString &objectPath)
{
    Q_Q(UserAccount);

    bus = connection;

    if (user) {
        q->disconnect(user, &OrgFreedesktopAccountsUserInterface::Changed, q,
                      &UserAccount::handleAccountChanged);
        user = nullptr;
    }

    user = new OrgFreedesktopAccountsUserInterface(QStringLiteral("org.freedesktop.Accounts"),
                                                   objectPath, bus, q);
    q->connect(user, &OrgFreedesktopAccountsUserInterface::Changed, q,
               &UserAccount::handleAccountChanged);

    emitSignals();
}

void UserAccountPrivate::emitSignals()
{
    Q_Q(UserAccount);

    Q_EMIT q->userIdChanged();
    Q_EMIT q->groupIdChanged();
    Q_EMIT q->accountTypeChanged();
    Q_EMIT q->lockedChanged();
    Q_EMIT q->automaticLoginChanged();
    Q_EMIT q->passwordModeChanged();
    Q_EMIT q->userNameChanged();
    Q_EMIT q->realNameChanged();
    Q_EMIT q->displayNameChanged();
    Q_EMIT q->homeDirectoryChanged();
    Q_EMIT q->shellChanged();
    Q_EMIT q->iconFileNameChanged();
    Q_EMIT q->emailChanged();
    Q_EMIT q->languageChanged();
    Q_EMIT q->locationChanged();
    Q_EMIT q->xsessionChanged();
}

/*!
    \class UserAcccount
    \brief The UserAccount class describes user accounts.

    This class describes user accounts retrieved with AccountsManager.

    \author Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>

    \sa AccountsManager
*/

/*!
    Constructs a UserAccount object for the currently logged in user.
*/
UserAccount::UserAccount(const QDBusConnection &bus, QObject *parent)
    : QObject(parent)
    , d_ptr(new UserAccountPrivate(this))
{
    Q_D(UserAccount);

    QString objectPath =
        QStringLiteral("/org/freedesktop/Accounts/User") + QString::number(getuid());
    d->initialize(bus, objectPath);
}

/*!
    \internal

    Constructs a UserAccount object from a specific objectPath in the form of
    /org/freedesktop/Accounts/UserUID where UID is user's uid.

    \param objectPath Accounts Service object path for the user account.
*/
UserAccount::UserAccount(const QString &objectPath, const QDBusConnection &bus, QObject *parent)
    : QObject(parent)
    , d_ptr(new UserAccountPrivate(this))
{
    Q_D(UserAccount);
    d->initialize(bus, objectPath);
}

/*!
    Returns the user identifier.
*/
qlonglong UserAccount::userId() const
{
    Q_D(const UserAccount);
    return d->user->uid();
}

/*!
    Change the user identifier to \c uid.
    The object will hold information for the requested user identifier.
*/
void UserAccount::setUserId(qlonglong uid)
{
    Q_D(UserAccount);

    QString objectPath = QStringLiteral("/org/freedesktop/Accounts/User") + QString::number(uid);
    d->initialize(d->bus, objectPath);
}

/*!
    Returns group identifier.
*/
qlonglong UserAccount::groupId() const
{
    Q_D(const UserAccount);

    size_t bufSize = 0;
    long sizeMax = ::sysconf(_SC_GETPW_R_SIZE_MAX);
    if (sizeMax <= 0)
        bufSize = 16384;
    else
        bufSize = static_cast<size_t>(sizeMax);
    char *buf = static_cast<char *>(::malloc(bufSize));
    if (!buf)
        qFatal("Cannot allocate %lu bytes: %s", static_cast<long>(bufSize), strerror(errno));

    struct passwd pwd;
    struct passwd *result;
    int s = ::getpwuid_r(d->user->uid(), &pwd, buf, bufSize, &result);
    if (!result) {
        if (s == 0)
            qCritical("User with uid %lld not found", d->user->uid());
        else
            qCritical("Failed to get group information: %s", strerror(s));
        return 0;
    }

    return pwd.pw_gid;
}

/*!
    Returns the account type.
*/
UserAccount::AccountType UserAccount::accountType() const
{
    Q_D(const UserAccount);
    return static_cast<UserAccount::AccountType>(d->user->accountType());
}

/*!
    Sets the account type to \a type.

    \param type Account type.
*/
void UserAccount::setAccountType(AccountType type)
{
    Q_D(UserAccount);

    if (type == accountType())
        return;

    d->accountType = type;
    d->user->SetAccountType(static_cast<int>(type));
    Q_EMIT accountTypeChanged();
}

/*!
    Returns whether this account is locked or not.
*/
bool UserAccount::isLocked() const
{
    Q_D(const UserAccount);
    return d->user->locked();
}

/*!
    Locks or unlocks the user account.
    Locking an account prevents the user from logging in.

    \param locked Whether to lock or unlock the user account.
*/
void UserAccount::setLocked(bool locked)
{
    Q_D(UserAccount);

    if (isLocked() == locked)
        return;

    d->locked = locked;
    d->user->SetLocked(locked);
    Q_EMIT lockedChanged();
}

/*!
    Returns whether the user account will automatically log in when the
    system starts up.
*/
bool UserAccount::automaticLogin() const
{
    Q_D(const UserAccount);
    return d->user->automaticLogin();
}

/*!
    Sets whether the user account will automatically log in when
    the system starts up.

    \param automaticLogin Whether automatic login is enabled for the user.
*/
void UserAccount::setAutomaticLogin(bool automaticLogin)
{
    Q_D(UserAccount);

    if (this->automaticLogin() == automaticLogin)
        return;

    d->automaticLogin = automaticLogin;
    d->user->SetAutomaticLogin(automaticLogin);
    Q_EMIT automaticLoginChanged();
}

/*!
    Returns how often the user has logged in.
*/
qlonglong UserAccount::loginFrequency() const
{
    Q_D(const UserAccount);
    return d->user->loginFrequency();
}

/*!
    Returns the last login time.
*/
qlonglong UserAccount::loginTime() const
{
    Q_D(const UserAccount);
    return d->user->loginTime();
}

/*!
    Returns the password mode for the user account.
*/
UserAccount::PasswordMode UserAccount::passwordMode() const
{
    Q_D(const UserAccount);
    return static_cast<UserAccount::PasswordMode>(d->user->passwordMode());
}

/*!
    Sets the password mode for the user account.

    \param mode Password mode.
*/
void UserAccount::setPasswordMode(UserAccount::PasswordMode mode)
{
    Q_D(UserAccount);

    if (passwordMode() == mode)
        return;

    d->passwordMode = mode;
    d->user->SetPasswordMode(static_cast<int>(mode));
    Q_EMIT passwordModeChanged();
}

/*!
    Returns the password hint for the user.
*/
QString UserAccount::passwordHint() const
{
    Q_D(const UserAccount);
    return d->user->passwordHint();
}

/*!
    Returns whether the user is a local account or not.
*/
bool UserAccount::isLocalAccount() const
{
    Q_D(const UserAccount);
    return d->user->localAccount();
}

/*!
    Returns whether the user is a system account or not.
*/
bool UserAccount::isSystemAccount() const
{
    Q_D(const UserAccount);
    return d->user->systemAccount();
}

/*!
    Returns the user name.
*/
QString UserAccount::userName() const
{
    Q_D(const UserAccount);
    return d->user->userName();
}

/*!
    Sets the user name to \a userName.

    \param userName The new user name.
*/
void UserAccount::setUserName(const QString &userName)
{
    Q_D(UserAccount);

    if (this->userName() == userName)
        return;

    d->userName = userName;
    d->user->SetUserName(userName);
    Q_EMIT userNameChanged();
    Q_EMIT displayNameChanged();
}

/*!
    Returns user's real name.
*/
QString UserAccount::realName() const
{
    Q_D(const UserAccount);
    return d->user->realName();
}

/*!
    Sets the user's real name to \a realName.

    \param realName Real name.
*/
void UserAccount::setRealName(const QString &realName)
{
    Q_D(UserAccount);

    if (this->realName() == realName)
        return;

    d->realName = realName;
    d->user->SetRealName(realName);
    Q_EMIT realNameChanged();
    Q_EMIT displayNameChanged();
}

/*!
    Returns user's real name if not empty, otherwise the user name.
*/
QString UserAccount::displayName() const
{
    if (realName().isEmpty())
        return userName();
    return realName();
}

/*!
    Returns the home directory absolute path.
*/
QString UserAccount::homeDirectory() const
{
    Q_D(const UserAccount);
    return d->user->homeDirectory();
}

/*!
    Sets the home directory to \a homeDirectory.

    \param homeDirectory Home directory.
*/
void UserAccount::setHomeDirectory(const QString &homeDirectory)
{
    Q_D(UserAccount);

    if (this->homeDirectory() == homeDirectory)
        return;

    d->user->SetHomeDirectory(homeDirectory);
    Q_EMIT homeDirectoryChanged();
}

/*!
    Returns the login shell absolute path.
*/
QString UserAccount::shell() const
{
    Q_D(const UserAccount);
    return d->user->shell();
}

/*!
    Sets the login shell to \a shell.

    \param shell Login shell absolute path.
*/
void UserAccount::setShell(const QString &shell)
{
    Q_D(UserAccount);

    if (this->shell() == shell)
        return;

    d->shell = shell;
    d->user->SetShell(shell);
    Q_EMIT shellChanged();
}

/*!
    Returns user's picture absoulte path.
*/
QString UserAccount::iconFileName() const
{
    Q_D(const UserAccount);
    return QFile::exists(d->user->iconFile()) ? d->user->iconFile() : QString();
}

/*!
    Sets users' picture absolute path to \a fileName.

    \param fileName Picture absolute path.
*/
void UserAccount::setIconFileName(const QString &fileName)
{
    Q_D(UserAccount);

    if (iconFileName() == fileName)
        return;

    d->iconFileName = fileName;
    d->user->SetIconFile(fileName);
    Q_EMIT iconFileNameChanged();
}

/*!
    Returns user's email address.
*/
QString UserAccount::email() const
{
    Q_D(const UserAccount);
    return d->user->email();
}

/*!
    Sets user's email address to \a email.

    \param email Email address.
*/
void UserAccount::setEmail(const QString &email)
{
    Q_D(UserAccount);

    if (this->email() == email)
        return;

    d->email = email;
    d->user->SetEmail(email);
    Q_EMIT emailChanged();
}

/*!
    Returns user language.
*/
QString UserAccount::language() const
{
    Q_D(const UserAccount);
    return d->user->language();
}

/*!
    Sets user language to \a language.

    \param language Language.
*/
void UserAccount::setLanguage(const QString &language)
{
    Q_D(UserAccount);

    if (this->language() == language)
        return;

    d->language = language;
    d->user->SetLanguage(language);
    Q_EMIT languageChanged();
}

/*!
    Returns user location.
*/
QString UserAccount::location() const
{
    Q_D(const UserAccount);
    return d->user->location();
}

/*!
    Sets user \a location.

    \param location Location.
*/
void UserAccount::setLocation(const QString &location)
{
    Q_D(UserAccount);

    if (this->location() == location)
        return;

    d->location = location;
    d->user->SetLocation(location);
    Q_EMIT locationChanged();
}

/*!
    Returns the X11 session for the user account.
*/
QString UserAccount::xsession() const
{
    Q_D(const UserAccount);
    return d->user->xSession();
}

/*!
    Sets the X11 session for the user account.

    \param session X11 session name.
*/
void UserAccount::setXSession(const QString &session)
{
    Q_D(UserAccount);

    if (xsession() == session)
        return;

    d->xsession = session;
    d->user->SetXSession(session);
    Q_EMIT xsessionChanged();
}

/*!
    Sets the password for the user account.

    \param password.
    \param hint passwordHint.
 */
void UserAccount::setPassword(const QString &password, const QString &hint)
{
    Q_D(UserAccount);
    d->user->SetPassword(password, hint);
}

/*!
    Sets the password hint for the user account.

    \param hint Password hint.
 */
void UserAccount::setPasswordHint(const QString &hint)
{
    Q_D(UserAccount);
    d->user->SetPasswordHint(hint);
}

/*!
    \internal
 */
void UserAccount::handleAccountChanged()
{
    // Catch changes outside of this API

    Q_D(UserAccount);

    if (d->accountType != accountType()) {
        d->accountType = accountType();
        Q_EMIT accountTypeChanged();
    }

    if (d->locked != isLocked()) {
        d->locked = isLocked();
        Q_EMIT lockedChanged();
    }

    if (d->automaticLogin != automaticLogin()) {
        d->automaticLogin = automaticLogin();
        Q_EMIT automaticLoginChanged();
    }

    if (d->passwordMode != passwordMode()) {
        d->passwordMode = passwordMode();
        Q_EMIT passwordModeChanged();
    }

    if (d->userName != userName()) {
        d->userName = userName();
        Q_EMIT userNameChanged();
        Q_EMIT displayNameChanged();
    }

    if (d->realName != realName()) {
        d->realName = realName();
        Q_EMIT realNameChanged();
        Q_EMIT displayNameChanged();
    }

    if (d->homeDirectory != homeDirectory()) {
        d->homeDirectory = homeDirectory();
        Q_EMIT homeDirectoryChanged();
    }

    if (d->shell != shell()) {
        d->shell = shell();
        Q_EMIT shellChanged();
    }

    if (d->iconFileName != iconFileName()) {
        d->iconFileName = iconFileName();
        Q_EMIT iconFileNameChanged();
    }

    if (d->email != email()) {
        d->email = email();
        Q_EMIT emailChanged();
    }

    if (d->language != language()) {
        d->language = language();
        Q_EMIT languageChanged();
    }

    if (d->location != location()) {
        d->location = location();
        Q_EMIT locationChanged();
    }

    if (d->xsession != xsession()) {
        d->xsession = xsession();
        Q_EMIT xsessionChanged();
    }
}
}

#include "moc_useraccount.cpp"
