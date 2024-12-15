// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHUTILS_H
#define DBLUETOOTHUTILS_H

#include "dbluetoothdevice.h"
#include "dtkbluetooth_global.h"
#include "dbluetoothdbustypes.h"
#include "dbluetoothobextransfer.h"
#include "dbluetoothagent.h"
#include <QDBusObjectPath>
#include <QBluetoothUuid>
#include <QDBusArgument>

DBLUETOOTH_BEGIN_NAMESPACE

static constexpr auto &BlueZService = "org.bluez";
static constexpr auto &BlueZObexService = "org.bluez.obex";
static constexpr auto &FakeBlueZService = "org.FakeBlueZ";
static constexpr auto &FakeBlueZObexService = "org.FakeBluez.obex";
static constexpr auto &BlueZAdapterInterface = "org.bluez.Adapter1";
static constexpr auto &BlueZDeviceInterface = "org.bluez.Device1";
static constexpr auto &BlueZObexSessionInterface = "org.bluez.obex.Session1";
static constexpr auto &BlueZObexTransferInterface = "org.bluez.obex.Transfer1";
static constexpr auto &BlueZObexObjectPushInterface = "org.bluez.obex.ObjectPush1";
static constexpr auto &BlueZAgentManagerInterface = "org.bluez.AgentManager1";
static constexpr auto &BlueZObexAgentManagerInterface = "org.bluez.obex.AgentManager1";
static constexpr auto &BlueZObexClientInterface = "org.bluez.obex.Client1";

inline QList<QDBusObjectPath>
getSpecificObject(const ObjectMap &objects, const QStringList &requiredInterfaces, const MapVariantMap &values = {})
{
    QList<QDBusObjectPath> ret;
    for (auto it = objects.cbegin(); it != objects.cend(); ++it) {
        const auto &objInterfaces = it.value();
        for (const auto &requiredInterface : requiredInterfaces) {
            const auto &objInterface = objInterfaces.find(requiredInterface);
            if (objInterface != objInterfaces.cend()) {
                const auto &interfaceProps = values.find(requiredInterface);
                if (interfaceProps == values.cend()) {
                    ret.append(it.key());
                } else {
                    const auto &requiredPrpos = interfaceProps.value();
                    const auto &existsProps = objInterface.value();
                    bool found{false};
                    for (auto requiredProp = requiredPrpos.cbegin(); requiredProp != requiredPrpos.cend(); ++requiredProp) {
                        const auto &existsProp = existsProps.find(requiredProp.key());
                        if (existsProp == existsProps.cend())
                            break;
                        const auto &v1 = requiredProp.value(), v2 = existsProp.value();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                        if (v1.typeId() == v2.typeId() and v1 == v2) {
#else
                        if (v1.type() == v2.type() and v1 == v2) {
#endif
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        ret.append(it.key());
                }
            }
        }
    }
    return ret;
}

inline quint64 dBusPathToAdapterId(const QString &path)
{
    return path.split("/").last().mid(3).toUInt();  // Adapter UBI "/org/bluez/hci0"
}

inline qint64 dBusPathToAdapterId(const QDBusObjectPath &path)
{
    return dBusPathToAdapterId(path.path());
}

inline QString adapterIdToDBusPath(quint64 id)
{
    return QString("/org/bluez/hci") + QString::number(id);
}

inline QString dBusPathToDeviceAddr(const QDBusObjectPath &path)
{
    return path.path().split("/").last().mid(4).replace("_", ":");  // device UBI: "/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX"
}

inline QString deviceAddrToDBusPath(const QString &adapter, QString device)
{
    const auto &rawDeviceStr = "/dev_" + device.replace(":", "_");
    return adapter + rawDeviceStr;
}

inline QString transferstatusToString(DObexTransfer::TransferStatus transferstatus)
{
    switch (transferstatus) {
        case DObexTransfer::Queued:
            return "queued";
        case DObexTransfer::Active:
            return "active";
        case DObexTransfer::Suspended:
            return "suspended";
        case DObexTransfer::Complete:
            return "complete";
        case DObexTransfer::Error:
            return "error";
        default:
            return QString{};
    }
}

inline DObexTransfer::TransferStatus stringToTransferstatus(const QString &statusStr)
{
    if (statusStr == "queued")
        return DObexTransfer::Queued;
    else if (statusStr == "active")
        return DObexTransfer::Active;
    else if (statusStr == "suspended")
        return DObexTransfer::Suspended;
    else if (statusStr == "complete")
        return DObexTransfer::Complete;
    else if (statusStr == "error")
        return DObexTransfer::Error;
    else
        return DObexTransfer::Unknown;
}

inline ObexSessionInfo dBusPathToSessionInfo(const QDBusObjectPath &path)
{
    // session UBI: "/org/bluez/obex/client{server}/session0"
    const auto &list = path.path().split("/");
    quint64 id = list.last().mid(7).toUInt();
    ObexSessionType info{ObexSessionType::Client};
    if (list[list.size() - 2] == "server")
        info = ObexSessionType::Server;
    return ObexSessionInfo{info, id};
}

inline QString sessionInfoToDBusPath(const ObexSessionInfo &info)
{
    QString dest{"client"};
    if (info.sessionInfo == ObexSessionType::Server)
        dest = QLatin1String("server");
    return "/org/bluez/obex/" % dest % "/session" % QString::number(info.sessionId);
}

inline QString agentCapToString(const DAgent::Capability cap)
{
    switch (cap) {
        case DAgent::DisplayOnly:
            return "DisplayOnly";
        case DAgent::DisplayYesNo:
            return "DisplayYesNo";
        case DAgent::KeyboardOnly:
            return "KeyboardOnly";
        case DAgent::NoInputNoOutput:
            return "NoInputNoOutput";
        default:
            return QString{};
    }
}

inline QString addressTypeToString(DDevice::AddressType type)
{
    switch (type) {
        case DDevice::Public:
            return "public";
        case DDevice::Random:
            return "random";
        default:
            return QString{};
    }
}

inline DDevice::AddressType stringToAddressType(const QString &type)
{
    if (type == "public")
        return DDevice::Public;
    else if (type == "random")
        return DDevice::Random;
    else
        return DDevice::Unknown;
}

inline quint64 dBusPathToTransferId(const QString &transfer)
{
    // obex transfer UBI: "/org/bluez/obex/client{server}/session0/transfer0"
    return transfer.split("/").last().mid(8).toUInt();
}

inline QString transferIdToDBusPath(const QString &sessionPath, quint64 transferId)
{
    return sessionPath % "/transfer" % QString::number(transferId);
}

DBLUETOOTH_END_NAMESPACE

#endif
