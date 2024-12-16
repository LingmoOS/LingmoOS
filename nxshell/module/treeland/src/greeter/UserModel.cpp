/***************************************************************************
 * Copyright (c) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ***************************************************************************/

#include "UserModel.h"

#include "Configuration.h"

#include <DDBusInterface>

#include <QFile>
#include <QGuiApplication>
#include <QList>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include <QTranslator>

#include <memory>
#include <pwd.h>

using namespace DDM;
DACCOUNTS_USE_NAMESPACE

struct UserModelPrivate
{
    bool containsAllUsers{ true };
    int lastIndex{ 0 };
    QString currentUserName;
    DAccountsManager manager;
    QTranslator *lastTrans{ nullptr };
    QList<UserPtr> users;
};

UserModel::UserModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new UserModelPrivate())
{
    connect(&d->manager, &DAccountsManager::UserAoceand, this, &UserModel::onUserAoceand);
    connect(&d->manager, &DAccountsManager::UserDeleted, this, &UserModel::onUserDeleted);

    connect(this, &UserModel::currentUserNameChanged, [this] {
        auto user = getUser(d->currentUserName);
        if (!user) {
            qWarning() << "couldn't find user:" << d->currentUserName;
            return;
        }

        auto locale = user->locale();
        qInfo() << "current locale:" << locale.language();
        auto *newTrans = new QTranslator{ this };
        auto dirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        for (const auto &dir : dirs) {
            if (newTrans->load(locale, "greeter", ".", dir + "/ddm/translations", ".qm")) {
                if (d->lastTrans) {
                    QGuiApplication::removeTranslator(d->lastTrans);
                }
                d->lastTrans->deleteLater();
                d->lastTrans = newTrans;
                QGuiApplication::installTranslator(d->lastTrans);
                emit updateTranslations(locale);
                emit dataChanged(createIndex(0, 0), createIndex(rowCount(), 0));
                return;
            }
        }

        newTrans->deleteLater();
        qWarning() << "failed to load new translator under" << dirs.last();
    });

    auto userList = d->manager.userList();
    if (!userList) {
        qFatal() << userList.error();
    }

    for (auto uid : userList.value()) {
        auto user = d->manager.findUserById(uid);
        if (!user) {
            qWarning() << user.error();
            continue;
        }

        auto val = std::move(user).value();
        d->users.emplace_back(std::make_unique<User>(std::move(val)));
    }

    // sort users by username
    std::sort(d->users.begin(), d->users.end(), [](const UserPtr &u1, const UserPtr &u2) {
        return u1->userName() < u2->userName();
    });

    // find out index of the last user
    auto lastUserName = stateConfig.Last.User.get();
    const auto &facesDir = mainConfig.Theme.FacesDir.get();

    for (const auto &user : d->users) {
        if (user->userName() == lastUserName) {
            d->lastIndex = d->users.indexOf(user);
            break;
        }
    }
}

UserModel::~UserModel()
{
    delete d;
}

QHash<int, QByteArray> UserModel::roleNames() const
{
    // set role names
    QHash<int, QByteArray> names;
    names[NameRole] = QByteArrayLiteral("name");
    names[RealNameRole] = QByteArrayLiteral("realName");
    names[HomeDirRole] = QByteArrayLiteral("homeDir");
    names[IconRole] = QByteArrayLiteral("icon");
    names[NoPasswordRole] = QByteArrayLiteral("noPassword");
    names[LoginedRole] = QByteArray("logined");
    names[IdentityRole] = QByteArray("identity");
    names[PasswordHintRole] = QByteArray("passwordHint");
    names[LocaleRole] = QByteArray("LocaleRole");
    return names;
}

int UserModel::lastIndex() const
{
    return d->lastIndex;
}

QString UserModel::lastUser()
{
    return stateConfig.Last.User.get();
}

int UserModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(d->users.length());
}

