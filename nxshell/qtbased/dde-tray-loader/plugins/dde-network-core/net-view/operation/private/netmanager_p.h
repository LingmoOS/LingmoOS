// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETMANAGERPRIVATE_H
#define NETMANAGERPRIVATE_H

#include "netmanager.h"

#include <QMap>
#include <QObject>

#define WirelessDeviceIndex 0
#define WiredDeviceIndex 1
#define DeviceItemCount 2

namespace dde {
namespace network {
class NetManagerThreadPrivate;
class NetManager;
class NetItem;
class NetControlItem;
class NetDeviceItem;
enum class NetConnectionStatus;
struct PasswordRequest;

// 业务处理，主线程
class NetManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit NetManagerPrivate(NetManager *manager, bool tipsLinkEnabled);
    ~NetManagerPrivate() override;

    void setServiceLoadForNM(bool isNM);
    void setMonitorNetworkNotify(bool monitor);
    void setUseSecretAgent(bool enabled);
    void setNetwork8021XMode(NetManager::Network8021XMode mode);
    void setAutoScanInterval(int ms);
    void setAutoScanEnabled(bool enabled);
    void setEnabled(bool enabled);
    void setServerKey(const QString &serverKey);
    void init();
    bool netCheckAvailable();

Q_SIGNALS:
    void request(NetManager::CmdType cmd, const QString &id, const QVariantMap &param);

protected Q_SLOTS:
    //  执行操作
    void exec(NetManager::CmdType cmd, const QString &id, const QVariantMap &param);
    // 获取数据
    void onItemAdded(const QString &parentID, NetItem *item);
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
    void setDeviceEnabled(NetControlItem *controlItem, bool enabled);
    void updateControl();
    void updateItemVisible(const QString &id, bool visible);
    void updateControlEnabled(int type);
    void updateAirplaneMode(bool enabled);
    void updatePrimaryConnectionType(NetManager::ConnectionType type);

    void addItem(NetItem *item, NetItem *parentItem = nullptr);
    void removeItem(NetItem *item);
    void removeAndDeleteItem(NetItem *item);

    inline NetItem *findItem(const QString &id) const { return m_dataMap.value(id, nullptr); }

private:
    bool m_isGreeterMode;
    NetItem *m_root;
    bool m_isDeleting;
    bool m_airplaneMode;
    NetManager::ConnectionType m_primaryConnectionType;
    bool m_monitorNetworkNotify;
    bool m_secretAgent;
    bool m_autoAddConnection;
    NetManagerThreadPrivate *m_managerThread;
    QMap<QString, NetItem *> m_dataMap;
    PasswordRequest *m_passwordRequestData;
    QString m_showInputId;

    int m_deviceCount[DeviceItemCount];

    NetManager *q_ptr;
    Q_DECLARE_PUBLIC(NetManager)

    friend class NetManager;
};

} // namespace network
} // namespace dde

#endif //  NETMANAGERPRIVATE_H
