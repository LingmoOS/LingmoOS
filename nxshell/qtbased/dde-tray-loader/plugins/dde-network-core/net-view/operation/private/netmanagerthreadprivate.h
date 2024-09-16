// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETMANAGERTHREADPRIVATE_H
#define NETMANAGERTHREADPRIVATE_H

#include "netitem.h"
#include "netmanager.h"

#include <NetworkManagerQt/Device>

#include <QObject>

class QTimer;

namespace NetworkManager {
class WiredDevice;
}

namespace dde {
namespace network {
class NetManager;
class NetItem;
class NetDeviceItem;
class NetSecretAgentInterface;
enum class NetConnectionStatus;
enum class NetworkNotifyType;

class NetworkDeviceBase;
class WirelessDevice;
class WiredConnection;
class AccessPoints;
enum class ConnectionStatus;
enum class ServiceLoadType;

// 与NM交互,子线程内
class NetManagerThreadPrivate : public QObject
{
    Q_OBJECT
public:
    explicit NetManagerThreadPrivate();
    ~NetManagerThreadPrivate() override;

    static QVariantMap CheckParamValid(const QVariantMap &param);
    inline bool NetCheckAvailable() {return m_netCheckAvailable; }
    inline bool AirplaneModeEnabled() { return m_airplaneModeEnabled; }
    void setServiceLoadForNM(bool isNM);
    void setMonitorNetworkNotify(bool monitor);
    void setUseSecretAgent(bool enabled);

    void setEnabled(bool enabled);
    void setNetwork8021XMode(NetManager::Network8021XMode mode);
    void setAutoUpdateHiddenConfig(bool autoUpdate);
    void setAutoScanInterval(int ms);
    void setAutoScanEnabled(bool enabled);
    void setServerKey(const QString &serverKey);

    void init();

    enum DataChanged {
        NameChanged,
        EnabledChanged,
        WiredStatusChanged,
        WirelessStatusChanged,
        StrengthChanged,
        SecuredChanged,
        IPChanged,
        DeviceStatusChanged,
        HotspotEnabledChanged,
        DeviceAvailableChanged,
        AvailableConnectionsChanged,
        AirplaneModeEnabledChanged,
        VPNConnectionStateChanged,
        primaryConnectionTypeChanged
    };

Q_SIGNALS:
    void request(NetManager::CmdType cmd, const QString &id, const QVariantMap &param);
    void requestInputPassword(const QString &dev, const QString &id, const QVariantMap &param);

    void itemAdded(const QString &parentID, NetItem *item);
    void itemRemoved(const QString &id);

    void dataChanged(int dataType, const QString &id, const QVariant &value);
    // clang-format off
    void networkNotify(const QString &inAppName, int replacesId, const QString &appIcon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expireTimeout);
    // clang-format on
    void toControlCenter();
    void netCheckAvailableChanged(const bool &netCheckAvailable);

public Q_SLOTS:
    void setDeviceEnabled(const QString &id, bool enabled);
    void requestScan(const QString &id);
    void disconnectDevice(const QString &id);
    void connectHidden(const QString &id, const QString &ssid);
    void connectWired(const QString &id, const QVariantMap &param);
    void connectWireless(const QString &id, const QVariantMap &param);
    void gotoControlCenter(const QString &page);
    void gotoSecurityTools(const QString &page);
    void userCancelRequest(const QString &id);
    void retranslate(const QString &locale); // 更新翻译
    void sendNotify(const QString &appIcon,
                    const QString &body,
                    const QString &summary = QString(),
                    const QString &inAppName = "dde-control-center",
                    int replacesId = -1,
                    const QStringList &actions = {},
                    const QVariantMap &hints = {},
                    int expireTimeout = 5000);
    void onNetCheckPropertiesChanged(QString, QVariantMap properties, QStringList);
    void onAirplaneModeEnabledPropertiesChanged(QString, QVariantMap properties, QStringList);

protected Q_SLOTS:
    void doInit();
    void clearData();
    //  执行操作
    void doSetDeviceEnabled(const QString &id, bool enabled);
    void doRequestScan(const QString &id);
    void doDisconnectDevice(const QString &id);
    void doConnectHidden(const QString &id, const QString &ssid);
    void doConnectWired(const QString &id, const QVariantMap &param);
    void doConnectWireless(const QString &id, const QVariantMap &param);
    void doGotoControlCenter(const QString &page);
    void doGotoSecurityTools(const QString &page);
    void doUserCancelRequest(const QString &id);
    void doRetranslate(const QString &locale);
    void updateNetCheckAvailabled(const QDBusVariant &availabled);
    void updateAirplaneModeEnabled(const QDBusVariant &enabled);
    bool supportAirplaneMode() const;

