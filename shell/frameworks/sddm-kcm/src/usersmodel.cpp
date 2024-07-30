/*
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>

    SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "usersmodel.h"

#include <KLocalizedString>
#include <KUser>

UsersModel::UsersModel(QObject *parent)
    : QAbstractListModel(parent)
{
    populate();
}

UsersModel::~UsersModel()
{
}

int UsersModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return mUserList.size();
}

QVariant UsersModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    if (row < 0 || row >= mUserList.count()) {
        return QVariant();
    }
    const KUser &user = mUserList[row];

    switch (role) {
    case Qt::DisplayRole:
        return user.loginName();
    case UidRole:
        return user.userId().nativeId();
    }

    return QVariant();
}

void UsersModel::add(const KUser &user)
{
    beginInsertRows(QModelIndex(), mUserList.count(), mUserList.count());

    mUserList.append(KUser(user));

    endInsertRows();
}

void UsersModel::populate()
{
    mUserList.clear();

    const QList<KUser> userList = KUser::allUsers();

    for (const KUser &user : userList) {
        K_UID uuid = user.userId().nativeId();

        // invalid user
        if (uuid == (uid_t)-1) {
            continue;
        }

        add(user);
        /*qDebug() << user.loginName() << ",uid" << uuid;
        qDebug() << " home:" << user.homeDir();
        qDebug() << " isSuperUser:" << user.isSuperUser() << ",isValid:" << user.isValid();
        qDebug() << " faceIconPath:" << user.faceIconPath();*/
    }
}

int UsersModel::indexOf(const QString &user)
{
    if (user.isEmpty()) {
        return 0;
    }
    // find user index
    for (int i = 0; i < mUserList.size(); ++i) {
        if (mUserList.at(i).loginName() == user) {
            return i;
        }
    }
    // user not found
    return 0;
}
