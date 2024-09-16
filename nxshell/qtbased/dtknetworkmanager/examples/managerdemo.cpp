// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnetworkmanager.h"
#include "dactiveconnection.h"
#include <QDebug>
#include <QCoreApplication>

DNETWORKMANAGER_USE_NAMESPACE

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    DNetworkManager manager;
    auto success =
        QObject::connect(&manager, &DNetworkManager::activeConnectionsChanged, &app, [](const QList<quint64> &connIds) {
            qDebug() << "activeConnections changed:" << connIds;
        });

    if (manager.networkingEnabled() and manager.wirelessEnabled() and manager.wirelessHardwareEnabled())
        qDebug() << "wireless normal";
    auto ret = manager.checkConnectivity();
    if (!ret)
        qDebug() << ret.error();
    if (ret.value() == Dtk::NetworkManager::NMConnectivityState::Full)
        qDebug() << "full connection";
    else
        qDebug() << "error";
    auto per = manager.permissions();
    if (!per)
        qDebug() << per.error();
    auto perMap = per.value();
    for (const auto &it : perMap)
        qDebug() << it;
    auto activeConnList = manager.getActiveConnectionsIdList();
    for (auto i : activeConnList) {
        qDebug() << i;
        auto activeConn = manager.getActiveConnectionObject(i);
        if (!activeConn)
            qDebug() << activeConn.error();
        const auto &conn = activeConn.value();
        auto type = conn->connectionType();
        qDebug() << DNMSetting::typeToString(type);
        if (type == DNMSetting::SettingType::Wireless) {
            auto deviceId = conn->devices()[0];
            auto connId = conn->connection();
            auto ret1 = manager.deactivateConnection(i);
            if (!ret1)
                qDebug() << ret1.error();
            sleep(3);
            auto ret2 = manager.activateConnection(connId, deviceId);
            if (!ret2)
                qDebug() << ret2.error();
        }
    }
    return app.exec();
}
