// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef USER_H
#define USER_H

#include <DAccountsManager>
#include <DAccountsTypes>
#include <DAccountsUser>

DACCOUNTS_USE_NAMESPACE

struct UserPrivate;

class User : public QObject
{
    Q_OBJECT
public:
    explicit User(AccountsUserPtr ptr);
    ~User() override;

    [[nodiscard]] bool noPasswdLogin() const noexcept;
    [[nodiscard]] QString identity() const noexcept;
    [[nodiscard]] quint64 UID() const noexcept;
    [[nodiscard]] quint64 GID() const noexcept;
    [[nodiscard]] const QString &userName() const noexcept;
    [[nodiscard]] const QString &fullName() const noexcept;
    [[nodiscard]] const QString &homeDir() const noexcept;
    [[nodiscard]] const QUrl &iconFile() const noexcept;
    [[nodiscard]] const QString &passwordHint() const noexcept;
    [[nodiscard]] bool logined() const noexcept;
    [[nodiscard]] const QLocale &locale() const noexcept;
    [[nodiscard]] static QString toString(AccountTypes type) noexcept;
    void setLogined(bool newState) const noexcept;
    void updateLimitTime(const QString &time) noexcept;

Q_SIGNALS:
    void userDataChanged();
    void limitTimeChanged(const QString &time);

private:
    UserPrivate *d{ nullptr };
};

#endif
