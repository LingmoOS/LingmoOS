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

#include <QtGui/QPixmap>

#include "usersmodel.h"
#include "usersmodel_p.h"

namespace QtAccountsService {

/*
 * UsersModelPrivate
 */

UsersModelPrivate::UsersModelPrivate(UsersModel *q)
    : manager(new AccountsManager())
    , q_ptr(q)
{
}

UsersModelPrivate::~UsersModelPrivate()
{
    delete manager;
}

void UsersModelPrivate::_q_userAdded(UserAccount *account)
{
    Q_Q(UsersModel);

    q->connect(account, &UserAccount::accountChanged, q, [account, q, this]() {
        auto index = q->index(list.indexOf(account));
        if (index.isValid())
            Q_EMIT q->dataChanged(index, index);
    });

    q->beginInsertRows(QModelIndex(), list.size(), list.size());
    list.append(account);
    q->endInsertRows();
}

void UsersModelPrivate::_q_userDeleted(qlonglong uid)
{
    Q_Q(UsersModel);

    for (int i = 0; i < list.size(); i++) {
        UserAccount *curAccount = list.at(i);

        if (curAccount->userId() == uid) {
            q->beginRemoveRows(QModelIndex(), i, i);
            list.removeOne(curAccount);
            q->endRemoveRows();
            break;
        }
    }
}

/*
 * UsersModel
 */

UsersModel::UsersModel(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new UsersModelPrivate(this))
{
    Q_D(UsersModel);

    connect(d->manager, SIGNAL(userAdded(UserAccount *)), // clazy:exclude=old-style-connect
            this, SLOT(_q_userAdded(UserAccount *)));
    connect(d->manager, SIGNAL(userDeleted(qlonglong)), // clazy:exclude=old-style-connect
            this, SLOT(_q_userDeleted(qlonglong)));

    connect(d->manager, &AccountsManager::listCachedUsersFinished,
            this, [d](const UserAccountList &list) {
                for (auto account : list)
                    d->_q_userAdded(account);
            });
    d->manager->listCachedUsers();
}

QHash<int, QByteArray> UsersModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[UserAccountRole] = "userAccount";
    roles[UserIdRole] = "userId";
    roles[AccountTypeRole] = "accountType";
    roles[LockedRole] = "locked";
    roles[AutomaticLoginRole] = "automaticLogin";
    roles[LoginFrequencyRole] = "loginFrequency";
    roles[LoginTimeRole] = "loginTime";
    roles[PasswordModeRole] = "passwordMode";
    roles[PasswordHintRole] = "passwordHint";
    roles[LocalAccount] = "localAccount";
    roles[SystemAccount] = "systemAccount";
    roles[UserNameRole] = "userName";
    roles[RealNameRole] = "realName";
    roles[DisplayNameRole] = "displayNameRole";
    roles[HomeDirectoryRole] = "homeDirectory";
    roles[ShellRole] = "shell";
    roles[IconFileNameRole] = "iconFileName";
    roles[LanguageRole] = "language";
    roles[EmailRole] = "email";
    roles[LocationRole] = "location";
    roles[XSessionRole] = "xsession";
    return roles;
}

int UsersModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const UsersModel);

    if (parent == QModelIndex())
        return d->list.size();

    return 0;
}

QVariant UsersModel::data(const QModelIndex &index, int role) const
{
    UserAccount *user = userAccount(index);
    if (!user)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case DisplayNameRole:
        return user->displayName();
    case Qt::DecorationRole:
        return QPixmap(user->iconFileName());
    case UserAccountRole:
        return QVariant::fromValue(user);
    case UserIdRole:
        return user->userId();
    case AccountTypeRole:
        return user->accountType();
    case LockedRole:
        return user->isLocked();
    case AutomaticLoginRole:
        return user->automaticLogin();
    case LoginFrequencyRole:
        return user->loginFrequency();
    case LoginTimeRole:
        return user->loginTime();
    case PasswordModeRole:
        return user->passwordMode();
    case PasswordHintRole:
        return user->passwordHint();
    case LocalAccount:
        return user->isLocalAccount();
    case SystemAccount:
        return user->isSystemAccount();
    case UserNameRole:
        return user->userName();
    case RealNameRole:
        return user->realName();
    case HomeDirectoryRole:
        return user->homeDirectory();
    case ShellRole:
        return user->shell();
    case IconFileNameRole:
        return user->iconFileName();
    case EmailRole:
        return user->email();
    case LanguageRole:
        return user->language();
    case LocationRole:
        return user->location();
    case XSessionRole:
        return user->xsession();
    default:
        break;
    }

    return QVariant();
}

bool UsersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    UserAccount *user = userAccount(index);
    if (!user)
        return false;

    switch (role) {
    case UsersModel::UserNameRole:
        user->setUserName(value.toString());
        break;
    case UsersModel::RealNameRole:
        user->setRealName(value.toString());
        break;
    case UsersModel::IconFileNameRole:
        user->setIconFileName(value.toString());
        break;
    case UsersModel::AccountTypeRole:
        user->setAccountType((UserAccount::AccountType)value.toInt());
        break;
    case UsersModel::LanguageRole:
        user->setLanguage(value.toString());
        break;
    default:
        return false;
    }

    return true;
}

UserAccount *UsersModel::userAccount(const QModelIndex &index) const
{
    Q_D(const UsersModel);

    if (!index.isValid())
        return nullptr;
    if (index.row() >= d->list.size())
        return nullptr;

    return d->list[index.row()];
}
}

#include "moc_usersmodel.cpp"
