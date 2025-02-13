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

#ifndef QTACCOUNTSSERVICE_USERSMODEL_H
#define QTACCOUNTSSERVICE_USERSMODEL_H

#include <QtCore/QAbstractListModel>

namespace QtAccountsService {

class UserAccount;
class UsersModelPrivate;

class UsersModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(UsersModel)
public:
    enum Roles {
        UserAccountRole = Qt::UserRole + 1,
        UserIdRole,
        AccountTypeRole,
        LockedRole,
        AutomaticLoginRole,
        LoginFrequencyRole,
        LoginTimeRole,
        PasswordModeRole,
        PasswordHintRole,
        LocalAccount,
        SystemAccount,
        UserNameRole,
        RealNameRole,
        DisplayNameRole,
        HomeDirectoryRole,
        ShellRole,
        IconFileNameRole,
        EmailRole,
        LanguageRole,
        LocationRole,
        XSessionRole
    };
    Q_ENUM(Roles)

    explicit UsersModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const;

    int rowCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    UserAccount *userAccount(const QModelIndex &index) const;

private:
    UsersModelPrivate *const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_userAdded(UserAccount *account))
    Q_PRIVATE_SLOT(d_func(), void _q_userDeleted(qlonglong uid))
};
}

#endif // QTACCOUNTSSERVICE_USERSMODEL_H