void UserModel::updateUserLoginState(const QString &username, bool logined)
{
    auto user = std::find_if(d->users.begin(), d->users.end(), [&username](const UserPtr &user) {
        return user->userName() == username;
    });

    if (user != d->users.end()) {
        (*user)->setLogined(logined);
        auto pos = std::distance(d->users.end(), user);
        emit dataChanged(index(0, pos - 1), index(0, pos));
    }

    emit layoutChanged();
}

QVariant UserModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > d->users.count()) {
        return {};
    }

    // get user
    UserPtr user = d->users[index.row()];

    // return correct value
    switch (role) {
    case NameRole:
        return user->userName();
    case RealNameRole:
        return user->fullName();
    case HomeDirRole:
        return user->homeDir();
    case IconRole:
        return user->iconFile();
    case NoPasswordRole:
        return user->noPasswdLogin();
    case LoginedRole:
        return user->logined();
    case IdentityRole:
        return user->identity();
    case PasswordHintRole:
        return user->passwordHint();
    case LocaleRole:
        return user->locale();
    default:
        return {};
    }

    Q_UNREACHABLE();
}

int UserModel::disableAvatarsThreshold()
{
    return mainConfig.Theme.DisableAvatarsThreshold.get();
}

bool UserModel::containsAllUsers() const
{
    return d->containsAllUsers;
}

QVariant UserModel::get(const QString &username) const
{
    QVariantMap map;
    for (const auto &user : d->users) {
        if (user->userName() == username or user->fullName() == username) {
            map["name"] = user->userName();
            map["icon"] = user->iconFile();
            map["realName"] = user->fullName();
            map["homeDir"] = user->homeDir();
            map["noPassword"] = user->noPasswdLogin();
            map["logined"] = user->logined();
            map["identity"] = user->identity();
            map["passwordHint"] = user->passwordHint();
            map["locale"] = user->locale();
            break;
        }
    }

    return map;
}

QVariant UserModel::get(int index) const
{
    QVariantMap map;
    if (index < 0 or index > d->users.count()) {
        return {};
    }

    auto user = d->users.at(index);
    map["name"] = user->userName();
    map["icon"] = user->iconFile();
    map["realName"] = user->fullName();
    map["homeDir"] = user->homeDir();
    map["noPassword"] = user->noPasswdLogin();
    map["logined"] = user->logined();
    map["identity"] = user->identity();
    map["passwordHint"] = user->passwordHint();
    map["locale"] = user->locale();

    return map;
}

UserPtr UserModel::getUser(const QString &username) const noexcept
{
    for (const auto &user : d->users) {
        if (user->userName() == username) {
            return user;
        }
    }
    return nullptr;
}

QString UserModel::currentUserName() const noexcept
{
    return d->currentUserName;
}

void UserModel::updateUserLimits(const QString &userName, const QString &time) const noexcept
{
    auto user = std::find_if(d->users.begin(), d->users.end(), [&userName](const UserPtr &user) {
        return user->userName() == userName;
    });

    if (user != d->users.end()) {
        (*user)->updateLimitTime(time);
    }
}

void UserModel::setCurrentUserName(const QString &userName) noexcept
{
    d->currentUserName = userName;
    emit currentUserNameChanged();
}

void UserModel::onUserAoceand(quint64 uid)
{
    auto newUser = d->manager.findUserById(uid);
    if (!newUser) {
        qWarning() << "user " << uid << " has been aoceand but couldn't find it.";
        return;
    }

    beginResetModel();
    d->users.emplace_back(std::make_unique<User>(std::move(newUser).value()));
    std::sort(d->users.begin(), d->users.end(), [](const UserPtr &u1, const UserPtr &u2) {
        return u1->userName() < u2->userName();
    });
    endResetModel();

    emit countChanged();
}

void UserModel::onUserDeleted(quint64 uid)
{
    beginResetModel();
    d->users.removeIf([uid](const UserPtr &user) {
        return user->UID() == uid;
    });

    std::sort(d->users.begin(), d->users.end(), [](const UserPtr &u1, const UserPtr &u2) {
        return u1->userName() < u2->userName();
    });
    endResetModel();

    emit countChanged();
}
