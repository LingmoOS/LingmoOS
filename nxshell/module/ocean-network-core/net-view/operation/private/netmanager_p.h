// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETMANAGERPRIVATE_H
#define NETMANAGERPRIVATE_H

#include "netitem.h"
#include "netmanager.h"

#include <QMap>
#include <QObject>

#define WirelessDeviceIndex 0
#define WiredDeviceIndex 1
#define DeviceItemCount 2

namespace ocean {
namespace network {
class NetManagerThreadPrivate;
class NetManager;
class NetItem;
class NetControlItemPrivate;
class NetDeviceItem;
enum class NetConnectionStatus;
struct PasswordRequest;

// 业务处理，主线程
class NetManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit NetManagerPrivate(NetManager *manager);
    ~NetManagerPrivate() override;

    void setMonitorNetworkNotify(bool monitor);
    void setUseSecretAgent(bool enabled);
    void setNetwork8021XMode(NetManager::Network8021XMode mode);
    void setAutoScanInterval(int ms);
    void setAutoScanEnabled(bool enabled);
    void setEnabled(bool enabled);
    void setServerKey(const QString &serverKey);
    void init(NetType::NetManagerFlags flags);
    bool netCheckAvailable();

Q_SIGNALS:
    void request(NetManager::CmdType cmd, const QString &id, const QVariantMap &param);
    void vpnStatusChanged();

protected Q_SLOTS:
    //  执行操作
    void exec(NetManager::CmdType cmd, const QString &id, const QVariantMap &param);
    // 获取数据
    void onItemAoceand(const QString &parentID, NetItemPrivate *item);
    void onItemRemoved(const QString &id);
    void onDataChanged(int dataType, const QString &id, const QVariant &value);

    void sendRequest(NetManager::CmdType cmd, const QString &id, const QVariantMap &param = QVariantMap());

    void onRequestPassword(const QString &dev, const QString &id, const QVariantMap &param);
    void findPasswordRequestItem();
    void clearPasswordRequest(const QString &id);
    void retranslateUi();
    void onItemDestroyed(QObject *obj);

protected:
    void setDeviceEnabled(const QString &id, bool enabled);
    void setDeviceEnabled(NetControlItemPrivate *controlItem, bool enabled);
    void updateControl();
    void updateItemVisible(const QString &id, bool visible);
    void updateControlEnabled(int type);
    void updateAirplaneMode(bool enabled);
    void updatePrimaryConnectionType(NetManager::ConnectionType type);

    void addItem(NetItemPrivate *item, NetItemPrivate *parentItem = nullptr);
    void removeItem(NetItemPrivate *item);
    void removeAndDeleteItem(NetItemPrivate *item);

    inline NetItemPrivate *findItem(const QString &id) const { return m_dataMap.value(id, nullptr); }

private:
    NetControlItemPrivate *m_root;
    NetItemPrivate *m_deleteItem; // 关联无父对象item，用于删除
    bool m_isDeleting;
    bool m_airplaneMode;
    // NetManager::ConnectionType m_primaryConnectionType;
    bool m_monitorNetworkNotify;
    bool m_secretAgent;
    bool m_autoAddConnection;
    NetManagerThreadPrivate *m_managerThread;
    QMap<QString, NetItemPrivate *> m_dataMap;
    PasswordRequest *m_passwordRequestData;
    QString m_showInputId;

    int m_deviceCount[DeviceItemCount];

    NetManager *q_ptr;
    Q_DECLARE_PUBLIC(NetManager)

    friend class NetManager;
};

} // namespace network
} // namespace ocean

#endif //  NETMANAGERPRIVATE_H
