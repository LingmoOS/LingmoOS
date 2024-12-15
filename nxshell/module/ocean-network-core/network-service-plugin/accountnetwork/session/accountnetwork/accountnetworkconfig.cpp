// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "accountnetworkconfig.h"
#include "aes.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

using namespace accountnetwork::sessionservice;

static QString configfilename()
{
    return QString("%1/network/accountAuthen").arg(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation)[0]).path());
}

AccountNetworkConfig::AccountNetworkConfig(QObject *parent)
    : QObject(parent)
{
    loadConfig();
}

void AccountNetworkConfig::saveNetworkCertity(const QString &interfaceName, const QString &networkId, const QString &identity, const QString &password)
{
    if (networkId.isEmpty())
        return;

    // 同步最新的当前账户的网络，不是当前账户的网络，从列表中删除
    bool findNetwork = false;
    for (AuthenInfo &auth : m_authenInfo) {
        if (auth.interfaceName == interfaceName) {
            auth.id = networkId;
            auth.identity = identity;
            auth.password = network::service::PasswordHelper::aesEncryption(password, "uniontechnetwork", "ICBCCreditSuisse");
            findNetwork = true;
            break;
        }
    }
    if (!findNetwork) {
        AuthenInfo auth;
        auth.interfaceName = interfaceName;
        auth.id = networkId;
        auth.identity = identity;
        auth.password = network::service::PasswordHelper::aesEncryption(password, "uniontechnetwork", "ICBCCreditSuisse");
        m_authenInfo << auth;
    }

    QJsonArray array;
    for (const AuthenInfo &auth : m_authenInfo) {
        QJsonObject jsonObject;
        jsonObject.insert("id", auth.id);
        jsonObject.insert("interface", auth.interfaceName);
        jsonObject.insert("identity", auth.identity);
        jsonObject.insert("password", auth.password);
        array.append(jsonObject);
    }

    QJsonDocument writeJson;
    writeJson.setArray(array);

    QFile file(configfilename());
    file.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Truncate);
    file.write(writeJson.toJson());
    file.close();
}

QPair<QString, QString> AccountNetworkConfig::certityInfo(const QString &interfaceName, const QString &networkId)
{
    for (const AuthenInfo &auth : m_authenInfo) {
        if (auth.interfaceName == interfaceName && auth.id == networkId) {
            return qMakePair(auth.identity, auth.password);
        }
    }

    return {};
}

void AccountNetworkConfig::loadConfig()
{
    QFile file(configfilename());
    if (!file.exists())
        return;

    file.open(QFile::OpenModeFlag::ReadOnly);
    QJsonDocument json = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!json.isArray())
        return;

    m_authenInfo.clear();
    QJsonArray topArray = json.array();
    for (int i = 0; i < topArray.size(); i++) {
        QJsonValue value = topArray[i];
        QJsonObject jsonObject = value.toObject();
        AuthenInfo auth;
        auth.id = jsonObject.value("id").toString();
        auth.interfaceName = jsonObject.value("interface").toString();
        auth.identity = jsonObject.value("identity").toString();
        auth.password = jsonObject.value("password").toString();
        m_authenInfo << auth;
    }
}
