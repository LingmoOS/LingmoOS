// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETMANAGERTHREADPRIVATE_H
#define NETMANAGERTHREADPRIVATE_H

#include "netitem.h"
#include "netmanager.h"

#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/WirelessSecuritySetting>

#include <QObject>

class QTimer;

namespace NetworkManager {
class WiredDevice;
class AccessPoint;
} // namespace NetworkManager

namespace ocean {
namespace network {
class NetManager;
class NetItem;
class VPNItem;
class DSLItem;
class NetDeviceItemPrivate;
class NetSecretAgentInterface;
enum class NetConnectionStatus;
enum class NetworkNotifyType;
enum class ProxyMethod;

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
    static bool CheckPasswordValid(const QString &key, const QString &password);

    inline bool NetCheckAvailable() { return m_netCheckAvailable; }

    inline bool AirplaneModeEnabled() { return m_airplaneModeEnabled; }

    void setMonitorNetworkNotify(bool monitor);
    void setUseSecretAgent(bool enabled);

    void setEnabled(bool enabled);
    void setNetwork8021XMode(NetManager::Network8021XMode mode);
    void setAutoUpdateHiddenConfig(bool autoUpdate);
    void setAutoScanInterval(int ms);
    void setAutoScanEnabled(bool enabled);
    void setServerKey(const QString &serverKey);

    void init(NetType::NetManagerFlags flags);

    enum DataChanged {
        NameChanged,
        EnabledChanged,
        ConnectionStatusChanged,
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
        primaryConnectionTypeChanged,
        DetailsChanged,
        IndexChanged,
        ProxyMethodChanged,
        SystemAutoProxyChanged,
        SystemManualProxyChanged,
        AppProxyChanged,
        HotspotConfigChanged,
        HotspotOptionalDeviceChanged,
        HotspotShareDeviceChanged,
    };

Q_SIGNALS:
    void request(NetManager::CmdType cmd, const QString &id, const QVariantMap &param);
    void requestInputPassword(const QString &dev, const QString &id, const QVariantMap &param);

    void itemAoceand(const QString &parentID, NetItemPrivate *item);
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
    void disconnectConnection(const QString &path);
    void connectHidden(const QString &id, const QString &ssid);
    void connectWired(const QString &id, const QVariantMap &param);
    void connectWireless(const QString &id, const QVariantMap &param);
    void connectHotspot(const QString &id, const QVariantMap &param, bool connect);
    void gotoControlCenter(const QString &page);
    void gotoSecurityTools(const QString &page);
    void userCancelRequest(const QString &id);
    void retranslate(const QString &locale); // 更新翻译
    void sendNotify(const QString &appIcon,
                    const QString &body,
                    const QString &summary = QString(),
                    const QString &inAppName = "ocean-control-center",
                    int replacesId = -1,
                    const QStringList &actions = {},
                    const QVariantMap &hints = {},
                    int expireTimeout = 5000);
    void onNetCheckPropertiesChanged(QString, QVariantMap properties, QStringList);
    void onAirplaneModeEnabledPropertiesChanged(const QString &, const QVariantMap &properties, const QStringList &);
    void onAirplaneModePropertiesChanged(const QVariantMap &properties);

    void connectOrInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param);
    void getConnectInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param);
    void setConnectInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param);
    void deleteConnect(const QString &uuid);
    void importConnect(const QString &id, const QString &file);
    void exportConnect(const QString &id, const QString &file);
    void showPage(const QString &cmd);

