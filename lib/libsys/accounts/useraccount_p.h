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

#ifndef QTACCOUNTSSERVICE_USERACCOUNT_P_H
#define QTACCOUNTSSERVICE_USERACCOUNT_P_H

#include "useraccount.h"
#include "user_interface.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt AccountsService API.  It exists
// purely as an implementation detail.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

namespace QtAccountsService {

class UserAccountPrivate
{
    Q_DECLARE_PUBLIC(UserAccount)
    Q_DISABLE_COPY(UserAccountPrivate)
public:
    explicit UserAccountPrivate(UserAccount *q);

    void initialize(const QDBusConnection &connection, const QString &objectPath);
    void emitSignals();

    QDBusConnection bus;
    OrgFreedesktopAccountsUserInterface *user;

    UserAccount::AccountType accountType;
    bool locked;
    bool automaticLogin;
    UserAccount::PasswordMode passwordMode;
    QString userName;
    QString realName;
    QString homeDirectory;
    QString shell;
    QString iconFileName;
    QString email;
    QString language;
    QString location;
    QString xsession;

protected:
    UserAccount *q_ptr;
};
}

#endif // QTACCOUNTSSERVICE_USERACCOUNT_P_H
