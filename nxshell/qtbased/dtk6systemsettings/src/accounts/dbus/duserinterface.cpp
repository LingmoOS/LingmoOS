// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "duserinterface.h"

#include "daccountstypes_p.h"

#include <qdebug.h>

DACCOUNTS_BEGIN_NAMESPACE

DUserInterface::DUserInterface(const QString &path, QObject *parent)
    : QObject(parent)
{
#if defined(USE_FAKE_INTERFACE)
    const QString &Service = QStringLiteral("com.deepin.daemon.FakeAccounts");
    const QString &Interface = QStringLiteral("com.deepin.daemon.FakeAccounts.User");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.Accounts");
    const QString &Interface = QStringLiteral("org.freedesktop.Accounts.User");
    QDBusConnection Connection = QDBusConnection::systemBus();
    Connection.connect(Service, path, Interface, "Changed", this, SIGNAL(DataChanged()));
#endif
    LoginHistory_p::registerMetaType();

    m_inter = new DDBusInterface(Service, path, Interface, Connection, this);
};

bool DUserInterface::automaticLogin() const
{
    return qdbus_cast<bool>(m_inter->property("AutomaticLogin"));
}

bool DUserInterface::localAccount() const
{
    return qdbus_cast<bool>(m_inter->property("LocalAccount"));
}

bool DUserInterface::locked() const
{
    return qdbus_cast<bool>(m_inter->property("Locked"));
}

bool DUserInterface::systemAccount() const
{
    return qdbus_cast<bool>(m_inter->property("SystemAccount"));
}

qint32 DUserInterface::accountType() const
{
    return qdbus_cast<qint32>(m_inter->property("AccountType"));
}

qint32 DUserInterface::passwordMode() const
{
    return qdbus_cast<qint32>(m_inter->property("PasswordMode"));
}

qint64 DUserInterface::loginTime() const
{
    return qdbus_cast<qint64>(m_inter->property("LoginTime"));
}

QString DUserInterface::email() const
{
    return qdbus_cast<QString>(m_inter->property("Email"));
}

QString DUserInterface::homeDirectory() const
{
    return qdbus_cast<QString>(m_inter->property("HomeDirectory"));
}

QString DUserInterface::iconFile() const
{
    return qdbus_cast<QString>(m_inter->property("IconFile"));
}

QString DUserInterface::language() const
{
    return qdbus_cast<QString>(m_inter->property("Language"));
}

QString DUserInterface::location() const
{
    return qdbus_cast<QString>(m_inter->property("Location"));
}

QString DUserInterface::passwordHint() const
{
    return qdbus_cast<QString>(m_inter->property("PasswordHint"));
}

QString DUserInterface::realName() const
{
    return qdbus_cast<QString>(m_inter->property("RealName"));
}

QString DUserInterface::shell() const
{
    return qdbus_cast<QString>(m_inter->property("Shell"));
}

QString DUserInterface::userName() const
{
    return qdbus_cast<QString>(m_inter->property("UserName"));
}

QString DUserInterface::xSession() const
{
    return qdbus_cast<QString>(m_inter->property("XSession"));
}

quint64 DUserInterface::loginFrequency() const
{
    return qdbus_cast<quint64>(m_inter->property("LoginFrequency"));
}

quint64 DUserInterface::UID() const
{
    return qdbus_cast<quint64>(m_inter->property("Uid"));
}

QDBusPendingReply<qint64, qint64, qint64, qint64, qint64, qint64>
DUserInterface::getPasswordExpirationPolicy()
{
    return m_inter->asyncCall("GetPasswordExpirationPolicy");
}

QDBusPendingReply<void> DUserInterface::setAccountType(qint32 accountType)
{
    QVariantList args{ QVariant::fromValue(accountType) };
    return m_inter->asyncCallWithArgumentList("SetAccountType", args);
}

QDBusPendingReply<void> DUserInterface::setAutomaticLogin(bool enabled)
{
    QVariantList args{ QVariant::fromValue(enabled) };
    return m_inter->asyncCallWithArgumentList("SetAutomaticLogin", args);
}

QDBusPendingReply<void> DUserInterface::setEmail(const QString &email)
{
    QVariantList args{ QVariant::fromValue(email) };
    return m_inter->asyncCallWithArgumentList("SetEmail", args);
}

QDBusPendingReply<void> DUserInterface::setHomeDirectory(const QString &homedir)
{
    QVariantList args{ QVariant::fromValue(homedir) };
    return m_inter->asyncCallWithArgumentList("SetHomeDirectory", args);
}

QDBusPendingReply<void> DUserInterface::setIconFile(const QString &filename)
{
    QVariantList args{ QVariant::fromValue(filename) };
    return m_inter->asyncCallWithArgumentList("SetIconFile", args);
}

QDBusPendingReply<void> DUserInterface::setLanguage(const QString &language)
{
    QVariantList args{ QVariant::fromValue(language) };
    return m_inter->asyncCallWithArgumentList("SetLanguage", args);
}

QDBusPendingReply<void> DUserInterface::setLocation(const QString &location)
{
    QVariantList args{ QVariant::fromValue(location) };
    return m_inter->asyncCallWithArgumentList("SetLocation", args);
}

QDBusPendingReply<void> DUserInterface::setLocked(bool locked)
{
    QVariantList args{ QVariant::fromValue(locked) };
    return m_inter->asyncCallWithArgumentList("SetLocked", args);
}

QDBusPendingReply<void> DUserInterface::setPassword(const QString &password, const QString &hint)
{
    QVariantList args{ QVariant::fromValue(password), QVariant::fromValue(hint) };
    return m_inter->asyncCallWithArgumentList("SetPassword", args);
}

QDBusPendingReply<void> DUserInterface::setPasswordHint(const QString &hint)
{
    QVariantList args{ QVariant::fromValue(hint) };
    return m_inter->asyncCallWithArgumentList("SetPasswordHint", args);
}

QDBusPendingReply<void> DUserInterface::setPasswordMode(qint32 mode)
{
    QVariantList args{ QVariant::fromValue(mode) };
    return m_inter->asyncCallWithArgumentList("SetPasswordMode", args);
}

QDBusPendingReply<void> DUserInterface::setRealName(const QString &name)
{
    QVariantList args{ QVariant::fromValue(name) };
    return m_inter->asyncCallWithArgumentList("SetRealName", args);
}

QDBusPendingReply<void> DUserInterface::setShell(const QString &shell)
{
    QVariantList args{ QVariant::fromValue(shell) };
    return m_inter->asyncCallWithArgumentList("SetShell", args);
}

QDBusPendingReply<void> DUserInterface::setUserName(const QString &name)
{
    QVariantList args{ QVariant::fromValue(name) };
    return m_inter->asyncCallWithArgumentList("SetUserName", args);
}

QDBusPendingReply<void> DUserInterface::setXSession(const QString &x_session)
{
    QVariantList args{ QVariant::fromValue(x_session) };
    return m_inter->asyncCallWithArgumentList("SetXSession", args);
}

DACCOUNTS_END_NAMESPACE
