// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef ACCOUNTNETWORKCONFIG_H
#define ACCOUNTNETWORKCONFIG_H

#include <QObject>

namespace accountnetwork {
namespace sessionservice {

class AccountNetworkConfig : public QObject
{
    Q_OBJECT

public:
    explicit AccountNetworkConfig(QObject *parent = nullptr);
    void saveNetworkCertity(const QString &interfaceName, const QString &networkId, const QString &identity, const QString &password);
    QPair<QString, QString> certityInfo(const QString &interfaceName, const QString &networkId);

private:
    struct AuthenInfo {
        QString id;
        QString interfaceName;
        QString identity;
        QString password;
        AuthenInfo &operator=(const AuthenInfo &authen) {
            id = authen.id;
            interfaceName = authen.interfaceName;
            identity = authen.identity;
            password = authen.password;
            return *this;
        }

        bool operator==(const AuthenInfo &authen) const {
            return (id == authen.id)
                    && (interfaceName == authen.interfaceName)
                    && (identity == authen.identity)
                    && (password == authen.password);
        }
    };
    void loadConfig();

private:
    QList<AuthenInfo> m_authenInfo;
};

}
}

#endif // ACCOUNTNETWORKCONFIG_H