protected Q_SLOTS:
    void doInit();
    void clearData();
    //  执行操作
    void doSetDeviceEnabled(const QString &id, bool enabled);
    void doRequestScan(const QString &id);
    void doDisconnectDevice(const QString &id);
    void doDisconnectConnection(const QString &path);
    void doConnectHidden(const QString &id, const QString &ssid);
    void doConnectWired(const QString &id, const QVariantMap &param);
    void doConnectWireless(const QString &id, const QVariantMap &param);
    void doConnectHotspot(const QString &id, const QVariantMap &param, bool connect);
    void doGotoControlCenter(const QString &page);
    void doGotoSecurityTools(const QString &page);
    void doUserCancelRequest(const QString &id);
    void doRetranslate(const QString &locale);
    void updateNetCheckAvailabled(const QDBusVariant &availabled);
    void updateAirplaneModeEnabled(const QDBusVariant &enabled);
    void updateAirplaneModeEnabledable(const QDBusVariant &enabledable);
    bool supportAirplaneMode() const;
    void doConnectOrInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param);
    void doGetConnectInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param);
    void doSetConnectInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param);
    void doDeleteConnect(const QString &uuid);
    void changeVpnId();
    void doImportConnect(const QString &id, const QString &file);
    void doExportConnect(const QString &id, const QString &file);
    void doSetSystemProxy(const QVariantMap &param);
    void doSetAppProxy(const QVariantMap &param);
    void clearShowPageCmd();
    bool toShowPage();
    void doShowPage(const QString &cmd);

    void sendRequest(NetManager::CmdType cmd, const QString &id, const QVariantMap &param = QVariantMap());

    // 获取数据
    // Device
    void onDeviceAoceand(QList<NetworkDeviceBase *> devices);
    void onDeviceRemoved(QList<NetworkDeviceBase *> devices);
    void onConnectivityChanged();
    // Wired
    void onConnectionAoceand(const QList<WiredConnection *> &conns);
    void onConnectionRemoved(const QList<WiredConnection *> &conns);
    void addConnection(const NetworkDeviceBase *device, const QList<WiredConnection *> &conns);
    void onConnectionChanged();
    // Wireless
    void onNetworkAoceand(const QList<AccessPoints *> &aps);
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
    // VPN
    void onVPNAoceand(const QList<VPNItem *> &vpns);
    void onVPNRemoved(const QList<VPNItem *> &vpns);
    void onVPNEnableChanged(const bool enable);
    void onVpnActiveConnectionChanged();
    void onVPNConnectionChanged();
    // 系统代理
    void onSystemProxyExistChanged(bool exist);
    void onSystemProxyMethodChanged(const ProxyMethod &method);
    void onSystemAutoProxyChanged(const QString &url);
    void onSystemManualProxyChanged();

    // 应用代理
    void onAppProxyEnableChanged(bool enabled);
    void onAppProxyChanged();
    // 热点
    void updateHotspotEnabledChanged(const bool enabled);
    void onHotspotEnabledableChanged(const bool enabledable);
    void onHotspotConfigChanged(const QVariantMap &config);
    void onHotspotOptionalDeviceChanged(const QStringList &optionalDevice);
    void onHotspotShareDeviceChanged(const QStringList &shareDevice);
    // 飞行模式
    void getAirplaneModeEnabled();
    void setAirplaneModeEnabled(bool enabled);
    //  DSL
    void onDSLAoceand(const QList<DSLItem *> &dsls);
    void onDSLRemoved(const QList<DSLItem *> &dsls);
    void onDslActiveConnectionChanged();
    // 网络详情
    void updateDetails();

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
    void onPrepareForSleep(bool state);

protected:
    void addDevice(NetDeviceItemPrivate *deviceItem, NetworkDeviceBase *dev);

    void getNetCheckAvailableFromDBus();

    inline QString apID(AccessPoints *ap) const { return QString::number(reinterpret_cast<quintptr>(ap), 16); }

    AccessPoints *fromApID(const QString &id);
    void requestPassword(const QString &dev, const QString &id, const QVariantMap &param);
    QString connectionSuffixNum(const QString &matchConnName, const QString &name = QString());
    NetworkManager::WirelessSecuritySetting::KeyMgmt getKeyMgmtByAp(NetworkManager::AccessPoint *ap);

    static NetType::NetDeviceStatus toNetDeviceStatus(ConnectionStatus status);
    static NetType::NetConnectionStatus toNetConnectionStatus(ConnectionStatus status);
    static NetType::NetDeviceStatus deviceStatus(NetworkDeviceBase *device);

private:
    QThread *m_thread;
    QThread *m_parentThread;

    NetType::NetManagerFlags m_flags;

    // bool m_loadForNM;
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
    bool m_isSleeping;
    QString m_serverKey;
    QSet<QString> m_detailsItems;
    QString m_showPageCmd;
    QTimer *m_showPageTimer;
    QString m_newVPNuuid;
};

} // namespace network
} // namespace ocean

#endif // NETMANAGERTHREADPRIVATE_H
