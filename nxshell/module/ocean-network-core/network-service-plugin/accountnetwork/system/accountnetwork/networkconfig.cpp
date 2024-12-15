// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "networkconfig.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

using namespace accountnetwork::systemservice;

#define CONFIGFILE "/etc/network/accountnetwork"

NetworkConfig::NetworkConfig(QObject *parent)
    : QObject (parent)
{
    loadConfig();
}

void NetworkConfig::removeNetwork(const QString &account)
{
    if (account.isEmpty())
        return;

    if (!m_accountNetwork.contains(account))
        return;

    m_accountNetwork.remove(account);
    saveConfig(m_accountNetwork);
}

void NetworkConfig::removeNetwork(const QString &account, const QString &id, const QString &interfaceName)
{
    if (account.isEmpty() || id.isEmpty() || interfaceName.isEmpty())
        return;

    if (!m_accountNetwork.contains(account))
        return;

    if (!m_accountNetwork[account].contains(interfaceName))
        return;

    if (m_accountNetwork[account][interfaceName] != id)
        return;

    m_accountNetwork[account].remove(interfaceName);
    saveConfig(m_accountNetwork);
}

NetworkConfig::~NetworkConfig() = default;

void NetworkConfig::saveNetwork(const QString &account, const QString &id, const QString &interfaceName)
{
    if (account.isEmpty() || id.isEmpty() || interfaceName.isEmpty())
        return;

    m_accountNetwork[account][interfaceName] = id;
    saveConfig(m_accountNetwork);
}

QMap<QString, QString> NetworkConfig::network(const QString &account) const
{
    return m_accountNetwork.value(account);
}

void NetworkConfig::loadConfig()
{
    QFile file(CONFIGFILE);
    if (!file.exists())
        return;

    file.open(QFile::OpenModeFlag::ReadOnly);
    QJsonDocument json = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!json.isObject())
        return;

    m_accountNetwork.clear();
    QJsonObject topObject = json.object();
    for (auto it = topObject.begin(); it != topObject.end(); it++) {
        QJsonObject jsonObject = it.value().toObject();
        QMap<QString, QString> mapObject;
        for (auto itid = jsonObject.begin(); itid != jsonObject.end(); itid++) {
            mapObject[itid.key()] = itid.value().toString();
        }
        m_accountNetwork[it.key()] = mapObject;
    }
}

void NetworkConfig::saveConfig(const QMap<QString, QMap<QString, QString> > &accountNetwork)
{
    QJsonObject topObject;
    for (auto it = accountNetwork.begin(); it != accountNetwork.end(); it++) {
        QJsonObject jsonTypeId;
        QMap<QString, QString> idType = it.value();
        for (auto itid = idType.begin(); itid != idType.end(); itid++) {
            jsonTypeId.insert(itid.key(), itid.value());
        }
        topObject.insert(it.key(), jsonTypeId);
    }
    QJsonDocument json;
    json.setObject(topObject);

    QFile file(CONFIGFILE);
    file.open(QFile::OpenModeFlag::WriteOnly);
    file.write(json.toJson());
    file.close();
}
