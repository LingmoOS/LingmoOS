/*
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>

    SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef USERSMODEL_H
#define USERSMODEL_H

#include <QAbstractListModel>

class KUser;

class UsersModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        UserNameRole = Qt::UserRole + 1,
        RealNameRole,
        HomeDirRole,
        IconRole,
        UidRole,
    };
    Q_ENUM(Roles)

    explicit UsersModel(QObject *parent = nullptr);
    ~UsersModel() Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    void populate();
    int indexOf(const QString &user);

private:
    void add(const KUser &user);

    QList<KUser> mUserList;
};
#endif // USERSMODEL_H
