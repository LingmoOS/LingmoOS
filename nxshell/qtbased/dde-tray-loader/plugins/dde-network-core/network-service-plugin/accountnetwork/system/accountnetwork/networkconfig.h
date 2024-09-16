// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

#include <QObject>
#include <QVariantMap>

namespace accountnetwork {
namespace systemservice {

class NetworkConfig : public QObject
{
    Q_OBJECT

public:
    NetworkConfig(QObject *parent = Q_NULLPTR);
    ~NetworkConfig();
    void removeNetwork(const QString &account);
    void removeNetwork(const QString &account, const QString &id, const QString &interfaceName);
    void saveNetwork(const QString &account, const QString &id, const QString &interfaceName);
    QMap<QString, QString> network(const QString &account) const;

private:
    void loadConfig();
    void saveConfig(const QMap<QString, QMap<QString, QString>> &accountNetwork);

private:
    QMap<QString, QMap<QString, QString>> m_accountNetwork;
};

}
}

#endif // NETWORKSERVICE_H
