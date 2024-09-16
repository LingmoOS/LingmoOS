// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QObject>
#include <QVariantMap>

namespace accountnetwork {
namespace systemservice {

class NetworkConfig;

class AccountManager : public QObject
{
    Q_OBJECT

public:
    AccountManager(NetworkConfig *conf, QObject *parent = Q_NULLPTR);
    ~AccountManager();
    QString account() const;
    QStringList primaryAccount() const;
    bool accountExist(const QString &accountname) const;

signals:
    void accountChanged(const QString &accountname);
    void accountAdded(const QString &accountname, bool isIam);
    void accountRemoved(const QString &accountname);

private:
    void initAccount();
    QString parseAccount(const QString &accountInfo);
    QStringList iamUserList() const;

private slots:
    void onAccountChanged(const QString &username);
    void onUserAdded(const QString &path);
    void onUserDeleted(const QString &path);

private:
    QString m_account;
    NetworkConfig *m_networkConfig;
    QMap<QString, QString> m_userMap;
};

}
}

#endif // NETWORKSERVICE_H