    void sendRequest(NetManager::CmdType cmd, const QString &id, const QVariantMap &param = QVariantMap());

    // 获取数据
    // Device
    void onDeviceAdded(QList<NetworkDeviceBase *> devices);
    void onDeviceRemoved(QList<NetworkDeviceBase *> devices);
    void onConnectivityChanged();
    // Wired
    void onConnectionAdded(const QList<WiredConnection *> &conns);
    void onConnectionRemoved(const QList<WiredConnection *> &conns);
    void addConnection(const NetworkDeviceBase *device, const QList<WiredConnection *> &conns);
    // Wireless
    void onNetworkAdded(const QList<AccessPoints *> &aps);
    void onNetworkRemoved(const QList<AccessPoints *> &aps);
    void addNetwork(const NetworkDeviceBase *device, QList<AccessPoints *> aps);
    // device
    void onNameChanged(const QString &name);
    void onDevEnabledChanged(const bool enabled);
    void onDevAvailableChanged(const bool available);
    void onActiveConnectionChanged();
    void onIpV4Changed();
    void onDeviceStatusChanged();
    void onHotspotEnabledChanged();
    void onAvailableConnectionsChanged();
    // ap
    void onStrengthChanged(int strength);
    void onAPStatusChanged(ConnectionStatus status);
    void onAPSecureChanged(bool secure);

    // 自动扫描
    void updateAutoScan();
    void doAutoScan();
    // 处理通知
    void addDeviceNotify(const QString &path);
    void onNotifyDeviceStatusChanged(NetworkManager::Device::State newState, NetworkManager::Device::State oldState, NetworkManager::Device::StateChangeReason reason);
    void sendNetworkNotify(NetworkNotifyType type, const QString &name);
    // 处理隐藏网络
    void updateHiddenNetworkConfig(WirelessDevice *wireless);
    bool needSetPassword(AccessPoints *accessPoint) const;
    void handleAccessPointSecure(AccessPoints *accessPoint);
    void handle8021xAccessPoint(AccessPoints *ap);

protected:
    void addDevice(NetDeviceItem *deviceItem, NetworkDeviceBase *dev);

    void getNetCheckAvailableFromDBus();
    void getAirplaneModeEnabled();

    inline QString apID(AccessPoints *ap) const { return QString::number(reinterpret_cast<quintptr>(ap), 16); }

    void requestPassword(const QString &dev, const QString &id, const QVariantMap &param);

    static NetDeviceStatus toNetDeviceStatus(ConnectionStatus status);
    static NetConnectionStatus toNetConnectionStatus(ConnectionStatus status);
    static NetDeviceStatus deviceStatus(NetworkDeviceBase *device);

private:
    QThread *m_thread;
    QThread *m_parentThread;

    bool m_loadForNM;
    bool m_monitorNetworkNotify;
    bool m_useSecretAgent;
    NetManager::Network8021XMode m_network8021XMode;
    bool m_autoUpdateHiddenConfig; // 自动更新隐藏网络配置
    bool m_isInitialized;
    bool m_enabled;
    int m_autoScanInterval;
    bool m_autoScanEnabled;
    QTimer *m_autoScanTimer;
    int m_lastThroughTime;

    // 通知相关变量
    QString m_lastConnection;
    QString m_lastConnectionUuid;
    NetworkManager::Device::State m_lastState;
    // 密码处理
    NetSecretAgentInterface *m_secretAgent;

    bool m_netCheckAvailable;
    bool m_airplaneModeEnabled;
    QString m_serverKey;
};

} // namespace network
} // namespace dde

#endif // NETMANAGERTHREADPRIVATE_H
