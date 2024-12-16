// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "User.h"

#include <QObject>
#include <QUrl>

struct UserPrivate
{
    bool logined{ false };
    bool noPasswdLogin{ false };
    quint64 uid{ 0 };
    quint64 gid{ 0 };
    QString userName;
    QString fullName;
    QString homeDir;
    QUrl icon;
    QLocale locale;
    AccountTypes identity;
    QString passwordHint;
    QString limitTime;
    AccountsUserPtr inter{ nullptr };

    void updateUserData();
};

void UserPrivate::updateUserData()
{
    noPasswdLogin = inter->noPasswdLogin();
    identity = inter->accountType();
    uid = inter->UID();
    gid = inter->GID();
    userName = inter->userName();
    fullName = inter->fullName();
    homeDir = inter->homeDir();
    icon = inter->iconFile();
    passwordHint = inter->passwordHint();
    locale = QLocale{ inter->locale() };
}

User::User(AccountsUserPtr ptr)
    : d(new UserPrivate{})
{
    d->inter = std::move(ptr);

    if (!d->inter) {
        qFatal() << "connect to AccountService Failed";
    }

    connect(d->inter.data(), &DAccountsUser::userDataChanged, [this] {
        d->updateUserData();
        emit userDataChanged();
    });

    d->updateUserData();
}

User::~User()
{
    delete d;
}

bool User::noPasswdLogin() const noexcept
{
    return d->noPasswdLogin;
}

QString User::identity() const noexcept
{
    return toString(d->identity);
}

quint64 User::UID() const noexcept
{
    return d->uid;
}

quint64 User::GID() const noexcept
{
    return d->gid;
}

const QString &User::userName() const noexcept
{
    return d->userName;
}

const QString &User::fullName() const noexcept
{
    return d->fullName;
}

const QString &User::homeDir() const noexcept
{
    return d->homeDir;
}

const QUrl &User::iconFile() const noexcept
{
    return d->icon;
}

const QString &User::passwordHint() const noexcept
{
    return d->passwordHint;
}

const QLocale &User::locale() const noexcept
{
    return d->locale;
}

bool User::logined() const noexcept
{
    return d->logined;
}

void User::setLogined(bool newState) const noexcept
{
    d->logined = newState;
}

void User::updateLimitTime(const QString &time) noexcept
{
    d->limitTime = time;
    emit limitTimeChanged(time);
}

QString User::toString(AccountTypes type) noexcept
{
    DACCOUNTS_USE_NAMESPACE
    switch (type) {
    case AccountTypes::Admin:
        return tr("Administrator");
    case AccountTypes::Default:
        return tr("Standard User");
    default:
        qDebug() << "ignore other types.";
    }

    return {};
}
