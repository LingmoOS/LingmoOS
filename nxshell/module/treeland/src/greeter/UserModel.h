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

#ifndef SDDM_USERMODEL_H
#define SDDM_USERMODEL_H

#include "User.h"

#include <QAbstractListModel>
#include <QHash>
#include <QQmlEngine>

struct UserModelPrivate;
using UserPtr = std::shared_ptr<User>;

class UserModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentUserName READ currentUserName WRITE setCurrentUserName NOTIFY
                    currentUserNameChanged)
    Q_PROPERTY(int lastIndex READ lastIndex CONSTANT)
    Q_PROPERTY(QString lastUser READ lastUser CONSTANT)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(int disableAvatarsThreshold READ disableAvatarsThreshold CONSTANT)
    Q_PROPERTY(bool containsAllUsers READ containsAllUsers CONSTANT)
    QML_ELEMENT
public:
    enum UserRoles {
        NameRole = Qt::UserRole + 1,
        RealNameRole,
        HomeDirRole,
        IconRole,
        NoPasswordRole,
        LoginedRole,
        IdentityRole,
        PasswordHintRole,
        LocaleRole
    };
    Q_ENUM(UserRoles)

    UserModel(const UserModel &) = delete;
    UserModel &operator=(const UserModel &) = delete;
    explicit UserModel(QObject *parent = nullptr);
    ~UserModel() override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] int lastIndex() const;
    [[nodiscard]] static QString lastUser();
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] Q_INVOKABLE QVariant data(const QModelIndex &index,
                                            int role = Qt::DisplayRole) const override;
    [[nodiscard]] Q_INVOKABLE QVariant get(const QString &username) const;
    [[nodiscard]] Q_INVOKABLE QVariant get(int index) const;
    [[nodiscard]] UserPtr getUser(const QString &username) const noexcept;
    [[nodiscard]] QString currentUserName() const noexcept;
    void updateUserLimits(const QString &userName, const QString &time) const noexcept;
    void setCurrentUserName(const QString &userName) noexcept;
    void updateUserLoginState(const QString &username, bool logined);
    [[nodiscard]] static int disableAvatarsThreshold();
    [[nodiscard]] bool containsAllUsers() const;

Q_SIGNALS:
    void currentUserNameChanged();
    void updateTranslations(const QLocale &locale);
    void countChanged();

private Q_SLOTS:
    void onUserAoceand(quint64 uid);
    void onUserDeleted(quint64 uid);

private:
    UserModelPrivate *d{ nullptr };
};

#endif // USERMODEL_H
