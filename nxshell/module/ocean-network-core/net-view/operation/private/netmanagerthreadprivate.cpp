// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "netmanagerthreadprivate.h"

#include "configsetting.h"
#include "dslcontroller.h"
#include "hotspotcontroller.h"
#include "impl/configwatcher.h"
#include "impl/networkmanager/nmnetworkmanager.h"
#include "nethotspotcontroller.h"
#include "netitemprivate.h"
#include "netsecretagent.h"
#include "netsecretagentforui.h"
#include "netwirelessconnect.h"
#include "networkcontroller.h"
#include "networkdetails.h"
#include "networkdevicebase.h"
#include "networkmanagerqt/manager.h"
#include "wireoceanvice.h"
#include "wirelessdevice.h"

#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/PppSetting>
#include <NetworkManagerQt/PppoeSetting>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <impl/vpncontroller.h>
#include <proxycontroller.h>

#include <QDBusConnection>
#include <QThread>

using namespace NetworkManager;

namespace ocean {
namespace network {
enum class NetworkNotifyType {
    WiredConnecting,          // 有线连接中
    WirelessConnecting,       // 无线连接中
    WiredConnected,           // 有线已连接
    WirelessConnected,        // 无线已连接
    WiredDisconnected,        // 有线断开
    WirelessDisconnected,     // 无线断开
    WiredUnableConnect,       // 有线无法连接
    WirelessUnableConnect,    //  无线无法连接
    WiredConnectionFailed,    // 有线连接失败
    WirelessConnectionFailed, // 无线连接失败
    NoSecrets,                // 密码错误
    SsidNotFound,             // 没找到ssid
    Wireless8021X             // 企业版认证
};
const QString notifyIconNetworkOffline = "notification-network-offline";
const QString notifyIconWiredConnected = "notification-network-wired-connected";
const QString notifyIconWiredDisconnected = "notification-network-wired-disconnected";
const QString notifyIconWiredError = "notification-network-wired-disconnected";
const QString notifyIconWirelessConnected = "notification-network-wireless-full";
const QString notifyIconWirelessDisconnected = "notification-network-wireless-disconnected";
const QString notifyIconWirelessDisabled = "notification-network-wireless-disabled";
const QString notifyIconWirelessError = "notification-network-wireless-disconnected";
const QString notifyIconVpnConnected = "notification-network-vpn-connected";
const QString notifyIconVpnDisconnected = "notification-network-vpn-disconnected";
const QString notifyIconProxyEnabled = "notification-network-proxy-enabled";
const QString notifyIconProxyDisabled = "notification-network-proxy-disabled";
const QString notifyIconNetworkConnected = "notification-network-wired-connected";
const QString notifyIconNetworkDisconnected = "notification-network-wired-disconnected";
const QString notifyIconMobile2gConnected = "notification-network-mobile-2g-connected";
const QString notifyIconMobile2gDisconnected = "notification-network-mobile-2g-disconnected";
const QString notifyIconMobile3gConnected = "notification-network-mobile-3g-connected";
const QString notifyIconMobile3gDisconnected = "notification-network-mobile-3g-disconnected";
const QString notifyIconMobile4gConnected = "notification-network-mobile-4g-connected";
const QString notifyIconMobile4gDisconnected = "notification-network-mobile-4g-disconnected";
const QString notifyIconMobileUnknownConnected = "notification-network-mobile-unknown-connected";
const QString notifyIconMobileUnknownDisconnected = "notification-network-mobile-unknown-disconnected";
#define MANULCONNECTION 1

NetManagerThreadPrivate::NetManagerThreadPrivate()
    : QObject()
    , m_thread(new QThread(this))
    , m_parentThread(QThread::currentThread())
    , m_monitorNetworkNotify(false)
    , m_useSecretAgent(true)
    , m_network8021XMode(NetManager::ToControlCenter)
    , m_autoUpdateHioceannConfig(true)
    , m_isInitialized(false)
    , m_enabled(true)
    , m_autoScanInterval(0)
    , m_autoScanEnabled(false)
    , m_autoScanTimer(nullptr)
    , m_lastThroughTime(0)
    , m_lastState(NetworkManager::Device::State::UnknownState)
    , m_secretAgent(nullptr)
    , m_netCheckAvailable(false)
    , m_isSleeping(false)
    , m_showPageTimer(nullptr)
{
    moveToThread(m_thread);
    m_thread->start();
}

NetManagerThreadPrivate::~NetManagerThreadPrivate()
{
    m_thread->quit();
    m_thread->wait(QDeadlineTimer(200));
    if (m_thread->isRunning()) {
        m_thread->terminate();
    }
    m_thread->wait(QDeadlineTimer(200));
    delete m_thread;
}

// 检查参数,参数有错误的才在QVariantMap里,value暂时为空(预留以后要显示具体错误)
QVariantMap NetManagerThreadPrivate::CheckParamValid(const QVariantMap &param)
{
    QVariantMap validMap;
    for (auto &&it = param.cbegin(); it != param.cend(); ++it) {
        const QString &key = it.key();
        if (!CheckPasswordValid(key, it.value().toString())) {
            validMap.insert(key, QString());
        }
    }
    return validMap;
}

bool NetManagerThreadPrivate::CheckPasswordValid(const QString &key, const QString &password)
{
    if (key == "psk") {
        return NetworkManager::wpaPskIsValid(password);
    } else if (key == "wep-key0" || key == "wep-key1" || key == "wep-key2" || key == "wep-key3") {
        return NetworkManager::wepKeyIsValid(password, WirelessSecuritySetting::WepKeyType::Passphrase);
    }
    return !password.isEmpty();
}

void NetManagerThreadPrivate::getNetCheckAvailableFromDBus()
{
    QDBusMessage message = QDBusMessage::createMethodCall("com.lingmo.defender.netcheck", "/com/lingmo/defender/netcheck", "org.freedesktop.DBus.Properties", "Get");
    message << "com.lingmo.defender.netcheck"
            << "Availabled";
    QDBusConnection::systemBus().callWithCallback(message, this, SLOT(updateNetCheckAvailabled(QDBusVariant)));
}

void NetManagerThreadPrivate::getAirplaneModeEnabled()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.lingmo.ocean.AirplaneMode1", "/org/lingmo/ocean/AirplaneMode1", "org.freedesktop.DBus.Properties", "GetAll");
    message << "org.lingmo.ocean.AirplaneMode1";
    QDBusConnection::systemBus().callWithCallback(message, this, SLOT(onAirplaneModePropertiesChanged(QVariantMap)));
}

void NetManagerThreadPrivate::setAirplaneModeEnabled(bool enabled)
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.lingmo.ocean.AirplaneMode1", "/org/lingmo/ocean/AirplaneMode1", "org.lingmo.ocean.AirplaneMode1", "Enable");
    message << enabled;
    QDBusConnection::systemBus().callWithCallback(message, this, SLOT(getAirplaneModeEnabled()));
}

AccessPoints *NetManagerThreadPrivate::fromApID(const QString &id)
{
    AccessPoints *ap = nullptr;
    for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
        if (device->deviceType() == DeviceType::Wireless) {
            WirelessDevice *wirelessDev = qobject_cast<WirelessDevice *>(device);
            for (auto &&tmpAp : wirelessDev->accessPointItems()) {
                if (apID(tmpAp) == id) {
                    ap = tmpAp;
                    break;
                }
            }
            if (ap)
                break;
        }
    }
    return ap;
}

void NetManagerThreadPrivate::setMonitorNetworkNotify(bool monitor)
{
    if (m_isInitialized)
        return;
    m_monitorNetworkNotify = monitor;
}

void NetManagerThreadPrivate::setUseSecretAgent(bool enabled)
{
    if (m_isInitialized)
        return;
    m_useSecretAgent = enabled;
}

void NetManagerThreadPrivate::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

void NetManagerThreadPrivate::setNetwork8021XMode(NetManager::Network8021XMode mode)
{
    NetManager::Network8021XMode networkMode = mode;
    switch (mode) {
    case NetManager::Network8021XMode::ToControlCenterUnderConnect: {
        // 如果开起该配置，那么在第一次连接企业网的时候，弹出用户名密码输入框，否则就跳转到控制中心（工行定制）
        networkMode = ConfigSetting::instance()->enableEapInput() ? NetManager::Network8021XMode::ToConnect : NetManager::Network8021XMode::ToControlCenter;
        break;
    }
    case NetManager::Network8021XMode::SendNotifyUnderConnect: {
        // 如果开启该配置，那么在第一次连接企业网的时候，弹出用户名密码输入框，否则给出提示消息（工行定制）
        networkMode = ConfigSetting::instance()->enableEapInput() ? NetManager::Network8021XMode::ToConnect : NetManager::Network8021XMode::SendNotify;
        break;
    }
    default:
        break;
    }
    m_network8021XMode = networkMode;
}

void NetManagerThreadPrivate::setAutoUpdateHioceannConfig(bool autoUpdate)
{
    m_autoUpdateHioceannConfig = autoUpdate;
}

void NetManagerThreadPrivate::setAutoScanInterval(int ms)
{
    m_autoScanInterval = ms;
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "updateAutoScan", Qt::QueuedConnection);
}

void NetManagerThreadPrivate::setAutoScanEnabled(bool enabled)
{
    m_autoScanEnabled = enabled;
    if (m_isInitialized) {
        QMetaObject::invokeMethod(this, "updateAutoScan", Qt::QueuedConnection);
        if (m_autoScanEnabled)
            QMetaObject::invokeMethod(this, "doAutoScan", Qt::QueuedConnection);
    }
}

void NetManagerThreadPrivate::setServerKey(const QString &serverKey)
{
    m_serverKey = serverKey;
}

void NetManagerThreadPrivate::init(NetType::NetManagerFlags flags)
{
    // 在主线程中先安装翻译器，因为直接在子线程中安装翻译器可能会引起崩溃
    // NetworkController::installTranslator(QLocale().name());
    m_flags = flags;
    QMetaObject::invokeMethod(this, &NetManagerThreadPrivate::doInit, Qt::QueuedConnection);
}

void NetManagerThreadPrivate::setDeviceEnabled(const QString &id, bool enabled)
{
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "doSetDeviceEnabled", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(bool, enabled));
}

void NetManagerThreadPrivate::requestScan(const QString &id)
{
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "doRequestScan", Qt::QueuedConnection, Q_ARG(QString, id));
}

void NetManagerThreadPrivate::disconnectDevice(const QString &id)
{
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "doDisconnectDevice", Qt::QueuedConnection, Q_ARG(QString, id));
}

void NetManagerThreadPrivate::disconnectConnection(const QString &path)
{
    QMetaObject::invokeMethod(this, "doDisconnectConnection", Qt::QueuedConnection, Q_ARG(QString, path));
}

void NetManagerThreadPrivate::connectHioceann(const QString &id, const QString &ssid)
{
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "doConnectHioceann", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(QString, ssid));
}

void NetManagerThreadPrivate::connectWired(const QString &id, const QVariantMap &param)
{
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "doConnectWired", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(QVariantMap, param));
}

void NetManagerThreadPrivate::connectWireless(const QString &id, const QVariantMap &param)
{
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "doConnectWireless", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(QVariantMap, param));
}

void NetManagerThreadPrivate::connectHotspot(const QString &id, const QVariantMap &param, bool connect)
{
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "doConnectHotspot", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(QVariantMap, param), Q_ARG(bool, connect));
}

void NetManagerThreadPrivate::gotoControlCenter(const QString &page)
{
    QMetaObject::invokeMethod(this, "doGotoControlCenter", Qt::QueuedConnection, Q_ARG(QString, page));
}

void NetManagerThreadPrivate::gotoSecurityTools(const QString &page)
{
    QMetaObject::invokeMethod(this, "doGotoSecurityTools", Qt::QueuedConnection, Q_ARG(QString, page));
}

void NetManagerThreadPrivate::userCancelRequest(const QString &id)
{
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "doUserCancelRequest", Qt::QueuedConnection, Q_ARG(QString, id));
}

void NetManagerThreadPrivate::retranslate(const QString &locale)
{
    NetworkController::installTranslator(QLocale().name());
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "doRetranslate", Qt::QueuedConnection, Q_ARG(QString, locale));
}

// clang-format off
void NetManagerThreadPrivate::sendNotify(const QString &appIcon, const QString &body, const QString &summary, const QString &inAppName, int replacesId, const QStringList &actions, const QVariantMap &hints, int expireTimeout)
{
    if (!m_enabled)
        return;
    Q_EMIT networkNotify(inAppName, replacesId, appIcon, summary, body, actions, hints, expireTimeout);
}

// clang-format on
void NetManagerThreadPrivate::onNetCheckPropertiesChanged(QString, QVariantMap properties, QStringList)
{
    if (properties.contains("Availabled")) {
        updateNetCheckAvailabled(properties.value("Availabled").value<QDBusVariant>());
    }
}

void NetManagerThreadPrivate::onAirplaneModeEnabledPropertiesChanged(const QString &, const QVariantMap &properties, const QStringList &)
{
    onAirplaneModePropertiesChanged(properties);
}

void NetManagerThreadPrivate::onAirplaneModePropertiesChanged(const QVariantMap &properties)
{
    if (properties.contains("Enabled")) {
        updateAirplaneModeEnabled(QDBusVariant(properties.value("Enabled").value<bool>()));
    }
    if (properties.contains("HasAirplaneMode")) {
        updateAirplaneModeEnabledable(QDBusVariant(properties.value("HasAirplaneMode").value<bool>()));
    }
}

void NetManagerThreadPrivate::connectOrInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param)
{
    QMetaObject::invokeMethod(this, "doConnectOrInfo", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(NetType::NetItemType, type), Q_ARG(QVariantMap, param));
}

void NetManagerThreadPrivate::getConnectInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param)
{
    QMetaObject::invokeMethod(this, "doGetConnectInfo", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(NetType::NetItemType, type), Q_ARG(QVariantMap, param));
}

void NetManagerThreadPrivate::setConnectInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param)
{
    QMetaObject::invokeMethod(this, "doSetConnectInfo", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(NetType::NetItemType, type), Q_ARG(QVariantMap, param));
}

void NetManagerThreadPrivate::deleteConnect(const QString &uuid)
{
    QMetaObject::invokeMethod(this, "doDeleteConnect", Qt::QueuedConnection, Q_ARG(QString, uuid));
}

void NetManagerThreadPrivate::importConnect(const QString &id, const QString &file)
{
    QMetaObject::invokeMethod(this, "doImportConnect", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(QString, file));
}

void NetManagerThreadPrivate::exportConnect(const QString &id, const QString &file)
{
    QMetaObject::invokeMethod(this, "doExportConnect", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(QString, file));
}

void NetManagerThreadPrivate::showPage(const QString &cmd)
{
    QMetaObject::invokeMethod(this, "doShowPage", Qt::QueuedConnection, Q_ARG(QString, cmd));
}

void NetManagerThreadPrivate::doInit()
{
    if (m_isInitialized)
        return;
    m_isInitialized = true;

    qRegisterMetaType<NetworkManager::Device::State>("NetworkManager::Device::State");
    qRegisterMetaType<NetworkManager::Device::StateChangeReason>("NetworkManager::Device::StateChangeReason");
    qRegisterMetaType<Connectivity>("Connectivity");

    if (m_flags.testFlag(NetType::NetManagerFlag::Net_ServiceNM)) {
        NetworkController::alawaysLoadFromNM();
    }
    NetworkController::setIPConflictCheck(true);
    NetworkController *networkController = NetworkController::instance();

    connect(m_thread, &QThread::finished, this, &NetManagerThreadPrivate::clearData);
    connect(networkController, &NetworkController::deviceAoceand, this, &NetManagerThreadPrivate::onDeviceAoceand);
    connect(networkController, &NetworkController::deviceRemoved, this, &NetManagerThreadPrivate::onDeviceRemoved);
    connect(networkController, &NetworkController::connectivityChanged, this, &NetManagerThreadPrivate::onConnectivityChanged);

    if (m_flags.testFlag(NetType::NetManagerFlag::Net_UseSecretAgent)) {
        // 密码代理按设置来，不与ConfigSetting::instance()->serviceFromNetworkManager()同步
        if (m_flags.testFlag(NetType::NetManagerFlag::Net_ServiceNM)) {
            m_secretAgent = new NetSecretAgent(std::bind(&NetManagerThreadPrivate::requestPassword, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), true, this);
        } else {
            m_secretAgent = new NetSecretAgentForUI(std::bind(&NetManagerThreadPrivate::requestPassword, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), m_serverKey, this);
        }
    }

    onDeviceAoceand(networkController->devices());
    if (m_autoScanInterval == 0) { // 没有设置则以配置中值设置下
        m_autoScanInterval = ConfigSetting::instance()->wirelessScanInterval();
        connect(ConfigSetting::instance(), &ConfigSetting::wirelessScanIntervalChanged, this, &NetManagerThreadPrivate::setAutoScanInterval);
    }
    updateAutoScan();

    // VPN
    if (m_flags.testFlags(NetType::NetManagerFlag::Net_VPN)) {
        NetVPNControlItemPrivate *vpnControlItem = NetItemNew(VPNControlItem, "NetVPNControlItem");
        vpnControlItem->updatename("VPN");
        vpnControlItem->updateenabled(networkController->vpnController()->enabled());
        vpnControlItem->item()->moveToThread(m_parentThread);
        Q_EMIT itemAoceand("Root", vpnControlItem);

        connect(networkController->vpnController(), &VPNController::enableChanged, this, &NetManagerThreadPrivate::onVPNEnableChanged);
        // connect(networkController->vpnController(), &VPNController::itemChanged, this, vpnItemChanged);
        connect(networkController->vpnController(), &VPNController::itemAoceand, this, &NetManagerThreadPrivate::onVPNAoceand);
        connect(networkController->vpnController(), &VPNController::itemRemoved, this, &NetManagerThreadPrivate::onVPNRemoved);
        connect(networkController->vpnController(), &VPNController::activeConnectionChanged, this, &NetManagerThreadPrivate::onVpnActiveConnectionChanged);
        onVPNAoceand(networkController->vpnController()->items());
    }
    // 系统代理
    if (m_flags.testFlags(NetType::NetManagerFlag::Net_SysProxy)) {
        networkController->proxyController()->querySysProxyData();
        ProxyMethod method = networkController->proxyController()->proxyMethod();
        NetSystemProxyControlItemPrivate *item = NetItemNew(SystemProxyControlItem, "NetSystemProxyControlItem");
        item->updatename("SystemProxy");
        item->updateenabled(method == ProxyMethod::Auto || method == ProxyMethod::Manual);
        // item->updateenabledable(networkController->proxyController()->systemProxyExist());
        item->item()->moveToThread(m_parentThread);
        Q_EMIT itemAoceand("Root", item);
        onSystemAutoProxyChanged(networkController->proxyController()->autoProxy());
        onSystemManualProxyChanged();

        // connect(networkController->proxyController(), &ProxyController::systemProxyExistChanged, this, &NetManagerThreadPrivate::onSystemProxyExistChanged);
        connect(networkController->proxyController(), &ProxyController::proxyMethodChanged, this, &NetManagerThreadPrivate::onSystemProxyMethodChanged);
        connect(networkController->proxyController(), &ProxyController::autoProxyChanged, this, &NetManagerThreadPrivate::onSystemAutoProxyChanged);
        connect(networkController->proxyController(), &ProxyController::proxyChanged, this, &NetManagerThreadPrivate::onSystemManualProxyChanged);
        connect(networkController->proxyController(), &ProxyController::proxyAuthChanged, this, &NetManagerThreadPrivate::onSystemManualProxyChanged);
        connect(networkController->proxyController(), &ProxyController::proxyIgnoreHostsChanged, this, &NetManagerThreadPrivate::onSystemManualProxyChanged);
    }
    // 应用代理
    if (m_flags.testFlags(NetType::NetManagerFlag::Net_AppProxy)) {
        networkController->proxyController()->querySysProxyData();
        NetAppProxyControlItemPrivate *item = NetItemNew(AppProxyControlItem, "NetAppProxyControlItem");
        item->updatename("AppProxy");
        item->updateenabled(networkController->proxyController()->appProxyEnabled());
        // item->updateenabledable(networkController->proxyController()->appProxyExist());
        item->item()->moveToThread(m_parentThread);
        Q_EMIT itemAoceand("Root", item);
        onAppProxyChanged();

        connect(networkController->proxyController(), &ProxyController::appEnableChanged, this, &NetManagerThreadPrivate::onAppProxyEnableChanged);
        connect(networkController->proxyController(), &ProxyController::appIPChanged, this, &NetManagerThreadPrivate::onAppProxyChanged);
        connect(networkController->proxyController(), &ProxyController::appPasswordChanged, this, &NetManagerThreadPrivate::onAppProxyChanged);
        connect(networkController->proxyController(), &ProxyController::appTypeChanged, this, &NetManagerThreadPrivate::onAppProxyChanged);
        connect(networkController->proxyController(), &ProxyController::appUsernameChanged, this, &NetManagerThreadPrivate::onAppProxyChanged);
        connect(networkController->proxyController(), &ProxyController::appPortChanged, this, &NetManagerThreadPrivate::onAppProxyChanged);
    }

    m_isInitialized = true;
    m_netCheckAvailable = false;
    getNetCheckAvailableFromDBus();

    QDBusConnection::systemBus().connect("com.lingmo.defender.netcheck",
                                         "/com/lingmo/defender/netcheck",
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(onNetCheckPropertiesChanged(QString, QVariantMap, QStringList)));

    QDBusConnection::systemBus().connect("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PrepareForSleep", this, SLOT(onPrepareForSleep(bool)));

    // 优先网络
    auto updadePrimaryConnectionType = [this] {
        Q_EMIT dataChanged(DataChanged::primaryConnectionTypeChanged, "", NetworkManager::primaryConnectionType());
    };
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionTypeChanged, this, updadePrimaryConnectionType);
    updadePrimaryConnectionType();
    // 热点
    if (m_flags.testFlags(NetType::NetManagerFlag::Net_Hotspot)) {
        networkController->hotspotController();
        NetHotspotController *netHotspotController = new NetHotspotController(this);
        NetHotspotControlItemPrivate *hotspotcontrolitem = NetItemNew(HotspotControlItem, "NetHotspotControlItem");
        hotspotcontrolitem->updateconfig(netHotspotController->config());
        hotspotcontrolitem->updateenabledable(netHotspotController->enabledable());
        hotspotcontrolitem->updateenabled(netHotspotController->isEnabled());
        hotspotcontrolitem->updateoptionalDevice(netHotspotController->optionalDevice());
        hotspotcontrolitem->updateshareDevice(netHotspotController->shareDevice());
        hotspotcontrolitem->item()->moveToThread(m_parentThread);
        Q_EMIT itemAoceand("Root", hotspotcontrolitem);
        connect(netHotspotController, &NetHotspotController::enabledChanged, this, &NetManagerThreadPrivate::updateHotspotEnabledChanged);
        connect(netHotspotController, &NetHotspotController::enabledableChanged, this, &NetManagerThreadPrivate::onHotspotEnabledableChanged);
        connect(netHotspotController, &NetHotspotController::configChanged, this, &NetManagerThreadPrivate::onHotspotConfigChanged);
        connect(netHotspotController, &NetHotspotController::optionalDeviceChanged, this, &NetManagerThreadPrivate::onHotspotOptionalDeviceChanged);
        connect(netHotspotController, &NetHotspotController::shareDeviceChanged, this, &NetManagerThreadPrivate::onHotspotShareDeviceChanged);
    }
    // Airplane
    if (m_flags.testFlags(NetType::NetManagerFlag::Net_Airplane)) {
        m_airplaneModeEnabled = false;
        getAirplaneModeEnabled();
        QDBusConnection::systemBus().connect("org.lingmo.ocean.AirplaneMode1",
                                             "/org/lingmo/ocean/AirplaneMode1",
                                             "org.freedesktop.DBus.Properties",
                                             "PropertiesChanged",
                                             this,
                                             SLOT(onAirplaneModeEnabledPropertiesChanged(QString, QVariantMap, QStringList)));
    }
    // DSL
    if (m_flags.testFlags(NetType::NetManagerFlag::Net_DSL)) {
        NetDSLControlItemPrivate *vpnControlItem = NetItemNew(DSLControlItem, "NetDSLControlItem");
        vpnControlItem->updatename("DSL");
        vpnControlItem->updateenabled(networkController->vpnController()->enabled());
        vpnControlItem->item()->moveToThread(m_parentThread);
        Q_EMIT itemAoceand("Root", vpnControlItem);

        networkController->dslController()->connectItem("");
        // connect(networkController->dslController(), &DSLController::enableChanged, this, &NetManagerThreadPrivate::onVPNEnableChanged);
        // connect(networkController->vpnController(), &VPNController::itemChanged, this, vpnItemChanged);
        connect(networkController->dslController(), &DSLController::itemAoceand, this, &NetManagerThreadPrivate::onDSLAoceand);
        connect(networkController->dslController(), &DSLController::itemRemoved, this, &NetManagerThreadPrivate::onDSLRemoved);
        connect(networkController->dslController(), &DSLController::activeConnectionChanged, this, &NetManagerThreadPrivate::onDslActiveConnectionChanged);
        onDSLAoceand(networkController->dslController()->items());
    }
    // Details
    if (m_flags.testFlags(NetType::NetManagerFlag::Net_Details)) {
        NetDetailsItemPrivate *item = NetItemNew(DetailsItem, "Details");
        item->updatename("Details");
        item->item()->moveToThread(m_parentThread);
        Q_EMIT itemAoceand("Root", item);
        updateDetails();
        // connect(networkController, &NetworkController::deviceAoceand, this, &NetManagerThreadPrivate::updateDetails, Qt::QueuedConnection);
        // connect(networkController, &NetworkController::deviceRemoved, this, &NetManagerThreadPrivate::updateDetails, Qt::QueuedConnection);
        // connect(networkController, &NetworkController::connectivityChanged, this, &NetManagerThreadPrivate::updateDetails, Qt::QueuedConnection);
        connect(networkController, &NetworkController::activeConnectionChange, this, &NetManagerThreadPrivate::updateDetails, Qt::QueuedConnection);
    }
    // 初始化的关键参数,保留格式
    qCInfo(DNC) << "Interface Version :" << INTERFACE_VERSION;
    qCInfo(DNC) << "Manager Flags     :" << m_flags;
    qCInfo(DNC) << "Service From NM   :" << m_flags.testFlag(NetType::NetManagerFlag::Net_ServiceNM) << "Config:" << ConfigSetting::instance()->serviceFromNetworkManager();
    qCInfo(DNC) << "Use Secret Agent  :" << m_useSecretAgent;
    qCInfo(DNC) << "Secret Agent      :" << (dynamic_cast<QObject *>(m_secretAgent));
    qCInfo(DNC) << "Auto Scan Interval:" << m_autoScanInterval;
}

void NetManagerThreadPrivate::clearData()
{
    // 此函数是在线程中执行,线程中创建的对象应在此delete
    if (m_autoScanTimer) {
        delete m_autoScanTimer;
        m_autoScanTimer = nullptr;
    }
    if (m_secretAgent) {
        delete m_secretAgent;
        m_secretAgent = nullptr;
    }
    NetworkController::free();
}

void NetManagerThreadPrivate::doSetDeviceEnabled(const QString &id, bool enabled)
{
    if (id == "NetVPNControlItem") {
        NetworkController::instance()->vpnController()->setEnabled(enabled);
        return;
    }
    if (id == "NetSystemProxyControlItem") {
        NetworkController::instance()->proxyController()->setProxyMethod(enabled ? ConfigWatcher::instance()->proxyMethod() : ProxyMethod::None);
        return;
    }
    if (id == "NetHotspotControlItem") {
        HotspotController *hotspotController = NetworkController::instance()->hotspotController();
        for (auto dev : hotspotController->devices()) {
            hotspotController->setEnabled(dev, enabled);
        }
        return;
    }

    for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
        if (device->path() == id) {
            device->setEnabled(enabled);
            break;
        }
    }
}

void NetManagerThreadPrivate::doRequestScan(const QString &id)
{
    for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
        if (device->path() == id) {
            WirelessDevice *wirelessDevice = qobject_cast<WirelessDevice *>(device);
            if (wirelessDevice)
                wirelessDevice->scanNetwork();
            break;
        }
    }
}

void NetManagerThreadPrivate::doDisconnectDevice(const QString &id)
{
    for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
        if (device->path() == id) {
            NetworkDeviceBase *netDevice = qobject_cast<NetworkDeviceBase *>(device);
            if (netDevice)
                netDevice->disconnectNetwork();
            break;
        }
    }
}

void NetManagerThreadPrivate::doDisconnectConnection(const QString &path)
{
    NetworkManager::ActiveConnection::List activeConnections = NetworkManager::activeConnections();
    for (NetworkManager::ActiveConnection::Ptr activeConnection : activeConnections) {
        if (activeConnection->connection()->path() == path) {
            qCInfo(DNC) << "disconnect item:" << activeConnection->path();
            NetworkManager::deactivateConnection(activeConnection->path());
        }
    }
}

void NetManagerThreadPrivate::doConnectHioceann(const QString &id, const QString &ssid)
{
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    auto it = std::find_if(devices.begin(), devices.end(), [id](NetworkDeviceBase *dev) {
        return dev->path() == id;
    });
    if (it == devices.end())
        return;

    WirelessDevice *wirelessDevice = qobject_cast<WirelessDevice *>(*it);
    qCInfo(DNC) << "Wireless connect hioceann, id: " << id << "ssid: " << ssid << "wireless device: " << wirelessDevice;
    if (!wirelessDevice)
        return;
    NetWirelessConnect wConnect(wirelessDevice, nullptr, this);
    wConnect.setSsid(ssid);
    wConnect.initConnection();
    wConnect.connectNetwork();
}

void NetManagerThreadPrivate::doConnectWired(const QString &id, const QVariantMap &param)
{
    Q_UNUSED(param)
    QStringList ids = id.split(":");
    if (ids.size() != 2)
        return;
    for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
        if (device->path() == ids.first()) {
            WiredDevice *netDevice = qobject_cast<WiredDevice *>(device);
            for (auto &&conn : netDevice->items()) {
                if (conn->connection() && conn->connection()->path() == ids.at(1)) {
                    qCInfo(DNC) << "Connect wired, device name: " << netDevice->deviceName() << "connection name: " << conn->connection()->id();
                    netDevice->connectNetwork(conn);
                }
            }
            break;
        }
    }
}

void NetManagerThreadPrivate::doConnectWireless(const QString &id, const QVariantMap &param)
{
    WirelessDevice *wirelessDevice = nullptr;
    AccessPoints *ap = nullptr;
    for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
        if (device->deviceType() == DeviceType::Wireless) {
            WirelessDevice *wirelessDev = qobject_cast<WirelessDevice *>(device);
            for (auto &&tmpAp : wirelessDev->accessPointItems()) {
                if (apID(tmpAp) == id) {
                    wirelessDevice = wirelessDev;
                    ap = tmpAp;
                    break;
                }
            }
            if (ap)
                break;
        }
    }
    if (!wirelessDevice || !ap)
        return;
    qCInfo(DNC) << "Connect wireless, device name: " << wirelessDevice->deviceName() << "access point ssid: " << ap->ssid();
    if (m_secretAgent && m_secretAgent->hasTask()) {
        QVariantMap errMap;
        for (QVariantMap::const_iterator it = param.constBegin(); it != param.constEnd(); ++it) {
            if (it.value().toString().isEmpty()) {
                errMap.insert(it.key(), QString());
            }
        }
        if (!errMap.isEmpty()) {
            sendRequest(NetManager::InputError, id, errMap);
            return;
        }
        m_secretAgent->inputPassword(ap->ssid(), param, true);
        sendRequest(NetManager::CloseInput, id);
        return;
    }
    NetWirelessConnect wConnect(wirelessDevice, ap, this);
    wConnect.setSsid(ap->ssid());
    wConnect.initConnection();
    QString secret = wConnect.needSecrets();

    if (param.contains(secret)) {
        QVariantMap err = wConnect.connectNetworkParam(param);
        if (err.isEmpty())
            sendRequest(NetManager::CloseInput, id);
        else
            sendRequest(NetManager::InputError, id, err);
    } else if (wConnect.needInputIdentify()) { // 未配置，需要输入Identify
        handle8021xAccessPoint(ap);
        if (m_network8021XMode != NetManager::ToConnect)
            sendRequest(NetManager::CloseInput, id);
    } else if (wConnect.needInputPassword()) {
        sendRequest(NetManager::RequestPassword, id, { { "secrets", { secret } } });
    } else {
        wConnect.connectNetwork();
        sendRequest(NetManager::CloseInput, id);
    }
}

void NetManagerThreadPrivate::doConnectHotspot(const QString &id, const QVariantMap &param, bool connect)
{
    auto hotspotController = NetworkController::instance()->hotspotController();
    QString uuid = param.value("uuid").toString();
    if (uuid.isEmpty()) {
        return;
    }
    for (auto dev : hotspotController->devices()) {
        for (auto item : hotspotController->items(dev)) {
            if (item->connection()->uuid() == uuid) {
                if (connect) {
                    if (item->status() != ConnectionStatus::Activated && item->status() != ConnectionStatus::Activating) {
                        hotspotController->connectItem(item);
                    }
                } else {
                    if (item->status() == ConnectionStatus::Activated || item->status() == ConnectionStatus::Activating) {
                        hotspotController->disconnectItem(dev);
                    }
                }
                break;
            }
        }
    }
}

void NetManagerThreadPrivate::doGotoControlCenter(const QString &page)
{
    if (!m_enabled)
        return;
    QDBusMessage message = QDBusMessage::createMethodCall("org.lingmo.ocean.ControlCenter1", "/org/lingmo/ocean/ControlCenter1", "org.lingmo.ocean.ControlCenter1", "ShowPage");
    message << "network" << page;
    QDBusConnection::sessionBus().asyncCall(message);
    Q_EMIT toControlCenter();
}

void NetManagerThreadPrivate::doGotoSecurityTools(const QString &page)
{
    if (!m_enabled)
        return;

    QDBusMessage message = QDBusMessage::createMethodCall("com.lingmo.defender.hmiscreen", "/com/lingmo/defender/hmiscreen", "com.lingmo.defender.hmiscreen", "ShowPage");
    message << "securitytools" << page;
    QDBusConnection::sessionBus().asyncCall(message);
}

void NetManagerThreadPrivate::doUserCancelRequest(const QString &id)
{
    if (id.isEmpty()) {
        m_secretAgent->inputPassword(QString(), {}, false);
        return;
    }
    // 暂只处理无线
    WirelessDevice *wirelessDevice = nullptr;
    AccessPoints *ap = nullptr;
    for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
        if (device->deviceType() != DeviceType::Wireless)
            continue;
        WirelessDevice *wirelessDev = qobject_cast<WirelessDevice *>(device);
        for (auto &&tmpAp : wirelessDev->accessPointItems()) {
            if (apID(tmpAp) == id) {
                wirelessDevice = wirelessDev;
                ap = tmpAp;
                break;
            }
        }
        if (ap)
            break;
    }
    if (!wirelessDevice || !ap)
        return;
    m_secretAgent->inputPassword(ap->ssid(), {}, false);
}

void NetManagerThreadPrivate::doRetranslate(const QString &locale)
{
    NetworkController::instance()->retranslate(locale);
}

void NetManagerThreadPrivate::updateNetCheckAvailabled(const QDBusVariant &availabled)
{
    if (m_netCheckAvailable != availabled.variant().toBool()) {
        m_netCheckAvailable = availabled.variant().toBool();
        Q_EMIT netCheckAvailableChanged(m_netCheckAvailable);
    }
}

void NetManagerThreadPrivate::updateAirplaneModeEnabled(const QDBusVariant &enabled)
{
    m_airplaneModeEnabled = enabled.variant().toBool() && supportAirplaneMode();
    Q_EMIT dataChanged(DataChanged::EnabledChanged, "Root", QVariant(m_airplaneModeEnabled));
}

void NetManagerThreadPrivate::updateAirplaneModeEnabledable(const QDBusVariant &enabledable)
{
    bool airplaneEnabledable = enabledable.variant().toBool();
    Q_EMIT dataChanged(DataChanged::DeviceAvailableChanged, "Root", QVariant(airplaneEnabledable));
}

bool NetManagerThreadPrivate::supportAirplaneMode() const
{
    // ocean-dconfig配置优先级高于设备优先级
    if (!ConfigSetting::instance()->networkAirplaneMode()) {
        return false;
    }

    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device : devices) {
        if (device->type() == NetworkManager::Device::Type::Wifi && device->managed())
            return true;
    }

    return false;
}

void NetManagerThreadPrivate::doConnectOrInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param)
{
    switch (type) {
    case NetType::WiredItem:
        doConnectWired(id, param);
        break;
    case NetType::WirelessItem: {
        AccessPoints *ap = fromApID(id);
        if (!ap) {
            qCWarning(DNC) << "not find AccessPoint";
            return;
        }
        QString devPath = ap->devicePath();
        NetworkManager::WirelessDevice *netDevice = qobject_cast<NetworkManager::WirelessDevice *>(NetworkManager::findNetworkInterface(devPath).get());
        if (!netDevice) {
            qCWarning(DNC) << "not find Device";
            return;
        }

        ConnectionSettings::Ptr settings;
        for (const NetworkManager::Connection::Ptr &con : netDevice->availableConnections()) {
            NetworkManager::WirelessSetting::Ptr wSetting = con->settings()->setting(NetworkManager::Setting::SettingType::Wireless).staticCast<NetworkManager::WirelessSetting>();
            if (wSetting->ssid() != ap->ssid()) {
                continue;
            }
            settings = con->settings();

            WirelessSecuritySetting::Ptr const sSetting = settings->setting(Setting::SettingType::WirelessSecurity).staticCast<WirelessSecuritySetting>();
            sSetting->secretsFromMap(con->secrets(sSetting->name()).value().value(sSetting->name()));
            QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::activateConnection(con->path(), devPath, ap->path());
            if (reply.isError()) {
                qCWarning(DNC) << "activateConnection fiald:" << reply.error().message();
            }
            break;
        }
        if (settings.isNull()) {
            AccessPoint::Ptr nmAp = netDevice->findAccessPoint(ap->path());
            if (nmAp.isNull()) {
                qCWarning(DNC) << "not find NetworkManager AccessPoint";
                return;
            }
            WirelessSecuritySetting::KeyMgmt keyMgmt = getKeyMgmtByAp(nmAp.get());
            if (keyMgmt == WirelessSecuritySetting::WpaEap) {
                doGetConnectInfo(id, type, param);
            } else {
                NetworkManager::ConnectionSettings::Ptr settings = NetworkManager::ConnectionSettings::Ptr(new ConnectionSettings(ConnectionSettings::Wireless));
                settings->setId(ap->ssid());
                NetworkManager::WirelessSetting::Ptr wSetting = settings->setting(Setting::SettingType::Wireless).staticCast<WirelessSetting>();
                wSetting->setSsid(ap->ssid().toUtf8());
                wSetting->setInitialized(true);
                WirelessSecuritySetting::Ptr wsSetting = settings->setting(Setting::WirelessSecurity).dynamicCast<WirelessSecuritySetting>();
                switch (keyMgmt) {
                case WirelessSecuritySetting::KeyMgmt::WpaNone:
                    break;
                case WirelessSecuritySetting::KeyMgmt::Wep:
                    wsSetting->setKeyMgmt(keyMgmt);
                    wsSetting->setWepKeyFlags(Setting::None);
                    break;
                case WirelessSecuritySetting::KeyMgmt::WpaPsk:
                case WirelessSecuritySetting::KeyMgmt::SAE:
                    wsSetting->setKeyMgmt(keyMgmt);
                    wsSetting->setPskFlags(Setting::None);
                    break;
                default:
                    wsSetting->setKeyMgmt(keyMgmt);
                    break;
                }
                wsSetting->setInitialized(true);
                QString uuid = settings->createNewUuid();
                while (findConnectionByUuid(uuid)) {
                    qint64 second = QDateTime::currentDateTime().toSecsSinceEpoch();
                    uuid.replace(24, QString::number(second).length(), QString::number(second));
                }
                settings->setUuid(uuid);
                QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> reply = NetworkManager::addAndActivateConnection(settings->toMap(), devPath, ap->path());
                if (reply.isError()) {
                    qCWarning(DNC) << "activateConnection fiald:" << reply.error().message();
                }
            }
        }

    } break;
    case NetType::ConnectionItem: {
        NetworkManager::Connection::Ptr conn = findConnection(id);
        if (conn) {
            QString devicePath;
            NetworkManager::isNetworkingEnabled();
            for (NetworkManager::Device::Ptr device : NetworkManager::networkInterfaces()) {
                NetworkManager::Connection::List connections = device->availableConnections();
                NetworkManager::Connection::List::iterator itConnection = std::find_if(connections.begin(), connections.end(), [conn](NetworkManager::Connection::Ptr connection) {
                    return connection->path() == conn->path();
                });
                if (itConnection != connections.end()) {
                    devicePath = device->uni();
                    break;
                }
            }
            QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::activateConnection(conn->path(), devicePath, QString());
            if (reply.isError()) {
                qCWarning(DNC) << "activateConnection fiald:" << reply.error().message();
            }
        }
    } break;
    default:
        doGetConnectInfo(id, type, param);
        break;
    }
}

void NetManagerThreadPrivate::doGetConnectInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param)
{
    switch (type) {
    case NetType::WiredDeviceItem: // 新建有线网络
        for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
            if (device->path() == id) {
                NetworkManager::ConnectionSettings::Ptr settings = NetworkManager::ConnectionSettings::Ptr(new ConnectionSettings(ConnectionSettings::Wired));
                QString connName = connectionSuffixNum(tr("Wired Connection %1"));
                Security8021xSetting::Ptr securitySettings = settings->setting(Setting::Security8021x).dynamicCast<Security8021xSetting>();
                // if (securitySettings) {
                //     securitySettings->setSupportCertifiedEscape(ocean::network::ConfigSetting::instance()->supportCertifiedEscape());
                // }
                settings->setId(connName);

                QVariantMap retParam;
                const NMVariantMapMap &settingsMap = settings->toMap();
                for (auto it = settingsMap.cbegin(); it != settingsMap.cend(); it++) {
                    retParam.insert(it.key(), it.value());
                }
                QString mac = device->realHwAdr();
                if (mac.isEmpty()) {
                    mac = device->usingHwAdr();
                }
                mac = mac + " (" + device->interface() + ")";
                QVariantMap typeMap = retParam[settingsMap["connection"]["type"].toString()].value<QVariantMap>();
                typeMap.insert("optionalDevice", QStringList(mac));
                retParam[settingsMap["connection"]["type"].toString()] = typeMap;

                Q_EMIT request(NetManager::ConnectInfo, id, retParam);
            }
        }
        break;
    case NetType::WiredItem: {
        QStringList ids = id.split(":");
        if (ids.size() != 2)
            return;
        for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
            if (device->path() == ids.first()) {
                WiredDevice *netDevice = qobject_cast<WiredDevice *>(device);
                for (auto &&conn : netDevice->items()) {
                    if (conn->connection() && conn->connection()->path() == ids.at(1)) {
                        qCInfo(DNC) << "ConnectInfo wired, device name: " << netDevice->deviceName() << "connection name: " << conn->connection()->id()
                                    << "connection uuid: " << conn->connection()->uuid();
                        auto connection = findConnectionByUuid(conn->connection()->uuid());
                        if (!connection) {
                            qCWarning(DNC) << "Can not find connection by uuid, uuid: " << conn->connection()->uuid();
                            return;
                        }
                        auto connectionSettings = connection->settings();
                        Setting::SettingType sType = Setting::SettingType::Security8021x;
                        QSharedPointer<Security8021xSetting> sSetting = connectionSettings->setting(sType).staticCast<Security8021xSetting>();
                        // if (!sSetting->eapMethods().isEmpty()) {
                        sSetting->secretsFromMap(connection->secrets(sSetting->name()).value().value(sSetting->name()));
                        // }

                        QVariantMap retParam;
                        const NMVariantMapMap &settingsMap = connectionSettings->toMap();
                        for (auto it = settingsMap.cbegin(); it != settingsMap.cend(); it++) {
                            retParam.insert(it.key(), it.value());
                        }
                        // 可选设备
                        QString mac = netDevice->realHwAdr();
                        if (mac.isEmpty()) {
                            mac = netDevice->usingHwAdr();
                        }
                        mac = mac + " (" + netDevice->interface() + ")";
                        QVariantMap typeMap = retParam[settingsMap["connection"]["type"].toString()].value<QVariantMap>();
                        typeMap.insert("optionalDevice", QStringList(mac));
                        retParam[settingsMap["connection"]["type"].toString()] = typeMap;

                        Ipv6Setting::Ptr ipv6 = connectionSettings->setting(Setting::Ipv6).dynamicCast<Ipv6Setting>();
                        connection->path();
                        if (ipv6->method() == Ipv6Setting::Manual) {
                            // ipv6 gateway未获取，自己获取下
                            auto msg = QDBusMessage::createMethodCall("org.freedesktop.NetworkManager", connection->path(), "org.freedesktop.NetworkManager.Settings.Connection", "GetSettings");
                            QDBusPendingReply<NMVariantMapMap> settingsMap = QDBusConnection::systemBus().call(msg, QDBus::Block, 100);
                            if (!settingsMap.isError() && settingsMap.value().contains("ipv6") && settingsMap.value().value("ipv6").contains("gateway")) {
                                QString gateway = settingsMap.value().value("ipv6").value("gateway").toString();
                                QVariantMap ipv6Map = retParam["ipv6"].value<QVariantMap>();
                                ipv6Map.insert("gateway", gateway);
                                retParam["ipv6"] = ipv6Map;
                            }
                        }

                        Q_EMIT request(NetManager::ConnectInfo, id, retParam);
                        break;
                    }
                }
                break;
            }
        }
    } break;
    case NetType::WirelessDeviceItem:
        break;
    case NetType::WirelessItem: {
        AccessPoints *ap = fromApID(id);
        if (!ap) {
            qCWarning(DNC) << "not find AccessPoint";
            return;
        }
        QString devPath = ap->devicePath();
        NetworkManager::WirelessDevice *netDevice = qobject_cast<NetworkManager::WirelessDevice *>(NetworkManager::findNetworkInterface(devPath).get());
        if (!netDevice) {
            qCWarning(DNC) << "not find Device";
            return;
        }

        ConnectionSettings::Ptr settings;
        for (const NetworkManager::Connection::Ptr &con : netDevice->availableConnections()) {
            NetworkManager::WirelessSetting::Ptr wSetting = con->settings()->setting(NetworkManager::Setting::SettingType::Wireless).staticCast<NetworkManager::WirelessSetting>();
            if (wSetting->ssid() != ap->ssid()) {
                continue;
            }
            settings = con->settings();

            WirelessSecuritySetting::Ptr const sSetting = settings->setting(Setting::SettingType::WirelessSecurity).staticCast<WirelessSecuritySetting>();
            sSetting->secretsFromMap(con->secrets(sSetting->name()).value().value(sSetting->name()));
            ///////////
            QVariantMap retParam;
            const NMVariantMapMap &settingsMap = settings->toMap();
            for (auto it = settingsMap.cbegin(); it != settingsMap.cend(); it++) {
                retParam.insert(it.key(), it.value());
            }
            // 可选设备
            QString mac = netDevice->permanentHardwareAddress().toUpper();
            if (mac.isEmpty()) {
                mac = netDevice->hardwareAddress().toUpper();
            }
            mac = mac + " (" + netDevice->interfaceName() + ")";
            QVariantMap typeMap = retParam[settingsMap["connection"]["type"].toString()].value<QVariantMap>();
            typeMap.insert("optionalDevice", QStringList(mac));
            retParam[settingsMap["connection"]["type"].toString()] = typeMap;

            Ipv6Setting::Ptr ipv6 = settings->setting(Setting::Ipv6).dynamicCast<Ipv6Setting>();
            if (ipv6->method() == Ipv6Setting::Manual) {
                // ipv6 gateway未获取，自己获取下
                QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.NetworkManager", con->path(), "org.freedesktop.NetworkManager.Settings.Connection", "GetSettings");
                QDBusPendingReply<NMVariantMapMap> settingsMap = QDBusConnection::systemBus().call(msg, QDBus::Block, 100);
                if (!settingsMap.isError() && settingsMap.value().contains("ipv6") && settingsMap.value().value("ipv6").contains("gateway")) {
                    QString gateway = settingsMap.value().value("ipv6").value("gateway").toString();
                    QVariantMap ipv6Map = retParam["ipv6"].value<QVariantMap>();
                    ipv6Map.insert("gateway", gateway);
                    retParam["ipv6"] = ipv6Map;
                }
            }

            Q_EMIT request(NetManager::ConnectInfo, id, retParam);
            break;
        }
        if (settings.isNull()) { // 新项
            AccessPoint::Ptr nmAp = netDevice->findAccessPoint(ap->path());
            if (nmAp.isNull()) {
                qCWarning(DNC) << "not find NetworkManager AccessPoint";
                return;
            }
            NetworkManager::ConnectionSettings::Ptr settings = NetworkManager::ConnectionSettings::Ptr(new ConnectionSettings(ConnectionSettings::Wireless));
            settings->setId(ap->ssid());
            NetworkManager::WirelessSetting::Ptr wSetting = settings->setting(Setting::SettingType::Wireless).staticCast<WirelessSetting>();
            wSetting->setSsid(ap->ssid().toUtf8());
            wSetting->setInitialized(true);
            WirelessSecuritySetting::Ptr wsSetting = settings->setting(Setting::WirelessSecurity).dynamicCast<WirelessSecuritySetting>();
            WirelessSecuritySetting::KeyMgmt keyMgmt = getKeyMgmtByAp(nmAp.get());
            switch (keyMgmt) {
            case WirelessSecuritySetting::KeyMgmt::WpaNone:
                break;
            case WirelessSecuritySetting::KeyMgmt::Wep:
                wsSetting->setKeyMgmt(keyMgmt);
                wsSetting->setWepKeyFlags(Setting::None);
                break;
            case WirelessSecuritySetting::KeyMgmt::WpaPsk:
            case WirelessSecuritySetting::KeyMgmt::SAE:
                wsSetting->setKeyMgmt(keyMgmt);
                wsSetting->setPskFlags(Setting::None);
                break;
            default:
                wsSetting->setKeyMgmt(keyMgmt);
                break;
            }
            wsSetting->setInitialized(true);
            QVariantMap retParam;
            const NMVariantMapMap &settingsMap = settings->toMap();
            for (auto it = settingsMap.cbegin(); it != settingsMap.cend(); it++) {
                retParam.insert(it.key(), it.value());
            }
            // 可选设备
            QString mac = netDevice->permanentHardwareAddress().toUpper();
            if (mac.isEmpty()) {
                mac = netDevice->hardwareAddress().toUpper();
            }
            mac = mac + " (" + netDevice->interfaceName() + ")";
            QVariantMap typeMap = retParam[settingsMap["connection"]["type"].toString()].value<QVariantMap>();
            typeMap.insert("optionalDevice", QStringList(mac));
            retParam[settingsMap["connection"]["type"].toString()] = typeMap;
            Q_EMIT request(NetManager::ConnectInfo, id, retParam);
        }
    } break;
    case NetType::WirelessHioceannItem: { // 新建无线网络/隐藏网络
        QStringList ids = id.split(":");
        if (ids.size() != 2) {
            return;
        }
        QString devPath = ids.first();
        NetworkManager::WirelessDevice *netDevice = qobject_cast<NetworkManager::WirelessDevice *>(NetworkManager::findNetworkInterface(devPath).get());
        if (!netDevice) {
            qCWarning(DNC) << "not find Device";
            return;
        }

        NetworkManager::ConnectionSettings::Ptr settings = NetworkManager::ConnectionSettings::Ptr(new ConnectionSettings(ConnectionSettings::Wireless));
        settings->setting(Setting::SettingType::Wireless).staticCast<WirelessSetting>()->setHioceann(true);
        settings->setting(Setting::SettingType::Wireless).staticCast<WirelessSetting>()->setInitialized(true);
        QVariantMap retParam;
        const NMVariantMapMap &settingsMap = settings->toMap();
        for (auto it = settingsMap.cbegin(); it != settingsMap.cend(); it++) {
            retParam.insert(it.key(), it.value());
        }
        // 可选设备
        QString mac = netDevice->permanentHardwareAddress().toUpper();
        if (mac.isEmpty()) {
            mac = netDevice->hardwareAddress().toUpper();
        }
        mac = mac + " (" + netDevice->interfaceName() + ")";
        QVariantMap typeMap = retParam[settingsMap["connection"]["type"].toString()].value<QVariantMap>();
        typeMap.insert("optionalDevice", QStringList(mac));
        retParam[settingsMap["connection"]["type"].toString()] = typeMap;
        Q_EMIT request(NetManager::ConnectInfo, id, retParam);
    } break;
    case NetType::VPNControlItem: { // 新建VPN
        NetworkManager::ConnectionSettings::Ptr settings = NetworkManager::ConnectionSettings::Ptr(new ConnectionSettings(ConnectionSettings::Vpn));
        VpnSetting::Ptr const vpnSetting = settings->setting(Setting::SettingType::Vpn).staticCast<VpnSetting>();
        vpnSetting->setServiceType("org.freedesktop.NetworkManager.l2tp");
        vpnSetting->setInitialized(true);

        QVariantMap names;
        names.insert("l2tp", connectionSuffixNum(tr("VPN L2TP %1")));
        names.insert("pptp", connectionSuffixNum(tr("VPN PPTP %1")));
        names.insert("vpnc", connectionSuffixNum(tr("VPN VPNC %1")));
        names.insert("openvpn", connectionSuffixNum(tr("VPN OpenVPN %1")));
        names.insert("strongswan", connectionSuffixNum(tr("VPN StrongSwan %1")));
        names.insert("openconnect", connectionSuffixNum(tr("VPN OpenConnect %1")));
        settings->setId(names.value("l2tp").toString());

        QVariantMap retParam;
        const NMVariantMapMap &settingsMap = settings->toMap();
        for (auto it = settingsMap.cbegin(); it != settingsMap.cend(); it++) {
            retParam.insert(it.key(), it.value());
        }
        // QString mac = device->realHwAdr();
        // if (mac.isEmpty()) {
        //     mac = device->usingHwAdr();
        // }
        // mac = mac + " (" + device->interface() + ")";
        // QVariantMap connMap = retParam["connection"].value<QVariantMap>();
        // retParam["connection"] = connMap;
        retParam.insert("optionalName", names);

        Q_EMIT request(NetManager::ConnectInfo, id, retParam);
    } break;
    case NetType::DSLControlItem: { // 新建DSL
        QString deviceKey("802-3-ethernet");
        QStringList optionalDevice;
        QVariantMap retParam;
        QString parent;
        QString parentMac;
        for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
            if (device->deviceType() == DeviceType::Wired) {
                QString mac = device->realHwAdr();
                if (mac.isEmpty()) {
                    mac = device->usingHwAdr();
                }
                if (parent.isEmpty()) {
                    parent = device->interface();
                    parentMac = mac;
                    parentMac.remove(":");
                }
                mac = mac + " (" + device->interface() + ")";
                optionalDevice.append(mac);
            }
        }
        if (optionalDevice.isEmpty()) {
            return;
        }
        NetworkManager::ConnectionSettings::Ptr settings = NetworkManager::ConnectionSettings::Ptr(new ConnectionSettings(ConnectionSettings::Pppoe));
        QString connName = connectionSuffixNum(tr("PPPoE Connection %1"));
        settings->setId(connName);

        WiredSetting::Ptr const wiredSetting = settings->setting(Setting::SettingType::Wired).staticCast<WiredSetting>();
        wiredSetting->setMacAddress(QByteArray::fromHex(parentMac.toUtf8()));
        wiredSetting->setInitialized(true);
        PppoeSetting::Ptr const pSetting = settings->setting(Setting::SettingType::Pppoe).staticCast<PppoeSetting>();
        pSetting->setParent(parent);
        pSetting->setInitialized(true);
        PppSetting::Ptr const pppSetting = settings->setting(Setting::SettingType::Ppp).staticCast<PppSetting>();
        pppSetting->setLcpEchoFailure(5);
        pppSetting->setLcpEchoInterval(30);
        pppSetting->setInitialized(true);
        const NMVariantMapMap &settingsMap = settings->toMap();
        for (auto it = settingsMap.cbegin(); it != settingsMap.cend(); it++) {
            retParam.insert(it.key(), it.value());
        }
        if (retParam.contains(deviceKey)) {
            QVariantMap typeMap = retParam[deviceKey].value<QVariantMap>();
            typeMap.insert("optionalDevice", optionalDevice);
            retParam[deviceKey] = typeMap;
        }
        Q_EMIT request(NetManager::ConnectInfo, id, retParam);
    } break;
    case NetType::ConnectionItem: {
        NetworkManager::Connection::Ptr conn = findConnection(id);
        auto settings = conn->settings();
        QStringList optionalDevice;
        QString deviceKey;
        switch (settings->connectionType()) {
        case ConnectionSettings::Pppoe: {
            deviceKey = "802-3-ethernet";
            PppoeSetting::Ptr const pSetting = settings->setting(Setting::SettingType::Pppoe).staticCast<PppoeSetting>();
            pSetting->secretsFromMap(conn->secrets(pSetting->name()).value().value(pSetting->name()));
            for (NetworkDeviceBase *device : NetworkController::instance()->devices()) {
                if (device->deviceType() == DeviceType::Wired) {
                    QString mac = device->realHwAdr();
                    if (mac.isEmpty()) {
                        mac = device->usingHwAdr();
                    }
                    mac = mac + " (" + device->interface() + ")";
                    optionalDevice.append(mac);
                }
            }
        } break;
        case ConnectionSettings::Vpn: {
            VpnSetting::Ptr const vpnSetting = settings->setting(Setting::SettingType::Vpn).staticCast<VpnSetting>();
            vpnSetting->secretsFromMap(conn->secrets(vpnSetting->name()).value().value(vpnSetting->name()));
        } break;
        case ConnectionSettings::Wired:
            deviceKey = "802-3-ethernet";
            break;
        default:
            break;
        }
        QVariantMap retParam;
        const NMVariantMapMap &settingsMap = settings->toMap();
        for (auto it = settingsMap.cbegin(); it != settingsMap.cend(); it++) {
            retParam.insert(it.key(), it.value());
        }
        if (retParam.contains(deviceKey)) {
            QVariantMap typeMap = retParam[deviceKey].value<QVariantMap>();
            typeMap.insert("optionalDevice", optionalDevice);
            retParam[deviceKey] = typeMap;
        }
        Q_EMIT request(NetManager::ConnectInfo, id, retParam);
    } break;
    case NetType::HotspotControlItem:
        break;
    default:

        break;
    }
}

void NetManagerThreadPrivate::doSetConnectInfo(const QString &id, NetType::NetItemType type, const QVariantMap &param)
{
    QString devPath = id;
    switch (type) {
    case NetType::WiredDeviceItem:
    case NetType::WirelessDeviceItem:
        devPath = id;
        break;
    case NetType::WiredItem:
    case NetType::WirelessHioceannItem:
        devPath = id.split(":").first();
        break;
    case NetType::WirelessItem: {
        AccessPoints *ap = fromApID(id);
        if (ap) {
            devPath = ap->devicePath();
        } else {
            return;
        }
    } break;
    case NetType::SystemProxyControlItem: {
        doSetSystemProxy(param);
        return;
    } break;
    case NetType::AppProxyControlItem: {
        doSetAppProxy(param);
        return;
    } break;
    case NetType::VPNControlItem:
    case NetType::HotspotControlItem: {
        devPath = "";
    } break;
    case NetType::AirplaneControlItem: {
        setAirplaneModeEnabled(param.value("enable").toBool());
        return;
    } break;
    case NetType::DSLControlItem:
    case NetType::ConnectionItem: {
        devPath = "";
    } break;
    default:
        return;
        break;
    }
    NMVariantMapMap map;
    for (auto it = param.cbegin(); it != param.cend(); it++) {
        if (it.value().canConvert(QMetaType(QMetaType::QVariantMap))) {
            map.insert(it.key(), it.value().toMap());
        }
    }
    NetworkManager::ConnectionSettings::Ptr settings = NetworkManager::ConnectionSettings::Ptr(new ConnectionSettings(map));
    // fromMap 不能设置parent，单独处理下
    if (settings->connectionType() == ConnectionSettings::Pppoe && map.contains("pppoe") && map["pppoe"].contains("parent")) {
        PppoeSetting::Ptr const pSetting = settings->setting(Setting::SettingType::Pppoe).staticCast<PppoeSetting>();
        pSetting->setParent(map["pppoe"]["parent"].toString());
        pSetting->setInitialized(true);
    }
    NetworkManager::Connection::Ptr connection;
    if (settings->uuid() == "{00000000-0000-0000-0000-000000000000}") {
        // 新增
        QString uuid = settings->createNewUuid();
        while (findConnectionByUuid(uuid)) {
            qint64 second = QDateTime::currentDateTime().toSecsSinceEpoch();
            uuid.replace(24, QString::number(second).length(), QString::number(second));
        }
        settings->setUuid(uuid);
        // 调用addConnection的情况如下
        // 1、当前连接是VPN或者DSL或者当前连接没有开启自动连接
        // 2、当前连接是有线网络，且当前网卡没有插入网线（没有插入网线的情况下，如果调用addAndActivateConnection就会添加连接失败）
        QDBusPendingReply<QDBusObjectPath> reply;
        bool usedAdd = false;
        switch (settings->connectionType()) {
        case ConnectionSettings::Vpn:
        case ConnectionSettings::Pppoe: {
            usedAdd = true;
        } break;
        case ConnectionSettings::Wired: {
            NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(devPath);
            if (device && device->type() == NetworkManager::Device::Type::Ethernet) {
                // 有线连接且当前网卡没有插入网线的情况下，只调用addConnection而不调用addAndActivateConnection
                // 否则会无法添加连接
                usedAdd = !device.staticCast<NetworkManager::WiredDevice>()->carrier();
            }
        } break;
        default:
            break;
        }
        if (!settings->autoconnect()) {
            usedAdd = true;
        }
        // settings->connectionType()
        // if (settings->connectionType() == ConnectionSettings::Vpn || settings->connectionType() == ConnectionSettings::Pppoe || !settings->autoconnect()) {
        //     usedAdd = true;
        // } else if (settings->connectionType() == ConnectionSettings::Wired) {
        //     NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(devPath);
        //     if (device && device->type() == NetworkManager::Device::Type::Ethernet) {
        //         // 有线连接且当前网卡没有插入网线的情况下，只调用addConnection而不调用addAndActivateConnection
        //         // 否则会无法添加连接
        //         usedAdd = !device.staticCast<NetworkManager::WiredDevice>()->carrier();
        //     }
        // }
        if (usedAdd || devPath.isEmpty()) {
            reply = NetworkManager::addConnection(settings->toMap());
        } else {
            // 其他场景
            reply = NetworkManager::addAndActivateConnection(settings->toMap(), devPath, QString());
        }
        reply.waitForFinished();
        const QString &connPath = reply.argumentAt(0).value<QDBusObjectPath>().path();
        connection = findConnection(connPath);
        if (!connection) {
            qCWarning(DNC) << "Add and activate connection failed, error: " << reply.error();
            return;
        }
    } else {
        // 更新
        connection = findConnectionByUuid(settings->uuid());
        QDBusPendingReply<> reply = connection->isUnsaved() ? connection->updateUnsaved(settings->toMap()) : connection->update(settings->toMap());
        reply.waitForFinished();
        if (reply.isError()) {
            qCWarning(DNC) << "Error occurred while updating the connection, error: " << reply.error();
            return;
        }
    }
    switch (type) {
    case NetType::HotspotControlItem:
        // case NetType::ConnectionItem:
        NetworkManager::activateConnection(connection->path(), devPath, QString());
        break;
    default:
        if (!id.isEmpty() && (settings->autoconnect() || ocean::network::ConfigSetting::instance()->enableAccountNetwork())) {
            switch (settings->connectionType()) {
            case ConnectionSettings::Pppoe:
            case ConnectionSettings::Wireless: {
                QVariantMap option;
                option.insert("flags", MANULCONNECTION);
                QDBusPendingReply<> reply = NetworkManager::activateConnection2(connection->path(), devPath, QString(), option);
                reply.waitForFinished();
            }
            // case ConnectionSettings::Wired:
            // case ConnectionSettings::Vpn:
            default:
                NetworkManager::activateConnection(connection->path(), devPath, QString());
                break;
            }
        }
        break;
    }
}

void NetManagerThreadPrivate::doDeleteConnect(const QString &uuid)
{
    NetworkManager::Connection::Ptr conn = findConnectionByUuid(uuid);
    if (conn) {
        conn->remove();
    }
}

void NetManagerThreadPrivate::changeVpnId()
{
    if (m_newVPNuuid.isEmpty()) {
        return;
    }
    NetworkManager::Connection::Ptr uuidConn = findConnectionByUuid(m_newVPNuuid);
    if (uuidConn) {
        ConnectionSettings::Ptr connSettings = uuidConn->settings();
        QString vpnName = connectionSuffixNum(connSettings->id() + "(%1)");
        if (vpnName.isEmpty()) {
            m_newVPNuuid.clear();
            return;
        }
        connSettings->setId(vpnName);
        QDBusPendingReply<> reply = uuidConn->update(connSettings->toMap());
        reply.waitForFinished();
        if (reply.isError()) {
            qCWarning(DNC) << "Error occurred while updating the connection, error: " << reply.error();
            return;
        }
        qCInfo(DNC) << "Find connection by uuid successed";
        m_newVPNuuid.clear();
    }
}

QString vpnConfigType(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QString();

    const QString content = f.readAll();
    f.close();

    if (content.contains("openconnect"))
        return "openconnect";
    if (content.contains("l2tp"))
        return "l2tp";
    if (content.startsWith("[main]"))
        return "vpnc";

    return "openvpn";
}

void NetManagerThreadPrivate::doImportConnect(const QString &id, const QString &file)
{
    QFileInfo fInfo(file);
    const auto args = QStringList{ "connection", "import", "type", vpnConfigType(file), "file", file };
    QProcess p;
    p.setWorkingDirectory(fInfo.absolutePath());
    p.start("nmcli", args);
    p.waitForFinished();
    const auto stat = p.exitCode();
    const QString output = p.readAllStandardOutput();
    QString error = p.readAllStandardError();
    qCDebug(DNC) << "Import VPN, process exit code: " << stat << ", output:" << output << ", error: " << error;
    if (stat) {
        Q_EMIT request(NetManager::ImportError, id, { { "file", file } });
    } else {
        const QRegularExpression regexp(R"(\((\w{8}(-\w{4}){3}-\w{12})\))");
        const auto match = regexp.match(output);
        if (match.hasCaptured(1)) {
            m_newVPNuuid = match.captured(1);
            changeVpnId();
        }
    }
}

void NetManagerThreadPrivate::doExportConnect(const QString &id, const QString &file)
{
    QString exportFile(file);
    if (!exportFile.endsWith(".conf")) {
        exportFile.append(".conf");
    }
    NetworkManager::Connection::Ptr conn = findConnection(id);
    if (conn.isNull()) {
        return;
    }
    const QStringList args = { "connection", "export", conn->uuid(), exportFile };
    QProcess p;
    p.start("nmcli", args);
    p.waitForFinished();
    qCDebug(DNC) << "Save config finished, process output: " << p.readAllStandardOutput();
    qCWarning(DNC) << "Save config finished, process error: " << p.readAllStandardError();
    // 写ca文件
    QFile f(exportFile);
    f.open(QIODevice::ReadWrite);
    const QString data = f.readAll();
    f.seek(0);
    const QRegularExpression regex("^(?:ca\\s'(.+)'\\s*)$");
    QStringList ca_list;
    for (const auto &line : data.split('\n')) {
        const auto match = regex.match(line);
        if (match.hasMatch()) {
            for (int i = 1; i != match.capturedLength(); ++i) {
                const auto cap = match.captured(i);
                if (cap.isNull() || cap.isEmpty())
                    continue;
                ca_list << cap;
            }
        } else {
            f.write(line.toStdString().c_str());
            f.write("\n");
        }
    }
    f.write("\n");

    if (!ca_list.isEmpty()) {
        // write ca
        f.write("<ca>\n");
        for (const auto &ca : ca_list) {
            QFile caf(ca);
            caf.open(QIODevice::ReadOnly);
            f.write(caf.readAll());
            f.write("\n");
        }
        f.write("</ca>\n");
    }

    f.flush();
    f.close();
}

void NetManagerThreadPrivate::doSetSystemProxy(const QVariantMap &param)
{
    bool ok;
    int method = param.value("method").toInt(&ok);
    if (!ok) {
        return;
    }
    ProxyController *controller = NetworkController::instance()->proxyController();
    switch (method) {
    case NetType::None: {
        controller->setProxyMethod(ProxyMethod::None);
    } break;
    case NetType::Auto: {
        QString autoUrl = param.value("autoUrl").toString();
        if (!autoUrl.isEmpty()) {
            controller->setAutoProxy(autoUrl);
            controller->setProxyMethod(ProxyMethod::Auto);
            // 保存上一次设置的代理类型
            ConfigWatcher::instance()->setProxyMethod(ProxyMethod::Auto);
        }
    } break;
    case NetType::Manual: {
        controller->setProxy(SysProxyType::Http, param.value("httpAddr").toString(), param.value("httpPort").toString());
        controller->setProxyAuth(SysProxyType::Http, param.value("httpUser").toString(), param.value("httpPassword").toString(), param.value("httpAuth").toBool());
        controller->setProxy(SysProxyType::Https, param.value("httpsAddr").toString(), param.value("httpsPort").toString());
        controller->setProxyAuth(SysProxyType::Https, param.value("httpsUser").toString(), param.value("httpsPassword").toString(), param.value("httpsAuth").toBool());
        controller->setProxy(SysProxyType::Ftp, param.value("ftpAddr").toString(), param.value("ftpPort").toString());
        controller->setProxyAuth(SysProxyType::Ftp, param.value("ftpUser").toString(), param.value("ftpPassword").toString(), param.value("ftpAuth").toBool());
        controller->setProxy(SysProxyType::Socks, param.value("socksAddr").toString(), param.value("socksPort").toString());
        controller->setProxyAuth(SysProxyType::Socks, param.value("socksUser").toString(), param.value("socksPassword").toString(), param.value("socksAuth").toBool());
        controller->setProxyIgnoreHosts(param.value("ignoreHosts").toString());
        controller->setProxyMethod(ProxyMethod::Manual);
        // 保存上一次设置的代理类型
        ConfigWatcher::instance()->setProxyMethod(ProxyMethod::Manual);
    } break;
    default:
        break;
    }
}

void NetManagerThreadPrivate::doSetAppProxy(const QVariantMap &param)
{
    if (!param.contains("enable")) {
        return;
    }
    ProxyController *controller = NetworkController::instance()->proxyController();
    if (param.value("enable").toBool()) {
        AppProxyConfig config;
        const QMap<QString, AppProxyType> cProxyTypeMap = {
            { "http", AppProxyType::Http },
            { "socks4", AppProxyType::Socks4 },
            { "socks5", AppProxyType::Socks5 },
        };
        config.type = cProxyTypeMap.value(param.value("type").toString());
        config.ip = param.value("url").toString();
        config.port = param.value("port").toUInt();
        config.username = param.value("user").toString();
        config.password = param.value("password").toString();
        controller->setAppProxy(config);
        controller->setAppProxyEnabled(true);
    } else {
        controller->setAppProxyEnabled(false);
    }
}

void NetManagerThreadPrivate::clearShowPageCmd()
{
    m_showPageCmd.clear();
    if (m_showPageTimer) {
        m_showPageTimer->stop();
        m_showPageTimer->deleteLater();
        m_showPageTimer = nullptr;
    }
}

bool NetManagerThreadPrivate::toShowPage()
{
    QMap<QString, QString> paramMap;
    QStringList params = m_showPageCmd.split('&');
    for (auto param : params) {
        auto keyMap = param.split('=');
        if (keyMap.size() != 2) {
            continue;
        }
        paramMap.insert(keyMap.at(0), keyMap.at(1));
    }
    if (paramMap.contains("page")) {
        QString page = paramMap.value("page");
        DeviceType type = DeviceType::Unknown;
        if (page == "wired") {
            type = DeviceType::Wired;
        } else if (page == "wireless") {
            type = DeviceType::Wireless;
        }
        if (type != DeviceType::Unknown) {
            QString index;
            for (auto dev : NetworkController::instance()->devices()) {
                if (dev->deviceType() == type) {
                    qsizetype i = dev->path().lastIndexOf('/');
                    if (i > 0) {
                        index = dev->path().mid(i + 1);
                    }
                }
            }
            if (index.isEmpty()) {
                return false;
            }
            page += index;
        }
        QDBusMessage message = QDBusMessage::createMethodCall("org.lingmo.ocean.ControlCenter1", "/org/lingmo/ocean/ControlCenter1", "org.lingmo.ocean.ControlCenter1", "ShowPage");
        message << page;
        QDBusConnection::sessionBus().asyncCall(message);
        clearShowPageCmd();
        return true;
    } else if (paramMap.contains("ssid")) {
        QString devPath = paramMap.value("device");
        QString ssid = paramMap.value("ssid");
        if (ssid.isEmpty()) {
            clearShowPageCmd();
            return true;
        }
        for (auto dev : NetworkController::instance()->devices()) {
            if (dev->deviceType() == DeviceType::Wireless && (devPath.isEmpty() || dev->path() == devPath)) {
                WirelessDevice *wDev = qobject_cast<WirelessDevice *>(dev);
                if (!wDev) {
                    continue;
                }
                for (auto ap : wDev->accessPointItems()) {
                    if (ap->ssid() == ssid) {
                        doGetConnectInfo(apID(ap), NetType::WirelessItem, QVariantMap());
                        QDBusMessage message = QDBusMessage::createMethodCall("org.lingmo.ocean.ControlCenter1", "/org/lingmo/ocean/ControlCenter1", "org.lingmo.ocean.ControlCenter1", "Show");
                        QDBusConnection::sessionBus().asyncCall(message);
                        clearShowPageCmd();
                        return true;
                    }
                }
            }
        }
    } else {
        clearShowPageCmd();
        return true;
    }
    return false;
}

void NetManagerThreadPrivate::doShowPage(const QString &cmd)
{
    if (m_showPageCmd != cmd) {
        m_showPageCmd = cmd;
        if (!toShowPage()) {
            if (!m_showPageTimer) {
                m_showPageTimer = new QTimer(this);
                connect(m_showPageTimer, &QTimer::timeout, this, &NetManagerThreadPrivate::toShowPage);
                QTimer::singleShot(10000, this, &NetManagerThreadPrivate::clearShowPageCmd);
                m_showPageTimer->start(100);
            }
        }
    }
}

void NetManagerThreadPrivate::sendRequest(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    if (!m_enabled)
        return;
    Q_EMIT request(cmd, id, param);
}

void NetManagerThreadPrivate::onDeviceAoceand(QList<NetworkDeviceBase *> devices)
{
    for (NetworkDeviceBase *device : devices) {
        qCInfo(DNC) << "On device aoceand, new device:" << device->deviceName();
        switch (device->deviceType()) {
        case DeviceType::Wired: {
            WiredDevice *wiredDevice = static_cast<WiredDevice *>(device);
            NetWiredDeviceItemPrivate *wiredDeviceItem = NetItemNew(WiredDeviceItem, wiredDevice->path());
            addDevice(wiredDeviceItem, wiredDevice);
            wiredDeviceItem->item()->moveToThread(m_parentThread);
            Q_EMIT itemAoceand("Root", wiredDeviceItem);
            addConnection(wiredDevice, wiredDevice->items());
            connect(wiredDevice, &WiredDevice::connectionAoceand, this, &NetManagerThreadPrivate::onConnectionAoceand);
            connect(wiredDevice, &WiredDevice::connectionRemoved, this, &NetManagerThreadPrivate::onConnectionRemoved);
            connect(wiredDevice, &WiredDevice::carrierChanged, this, &NetManagerThreadPrivate::onDeviceStatusChanged);
        } break;
        case DeviceType::Wireless: {
            WirelessDevice *wirelessDevice = static_cast<WirelessDevice *>(device);
            NetWirelessDeviceItemPrivate *wirelessDeviceItem = NetItemNew(WirelessDeviceItem, wirelessDevice->path());
            addDevice(wirelessDeviceItem, wirelessDevice);
            wirelessDeviceItem->updateapMode(wirelessDevice->hotspotEnabled());
            wirelessDeviceItem->item()->moveToThread(m_parentThread);
            Q_EMIT itemAoceand("Root", wirelessDeviceItem);
            getAirplaneModeEnabled();
            addNetwork(wirelessDevice, wirelessDevice->accessPointItems());
            connect(wirelessDevice, &WirelessDevice::networkAoceand, this, &NetManagerThreadPrivate::onNetworkAoceand);
            connect(wirelessDevice, &WirelessDevice::networkRemoved, this, &NetManagerThreadPrivate::onNetworkRemoved);
            connect(wirelessDevice, &WirelessDevice::hotspotEnableChanged, this, &NetManagerThreadPrivate::onHotspotEnabledChanged);
            connect(wirelessDevice, &WirelessDevice::wirelessConnectionAoceand, this, &NetManagerThreadPrivate::onAvailableConnectionsChanged);
            connect(wirelessDevice, &WirelessDevice::wirelessConnectionRemoved, this, &NetManagerThreadPrivate::onAvailableConnectionsChanged);
            connect(wirelessDevice, &WirelessDevice::wirelessConnectionPropertyChanged, this, &NetManagerThreadPrivate::onAvailableConnectionsChanged);
        } break;
        default:
            break;
        }
    }
}

void NetManagerThreadPrivate::onDeviceRemoved(QList<NetworkDeviceBase *> devices)
{
    for (auto &device : devices) {
        Q_EMIT itemRemoved(device->path());
    }
    getAirplaneModeEnabled();
    if (m_flags.testFlags(NetType::Net_Details)) {
        updateDetails();
    }
}

void NetManagerThreadPrivate::onConnectivityChanged()
{
    for (auto &&dev : NetworkController::instance()->devices())
        Q_EMIT dataChanged(DataChanged::DeviceStatusChanged, dev->path(), QVariant::fromValue(deviceStatus(dev)));
}

void NetManagerThreadPrivate::onConnectionAoceand(const QList<WiredConnection *> &conns)
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;
    addConnection(dev, conns);
}

void NetManagerThreadPrivate::onConnectionRemoved(const QList<WiredConnection *> &conns)
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;
    for (auto &conn : conns) {
        Q_EMIT itemRemoved(dev->path() + ":" + conn->connection()->path());
    }
}

void NetManagerThreadPrivate::addConnection(const NetworkDeviceBase *device, const QList<WiredConnection *> &conns)
{
    for (auto &conn : conns) {
        NetWiredItemPrivate *item = NetItemNew(WiredItem, device->path() + ":" + conn->connection()->path()); // new NetWiredItem(device->path() + ":" + conn->connection()->path());
        connect(conn, &WiredConnection::connectionChanged, this, &NetManagerThreadPrivate::onConnectionChanged);
        item->updatename(conn->connection()->id());
        item->updatestatus(toNetConnectionStatus(conn->status()));
        item->item()->moveToThread(m_parentThread);
        Q_EMIT itemAoceand(device->path(), item);
    }
}

void NetManagerThreadPrivate::onConnectionChanged()
{
    WiredConnection *conn = qobject_cast<WiredConnection *>(sender());
    QString devPath;
    if (conn) {
        for (auto dev : NetworkController::instance()->devices()) {
            if (!devPath.isEmpty()) {
                break;
            }
            if (dev->deviceType() == DeviceType::Wired) {
                WiredDevice *wDev = qobject_cast<WiredDevice *>(dev);
                for (auto tmpConn : wDev->items()) {
                    if (tmpConn == conn) {
                        devPath = wDev->path();
                        break;
                    }
                }
            }
        }

        Q_EMIT dataChanged(DataChanged::NameChanged, devPath + ":" + conn->connection()->path(), conn->connection()->id());
    }
}

void NetManagerThreadPrivate::onNetworkAoceand(const QList<AccessPoints *> &aps)
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;
    addNetwork(dev, aps);
}

void NetManagerThreadPrivate::onNetworkRemoved(const QList<AccessPoints *> &aps)
{
    for (auto &ap : aps) {
        Q_EMIT itemRemoved(apID(ap));
    }
}

void NetManagerThreadPrivate::addNetwork(const NetworkDeviceBase *device, QList<AccessPoints *> aps)
{
    QString path = device->path();
    NetworkManager::Device::Ptr dev = NetworkManager::findNetworkInterface(path);
    if (dev.isNull()) {
        dev.reset(new NetworkManager::Device(path));
    }
    NetworkManager::Connection::List connections = dev->availableConnections();

    for (auto &ap : aps) {
        NetWirelessItemPrivate *item = NetItemNew(WirelessItem, apID(ap)); // ap->path());
        item->updatename(ap->ssid());
        item->updateflags(static_cast<uint>(ap->type()));
        item->updatestrength(ap->strength());
        item->updatesecure(ap->secured());
        item->updatestatus(toNetConnectionStatus(ap->status()));
        item->item()->moveToThread(m_parentThread);

        NetworkManager::Connection::List::iterator itConnection = std::find_if(connections.begin(), connections.end(), [ap](NetworkManager::Connection::Ptr connection) {
            NetworkManager::WirelessSetting::Ptr wirelessSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
            if (wirelessSetting.isNull())
                return false;
            return wirelessSetting->ssid() == ap->ssid() && !connection->isUnsaved();
        });
        item->updatehasConnection(itConnection != connections.end());
        Q_EMIT itemAoceand(device->path(), item);
        connect(ap, &AccessPoints::strengthChanged, this, &NetManagerThreadPrivate::onStrengthChanged);
        connect(ap, &AccessPoints::connectionStatusChanged, this, &NetManagerThreadPrivate::onAPStatusChanged);
        connect(ap, &AccessPoints::securedChanged, this, &NetManagerThreadPrivate::onAPSecureChanged);
    }
}

void NetManagerThreadPrivate::onNameChanged(const QString &name)
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;
    Q_EMIT dataChanged(DataChanged::NameChanged, dev->path(), name);
}

void NetManagerThreadPrivate::onDevEnabledChanged(const bool enabled)
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;
    Q_EMIT dataChanged(DataChanged::EnabledChanged, dev->path(), dev->available() && enabled);
    Q_EMIT dataChanged(DataChanged::DeviceAvailableChanged, dev->path(), dev->available());
}

void NetManagerThreadPrivate::onDevAvailableChanged(const bool available)
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;
    Q_EMIT dataChanged(DataChanged::EnabledChanged, dev->path(), available && dev->isEnabled());
    Q_EMIT dataChanged(DataChanged::DeviceAvailableChanged, dev->path(), available);
}

void NetManagerThreadPrivate::onActiveConnectionChanged()
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;

    switch (dev->deviceType()) {
    case DeviceType::Wired: {
        WiredDevice *wiredDev = qobject_cast<WiredDevice *>(dev);
        if (!wiredDev)
            return;
        for (auto &&conn : wiredDev->items()) {
            Q_EMIT dataChanged(DataChanged::ConnectionStatusChanged, wiredDev->path() + ":" + conn->connection()->path(), QVariant::fromValue(toNetConnectionStatus(conn->status())));
        }
    } break;
    case DeviceType::Wireless:
        updateHioceannNetworkConfig(qobject_cast<WirelessDevice *>(dev));
        break;
    default:
        break;
    }
    if (m_flags.testFlags(NetType::Net_Details)) {
        updateDetails();
    }
}

void NetManagerThreadPrivate::onIpV4Changed()
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;
    Q_EMIT dataChanged(DataChanged::IPChanged, dev->path(), QVariant::fromValue(dev->ipv4()));
    if (m_flags.testFlags(NetType::Net_Details)) {
        updateDetails();
    }
}

void NetManagerThreadPrivate::onDeviceStatusChanged()
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;
    Q_EMIT dataChanged(DataChanged::DeviceStatusChanged, dev->path(), QVariant::fromValue(deviceStatus(dev)));
    if (m_flags.testFlags(NetType::Net_Details)) {
        updateDetails();
    }
}

void NetManagerThreadPrivate::onHotspotEnabledChanged()
{
    WirelessDevice *dev = qobject_cast<WirelessDevice *>(sender());
    if (!dev)
        return;
    Q_EMIT dataChanged(DataChanged::HotspotEnabledChanged, dev->path(), dev->hotspotEnabled());
}

void NetManagerThreadPrivate::onAvailableConnectionsChanged()
{
    QPointer<WirelessDevice> dev = qobject_cast<WirelessDevice *>(sender());
    if (!dev)
        return;
    // 因为在实际情况中，ConnectionsChanged信号和ActiveConnectionsChanged信号发出的前后顺序
    // 可能会乱，先收到AvailableConnectionsChanged，后又收到WirelessStatusChanged，导致本该移除的item又加回来了
    // TODO: 临时方案，暂时没有更好的方案
    QTimer::singleShot(200, this, [=] {
        if (!dev)
            return;
        QList<QString> availableAccessPoints;
        NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(dev->path());
        if (device.isNull()) {
            device.reset(new NetworkManager::Device(dev->path()));
        }
        NetworkManager::Connection::List connections = device->availableConnections();
        for (auto &&tmpAp : dev->accessPointItems()) {
            NetworkManager::Connection::List::iterator itConnection = std::find_if(connections.begin(), connections.end(), [tmpAp](NetworkManager::Connection::Ptr connection) {
                NetworkManager::WirelessSetting::Ptr wirelessSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
                if (wirelessSetting.isNull())
                    return false;
                return wirelessSetting->ssid() == tmpAp->ssid() && !connection->isUnsaved();
            });
            if (itConnection != connections.end()) {
                availableAccessPoints.append(apID(tmpAp));
            }
        }
        Q_EMIT dataChanged(DataChanged::AvailableConnectionsChanged, dev->path(), QVariant(availableAccessPoints));
    });
}

void NetManagerThreadPrivate::onStrengthChanged(int strength)
{
    AccessPoints *ap = qobject_cast<AccessPoints *>(sender());
    if (!ap)
        return;
    Q_EMIT dataChanged(DataChanged::StrengthChanged, apID(ap), strength);
}

void NetManagerThreadPrivate::onAPStatusChanged(ConnectionStatus status)
{
    AccessPoints *ap = qobject_cast<AccessPoints *>(sender());
    if (!ap)
        return;
    Q_EMIT dataChanged(DataChanged::WirelessStatusChanged, apID(ap), QVariant::fromValue(toNetConnectionStatus(status)));
}

void NetManagerThreadPrivate::onAPSecureChanged(bool secure)
{
    AccessPoints *ap = qobject_cast<AccessPoints *>(sender());
    if (!ap)
        return;
    Q_EMIT dataChanged(DataChanged::SecuredChanged, apID(ap), secure);

    handleAccessPointSecure(ap);
}

void NetManagerThreadPrivate::onVPNAoceand(const QList<VPNItem *> &vpns)
{
    changeVpnId();
    for (auto &&item : vpns) {
        NetConnectionItemPrivate *connItem = NetItemNew(ConnectionItem, item->connection()->path());
        connect(item, &VPNItem::connectionChanged, this, &NetManagerThreadPrivate::onVPNConnectionChanged, Qt::UniqueConnection);
        connItem->updatename(item->connection()->id());
        connItem->updatestatus(toNetConnectionStatus(item->status()));
        connItem->item()->moveToThread(m_parentThread);
        Q_EMIT itemAoceand("NetVPNControlItem", connItem);
    }
}

void NetManagerThreadPrivate::onVPNRemoved(const QList<VPNItem *> &vpns)
{
    for (auto &&item : vpns) {
        Q_EMIT itemRemoved(item->connection()->path());
    }
}

void NetManagerThreadPrivate::onVPNEnableChanged(const bool enable)
{
    Q_EMIT dataChanged(DataChanged::EnabledChanged, "NetVPNControlItem", enable);
}

void NetManagerThreadPrivate::onVpnActiveConnectionChanged()
{
    VPNController *vpnController = qobject_cast<VPNController *>(sender());
    if (!vpnController) {
        return;
    }
    for (auto &&conn : vpnController->items()) {
        Q_EMIT dataChanged(DataChanged::ConnectionStatusChanged, conn->connection()->path(), QVariant::fromValue(toNetConnectionStatus(conn->status())));
    }
    if (m_flags.testFlags(NetType::Net_Details)) {
        updateDetails();
    }
}

void NetManagerThreadPrivate::onVPNConnectionChanged()
{
    VPNItem *vpnItem = qobject_cast<VPNItem *>(sender());
    if (!vpnItem) {
        return;
    }
    Q_EMIT dataChanged(DataChanged::NameChanged, vpnItem->connection()->path(), QVariant::fromValue(vpnItem->connection()->id()));
}

void NetManagerThreadPrivate::onSystemProxyExistChanged(bool exist)
{
    Q_EMIT dataChanged(DataChanged::DeviceAvailableChanged, "NetSystemProxyControlItem", exist);
}

void NetManagerThreadPrivate::onSystemProxyMethodChanged(const ProxyMethod &method)
{
    Q_EMIT dataChanged(DataChanged::EnabledChanged, "NetSystemProxyControlItem", (method == ProxyMethod::Auto || method == ProxyMethod::Manual));
    Q_EMIT dataChanged(DataChanged::ProxyMethodChanged, "NetSystemProxyControlItem", QVariant::fromValue(NetType::ProxyMethod(method)));
}

void NetManagerThreadPrivate::onSystemAutoProxyChanged(const QString &url)
{
    Q_EMIT dataChanged(DataChanged::SystemAutoProxyChanged, "NetSystemProxyControlItem", url);
}

void NetManagerThreadPrivate::onSystemManualProxyChanged()
{
    auto controller = NetworkController::instance()->proxyController();
    QVariantMap config;
    static const QList<QPair<SysProxyType, QString>> types{ { SysProxyType::Http, "http" }, { SysProxyType::Https, "https" }, { SysProxyType::Ftp, "ftp" }, { SysProxyType::Socks, "socks" } };
    for (auto &&type : types) {
        QVariantMap proxyConfig;
        const SysProxyConfig &proxy = controller->proxy(type.first);
        proxyConfig.insert("type", type.second);
        proxyConfig.insert("url", proxy.url);
        proxyConfig.insert("port", proxy.port);
        proxyConfig.insert("auth", proxy.enableAuth);
        proxyConfig.insert("user", proxy.userName);
        proxyConfig.insert("password", proxy.password);
        config.insert(type.second, proxyConfig);
    }
    config.insert("ignoreHosts", controller->proxyIgnoreHosts());

    Q_EMIT dataChanged(DataChanged::SystemManualProxyChanged, "NetSystemProxyControlItem", config);
}

void NetManagerThreadPrivate::onAppProxyEnableChanged(bool enabled)
{
    Q_EMIT dataChanged(DataChanged::EnabledChanged, "NetAppProxyControlItem", enabled);
}

void NetManagerThreadPrivate::onAppProxyChanged()
{
    QVariantMap config;
    auto controller = NetworkController::instance()->proxyController();
    const AppProxyConfig &proxy = controller->appProxy();
    static const QMap<AppProxyType, QString> typeMap{ { AppProxyType::Http, "http" }, { AppProxyType::Socks4, "socks4" }, { AppProxyType::Socks5, "socks5" } };
    config.insert("type", typeMap.value(proxy.type));
    config.insert("url", proxy.ip);
    config.insert("port", proxy.port);
    config.insert("auth", true);
    config.insert("user", proxy.username);
    config.insert("password", proxy.password);
    Q_EMIT dataChanged(DataChanged::AppProxyChanged, "NetAppProxyControlItem", config);
}

void NetManagerThreadPrivate::updateHotspotEnabledChanged(const bool enabled)
{
    Q_EMIT dataChanged(DataChanged::EnabledChanged, "NetHotspotControlItem", enabled);
}

void NetManagerThreadPrivate::onHotspotEnabledableChanged(const bool enabledable)
{
    Q_EMIT dataChanged(DataChanged::DeviceAvailableChanged, "NetHotspotControlItem", enabledable);
}

void NetManagerThreadPrivate::onHotspotConfigChanged(const QVariantMap &config)
{
    Q_EMIT dataChanged(DataChanged::HotspotConfigChanged, "NetHotspotControlItem", config);
}

void NetManagerThreadPrivate::onHotspotOptionalDeviceChanged(const QStringList &optionalDevice)
{
    Q_EMIT dataChanged(DataChanged::HotspotOptionalDeviceChanged, "NetHotspotControlItem", optionalDevice);
}

void NetManagerThreadPrivate::onHotspotShareDeviceChanged(const QStringList &shareDevice)
{
    Q_EMIT dataChanged(DataChanged::HotspotShareDeviceChanged, "NetHotspotControlItem", shareDevice);
}

void NetManagerThreadPrivate::onDSLAoceand(const QList<DSLItem *> &dsls)
{
    for (auto &&item : dsls) {
        NetConnectionItemPrivate *connItem = NetItemNew(ConnectionItem, item->connection()->path());
        connItem->updatename(item->connection()->id());
        connItem->updatestatus(toNetConnectionStatus(item->status()));
        connItem->item()->moveToThread(m_parentThread);
        Q_EMIT itemAoceand("NetDSLControlItem", connItem);
    }
}

void NetManagerThreadPrivate::onDSLRemoved(const QList<DSLItem *> &dsls)
{
    for (auto &&item : dsls) {
        NetConnectionItemPrivate *connItem = NetItemNew(ConnectionItem, item->connection()->path());
        connItem->updatename(item->connection()->id());
        connItem->updatestatus(toNetConnectionStatus(item->status()));
        connItem->item()->moveToThread(m_parentThread);
        Q_EMIT itemAoceand("NetDSLControlItem", connItem);
    }
}

void NetManagerThreadPrivate::onDslActiveConnectionChanged()
{
    DSLController *controller = qobject_cast<DSLController *>(sender());
    if (!controller) {
        return;
    }
    for (auto &&conn : controller->items()) {
        Q_EMIT dataChanged(DataChanged::ConnectionStatusChanged, conn->connection()->path(), QVariant::fromValue(toNetConnectionStatus(conn->status())));
    }
    if (m_flags.testFlags(NetType::Net_Details)) {
        updateDetails();
    }
}

void NetManagerThreadPrivate::updateDetails()
{
    QSet<QString> detailsItems = m_detailsItems;
    int index = 0;
    for (auto &&details : NetworkController::instance()->networkDetails()) {
        if (detailsItems.contains(details->name())) {
            detailsItems.remove(details->name());
        } else {
            m_detailsItems.insert(details->name());
            connect(details, &NetworkDetails::infoChanged, this, &NetManagerThreadPrivate::updateDetails, Qt::QueuedConnection);
            NetDetailsInfoItemPrivate *item = NetItemNew(DetailsInfoItem, details->name());
            item->updatename(details->name());
            item->updateindex(index);
            item->item()->moveToThread(m_parentThread);
            Q_EMIT itemAoceand("Details", item);
        }
        QList<QStringList> data;
        for (auto &&info : details->items()) {
            data.append({ info.first, info.second });
        }
        Q_EMIT dataChanged(DataChanged::DetailsChanged, details->name(), QVariant::fromValue(data));
        Q_EMIT dataChanged(DataChanged::IndexChanged, details->name(), QVariant::fromValue(index));
        ++index;
    }
    for (auto &&item : detailsItems) {
        m_detailsItems.remove(item);
        Q_EMIT itemRemoved(item);
    }
}

void NetManagerThreadPrivate::updateAutoScan()
{
    if (m_autoScanInterval == 0) {
        if (m_autoScanTimer) {
            m_autoScanTimer->stop();
            delete m_autoScanTimer;
            m_autoScanTimer = nullptr;
        }
    } else {
        if (!m_autoScanTimer) {
            m_autoScanTimer = new QTimer(this);
            connect(m_autoScanTimer, &QTimer::timeout, this, &NetManagerThreadPrivate::doAutoScan);
        }
        if (m_autoScanEnabled) {
            m_autoScanTimer->start(m_autoScanInterval);
        } else if (m_autoScanTimer->isActive()) {
            m_autoScanTimer->stop();
        }
    }
}

void NetManagerThreadPrivate::doAutoScan()
{
    if (m_isSleeping) {
        qCDebug(DNC) << "is in sleeping, can't scan network";
        return;
    }
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    for (NetworkDeviceBase *device : devices) {
        if (device->deviceType() == DeviceType::Wireless) {
            auto *wirelessDevice = dynamic_cast<WirelessDevice *>(device);
            wirelessDevice->scanNetwork();
        }
    }
}

void NetManagerThreadPrivate::addDeviceNotify(const QString &path)
{
    if (!m_monitorNetworkNotify) {
        return;
    }
    Device::Ptr dev = NetworkManager::findNetworkInterface(path);
    if (!dev.isNull()) {
        connect(dev.get(), &Device::stateChanged, this, &NetManagerThreadPrivate::onNotifyDeviceStatusChanged, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    }
}

void NetManagerThreadPrivate::onNotifyDeviceStatusChanged(NetworkManager::Device::State newState, NetworkManager::Device::State oldState, NetworkManager::Device::StateChangeReason reason)
{
    qCInfo(DNC) << "On notify device status changed, new state: " << newState << ", old state: " << oldState << ", reason: " << reason;
    if (!m_monitorNetworkNotify) {
        return;
    }
    auto *device = dynamic_cast<NetworkManager::Device *>(sender());
    NetworkManager::ActiveConnection::Ptr conn = device->activeConnection();
    if (!conn.isNull()) {
        m_lastConnection = conn->id();
        m_lastConnectionUuid = conn->uuid();
        m_lastState = newState;
    } else if (m_lastState != oldState || m_lastConnection.isEmpty()) {
        m_lastConnection.clear();
        m_lastConnectionUuid.clear();
        return;
    }
    switch (newState) {
    case Device::State::Preparing: { // 正在连接
        if (oldState == Device::State::Disconnected) {
            switch (device->type()) {
            case Device::Type::Ethernet:
                sendNetworkNotify(NetworkNotifyType::WiredConnecting, m_lastConnection);
                break;
            case Device::Type::Wifi:
                sendNetworkNotify(NetworkNotifyType::WirelessConnecting, m_lastConnection);
                break;
            default:
                break;
            }
        }
    } break;
    case Device::State::Activated: { // 连接成功
        switch (device->type()) {
        case Device::Type::Ethernet:
            sendNetworkNotify(NetworkNotifyType::WiredConnected, m_lastConnection);
            break;
        case Device::Type::Wifi:
            sendNetworkNotify(NetworkNotifyType::WirelessConnected, m_lastConnection);
            break;
        default:
            break;
        }
    } break;
    case Device::State::Failed:
    case Device::State::Disconnected:
    case Device::State::NeedAuth:
    case Device::State::Unmanaged:
    case Device::State::Unavailable: {
        if (reason == Device::StateChangeReason::DeviceRemovedReason) {
            return;
        }

        // ignore if device's old state is not available
        if (oldState <= Device::State::Unavailable) {
            qCDebug(DNC) << "No notify, old state is not available, old state: " << oldState;
            return;
        }

        // ignore invalid reasons
        if (reason == Device::StateChangeReason::UnknownReason) {
            qCDebug(DNC) << "No notify, device state reason invalid, reason: " << reason;
            return;
        }

        switch (reason) {
        case Device::StateChangeReason::UserRequestedReason:
            if (newState == Device::State::Disconnected) {
                switch (device->type()) {
                case Device::Type::Ethernet:
                    sendNetworkNotify(NetworkNotifyType::WiredDisconnected, m_lastConnection);
                    break;
                case Device::Type::Wifi:
                    sendNetworkNotify(NetworkNotifyType::WirelessDisconnected, m_lastConnection);
                    break;
                default:
                    break;
                }
            }
            break;
        case Device::StateChangeReason::ConfigUnavailableReason:
        case Device::StateChangeReason::AuthSupplicantTimeoutReason: // 超时
            switch (device->type()) {
            case Device::Type::Ethernet:
                sendNetworkNotify(NetworkNotifyType::WiredUnableConnect, m_lastConnection);
                break;
            case Device::Type::Wifi:
                sendNetworkNotify(NetworkNotifyType::WirelessUnableConnect, m_lastConnection);
                break;
            default:
                break;
            }
            break;
        case Device::StateChangeReason::AuthSupplicantDisconnectReason:
            if (oldState == Device::State::ConfiguringHardware && newState == Device::State::NeedAuth) {
                switch (device->type()) {
                case Device::Type::Ethernet:
                    sendNetworkNotify(NetworkNotifyType::WiredConnectionFailed, m_lastConnection);
                    break;
                case Device::Type::Wifi:
                    sendNetworkNotify(NetworkNotifyType::WirelessConnectionFailed, m_lastConnection);
                    break;
                default:
                    break;
                }
            }
            break;
        case Device::StateChangeReason::CarrierReason:
            if (device->type() == Device::Ethernet) {
                qCDebug(DNC) << "Unplugged device is ethernet, device type: " << device->type();
                sendNetworkNotify(NetworkNotifyType::WiredDisconnected, m_lastConnection);
            }
            break;
        case Device::StateChangeReason::NoSecretsReason:
            sendNetworkNotify(NetworkNotifyType::NoSecrets, m_lastConnection);
            break;
        case Device::StateChangeReason::SsidNotFound:
            sendNetworkNotify(NetworkNotifyType::SsidNotFound, m_lastConnection);
            break;
        default:
            break;
        }
    } break;
    default:
        break;
    }
}

void NetManagerThreadPrivate::sendNetworkNotify(NetworkNotifyType type, const QString &name)
{
    if (!m_enabled)
        return;
    switch (type) {
    case NetworkNotifyType::WiredConnecting:
        sendNotify(notifyIconWiredDisconnected, tr("Connecting \"%1\"").arg(name));
        break;
    case NetworkNotifyType::WirelessConnecting:
        sendNotify(notifyIconWirelessDisconnected, tr("Connecting \"%1\"").arg(name));
        break;
    case NetworkNotifyType::WiredConnected:
        sendNotify(notifyIconWiredConnected, tr("\"%1\" connected").arg(name));
        break;
    case NetworkNotifyType::WirelessConnected:
        sendNotify(notifyIconWirelessConnected, tr("\"%1\" connected").arg(name));
        break;
    case NetworkNotifyType::WiredDisconnected:
        sendNotify(notifyIconWiredDisconnected, tr("\"%1\" disconnected").arg(name));
        break;
    case NetworkNotifyType::WirelessDisconnected:
        sendNotify(notifyIconWirelessDisconnected, tr("\"%1\" disconnected").arg(name));
        break;
    case NetworkNotifyType::WiredUnableConnect:
        sendNotify(notifyIconWiredDisconnected, tr("Unable to connect \"%1\", please check your router or net cable.").arg(name));
        break;
    case NetworkNotifyType::WirelessUnableConnect:
        sendNotify(notifyIconWirelessDisconnected, tr("Unable to connect \"%1\", please keep closer to the wireless router").arg(name));
        break;
    case NetworkNotifyType::WiredConnectionFailed:
        sendNotify(notifyIconWiredDisconnected, tr("Connection failed, unable to connect \"%1\", wrong password").arg(name));
        break;
    case NetworkNotifyType::WirelessConnectionFailed:
        sendNotify(notifyIconWirelessDisconnected, tr("Connection failed, unable to connect \"%1\", wrong password").arg(name));
        break;
    case NetworkNotifyType::NoSecrets:
        sendNotify(notifyIconWirelessDisconnected, tr("Password is required to connect \"%1\"").arg(name));
        break;
    case NetworkNotifyType::SsidNotFound:
        sendNotify(notifyIconWirelessDisconnected, tr("The \"%1\" 802.11 WLAN network could not be found").arg(name));
        break;
    case NetworkNotifyType::Wireless8021X:
        sendNotify(notifyIconWirelessDisconnected, tr("To connect \"%1\", please set up your authentication info after logging in").arg(name));
        break;
    }
}

void NetManagerThreadPrivate::updateHioceannNetworkConfig(WirelessDevice *wireless)
{
    if (!m_autoUpdateHioceannConfig || !m_enabled)
        return;

    DeviceStatus const status = wireless->deviceStatus();
    // 存在ap时，连接wp2企业版隐藏网络需要处理Config阶段
    if (status != DeviceStatus::Config)
        return;

    QString const wirelessPath = wireless->path();
    for (const auto &conn : NetworkManager::activeConnections()) {
        if (!conn->id().isEmpty() && conn->devices().contains(wirelessPath)) {
            NetworkManager::ConnectionSettings::Ptr const connSettings = conn->connection()->settings();
            NetworkManager::WirelessSetting::Ptr const wSetting = connSettings->setting(NetworkManager::Setting::SettingType::Wireless).staticCast<NetworkManager::WirelessSetting>();
            if (wSetting.isNull())
                continue;

            const QString settingMacAddress = wSetting->macAddress().toHex().toUpper();
            const QString deviceMacAddress = wireless->realHwAdr().remove(":");
            if (!settingMacAddress.isEmpty() && settingMacAddress != deviceMacAddress)
                continue;

            // 隐藏网络配置错误时提示重连
            if ((wSetting) && wSetting->hioceann()) {
                NetworkManager::WirelessSecuritySetting::Ptr const wsSetting =
                        connSettings->setting(NetworkManager::Setting::SettingType::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
                if ((wsSetting) && NetworkManager::WirelessSecuritySetting::KeyMgmt::Unknown == wsSetting->keyMgmt()) {
                    for (auto *ap : wireless->accessPointItems()) {
                        if (ap->ssid() == wSetting->ssid() && ap->secured() && ap->strength() > 0) {
                            handleAccessPointSecure(ap);
                        }
                    }
                }
            }
        }
    }
}

bool NetManagerThreadPrivate::needSetPassword(AccessPoints *accessPoint) const
{
    // 如果当前热点不是隐藏热点，或者当前热点不是加密热点，则需要设置密码（因为这个函数只是处理隐藏且加密的热点）
    if (!accessPoint->hioceann() || !accessPoint->secured() || accessPoint->status() != ConnectionStatus::Activating)
        return false;

    WirelessDevice *wirelessDevice = nullptr;
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    for (NetworkDeviceBase *device : devices) {
        if (device->deviceType() == DeviceType::Wireless && device->path() == accessPoint->devicePath()) {
            wirelessDevice = dynamic_cast<WirelessDevice *>(device);
            break;
        }
    }

    // 如果连这个连接的设备都找不到，则无需设置密码
    if (!wirelessDevice)
        return false;

    // 查找该热点对应的连接的UUID
    NetworkManager::Connection::Ptr connection;
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(wirelessDevice->path());
    if (device.isNull()) {
        device.reset(new NetworkManager::WirelessDevice(wirelessDevice->path()));
    }
    NetworkManager::Connection::List connectionList = device->availableConnections();
    for (const NetworkManager::Connection::Ptr &conn : connectionList) {
        NetworkManager::WirelessSetting::Ptr wSetting = conn->settings()->setting(NetworkManager::Setting::SettingType::Wireless).staticCast<NetworkManager::WirelessSetting>();
        if (wSetting.isNull())
            continue;

        if (wSetting->ssid() != accessPoint->ssid())
            continue;

        connection = conn;
        break;
    }

    if (connection.isNull())
        return true;

    // 查找该连接对应的密码配置信息
    NetworkManager::ConnectionSettings::Ptr settings = connection->settings();
    if (settings.isNull())
        return true;

    auto securitySetting = settings->setting(NetworkManager::Setting::SettingType::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();

    NetworkManager::WirelessSecuritySetting::KeyMgmt keyMgmt = securitySetting->keyMgmt();
    if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaNone || keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::Unknown)
        return true;

    NetworkManager::Setting::SettingType sType = NetworkManager::Setting::SettingType::WirelessSecurity;
    if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap || keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEapSuiteB192)
        sType = NetworkManager::Setting::SettingType::Security8021x;

    NetworkManager::Setting::Ptr wsSetting = settings->setting(sType);
    if (wsSetting.isNull())
        return false;

    QDBusPendingReply<NMVariantMapMap> reply = connection->secrets(wsSetting->name());

    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
        return true;

    NMVariantMapMap sSecretsMapMap = reply.value();
    QSharedPointer<NetworkManager::WirelessSecuritySetting> setting = settings->setting(sType).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (setting) {
        setting->secretsFromMap(sSecretsMapMap.value(setting->name()));
    } else {
        qCWarning(DNC) << "get WirelessSecuritySetting failed";
    }

    if (securitySetting.isNull())
        return true;

    QString psk;
    switch (keyMgmt) {
    case NetworkManager::WirelessSecuritySetting::KeyMgmt::Wep:
        psk = securitySetting->wepKey0();
        break;
    case NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap: {
        NetworkManager::Security8021xSetting::Ptr security8021xSetting = settings->setting(NetworkManager::Setting::SettingType::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
        if (!security8021xSetting.isNull())
            psk = security8021xSetting->password();
        break;
    }
    case NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaPsk:
    default:
        psk = securitySetting->psk();
        break;
    }

    // 如果该密码存在，则无需调用设置密码信息
    return psk.isEmpty();
}

void NetManagerThreadPrivate::handleAccessPointSecure(AccessPoints *accessPoint)
{
    if (!m_autoUpdateHioceannConfig || !m_enabled)
        return;

    if (needSetPassword(accessPoint)) {
        if (accessPoint->hioceann()) {
            // 隐藏网络逻辑是要输入密码重连，所以后端无等待，前端重连
            // wpa2企业版ap,第一次连接时,需要先删除之前默认创建的conn,然后跳转控制中心完善设置.
            qCInfo(DNC) << "Reconnect hioceann wireless, access point path: " << accessPoint->path();
            NetworkManager::AccessPoint const nmAp(accessPoint->path());
            NetworkManager::AccessPoint::WpaFlags const wpaFlags = nmAp.wpaFlags();
            NetworkManager::AccessPoint::WpaFlags const rsnFlags = nmAp.rsnFlags();
            bool const needIdentify = (wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmt8021x) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmt8021x));
            if (needIdentify) {
                handle8021xAccessPoint(accessPoint);
                return;
            }
        }
        QString devicePath = accessPoint->devicePath();
        QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
        auto it = std::find_if(devices.begin(), devices.end(), [devicePath](NetworkDeviceBase *dev) {
            return dev->path() == devicePath;
        });
        if (it == devices.end())
            return;

        NetWirelessConnect wConnect(dynamic_cast<WirelessDevice *>(*it), accessPoint, this);
        wConnect.setSsid(accessPoint->ssid());
        wConnect.initConnection();
        requestPassword(accessPoint->devicePath(), accessPoint->ssid(), { { "secrets", { wConnect.needSecrets() } } });
    }
}

void NetManagerThreadPrivate::handle8021xAccessPoint(AccessPoints *ap)
{
    // 每次ap状态变化时都会做一次处理，频繁地向控制中心发送showPage指令，导致控制中心卡顿甚至卡死
    // 故增加防抖措施
    int msecs = QTime::currentTime().msecsSinceStartOfDay();
    if (qFabs(msecs - m_lastThroughTime) < 500) {
        return;
    }
    m_lastThroughTime = msecs;

    switch (m_network8021XMode) {
    case NetManager::ToControlCenter:
        gotoControlCenter(ap->devicePath() + "," + ap->ssid());
        break;
    case NetManager::SendNotify:
        sendNetworkNotify(NetworkNotifyType::Wireless8021X, ap->ssid());
        break;
    case NetManager::ToConnect: {
        QStringList secrets = { "identity", "password" };
        sendRequest(NetManager::RequestPassword, apID(ap), { { "secrets", secrets } });
    } break;
    }
}

void NetManagerThreadPrivate::onPrepareForSleep(bool state)
{
    qCInfo(DNC) << "prepare for sleep" << state;
    m_isSleeping = state;
}

void NetManagerThreadPrivate::addDevice(NetDeviceItemPrivate *deviceItem, NetworkDeviceBase *dev)
{
    deviceItem->updatepathIndex(dev->path().mid(dev->path().lastIndexOf('/') + 1).toInt());
    deviceItem->updatename(dev->deviceName());
    deviceItem->updateenabled(dev->isEnabled() && dev->available());
    deviceItem->updateenabledable(dev->available());
    deviceItem->updateips(dev->ipv4());
    deviceItem->updatestatus(static_cast<NetType::NetDeviceStatus>(deviceStatus(dev)));
    connect(dev, &NetworkDeviceBase::nameChanged, this, &NetManagerThreadPrivate::onNameChanged);
    connect(dev, &NetworkDeviceBase::enableChanged, this, &NetManagerThreadPrivate::onDevEnabledChanged);
    connect(dev, &NetworkDeviceBase::availableChanged, this, &NetManagerThreadPrivate::onDevAvailableChanged);
    connect(dev, &NetworkDeviceBase::activeConnectionChanged, this, &NetManagerThreadPrivate::onActiveConnectionChanged);
    connect(dev, &NetworkDeviceBase::activeConnectionChanged, this, &NetManagerThreadPrivate::onAvailableConnectionsChanged);
    connect(dev, &NetworkDeviceBase::ipV4Changed, this, &NetManagerThreadPrivate::onIpV4Changed);
    connect(dev, &NetworkDeviceBase::deviceStatusChanged, this, &NetManagerThreadPrivate::onDeviceStatusChanged);
    connect(dev, &NetworkDeviceBase::enableChanged, this, &NetManagerThreadPrivate::onDeviceStatusChanged);
    connect(dev, &NetworkDeviceBase::ipV4Changed, this, &NetManagerThreadPrivate::onDeviceStatusChanged);
    addDeviceNotify(dev->path());
}

void NetManagerThreadPrivate::requestPassword(const QString &dev, const QString &id, const QVariantMap &param)
{
    Q_EMIT requestInputPassword(dev, id, param);
}

QString NetManagerThreadPrivate::connectionSuffixNum(const QString &matchConnName, const QString &name)
{
    if (matchConnName.isEmpty()) {
        return QString();
    }

    NetworkManager::Connection::List connList = listConnections();
    QStringList connNameList;

    for (const auto &conn : connList) {
        connNameList.append(conn->name());
    }
    if (!name.isEmpty() && !connNameList.contains(name)) {
        return name;
    }
    QString connName;
    for (int i = 1; i <= connNameList.size(); ++i) {
        connName = matchConnName.arg(i);
        if (!connNameList.contains(connName)) {
            break;
        }
    }
    return connName;
}

NetworkManager::WirelessSecuritySetting::KeyMgmt NetManagerThreadPrivate::getKeyMgmtByAp(AccessPoint *ap)
{
    if (nullptr == ap) {
        return WirelessSecuritySetting::WpaPsk;
    }
    AccessPoint::Capabilities capabilities = ap->capabilities();
    AccessPoint::WpaFlags wpaFlags = ap->wpaFlags();
    AccessPoint::WpaFlags rsnFlags = ap->rsnFlags();

    WirelessSecuritySetting::KeyMgmt keyMgmt = WirelessSecuritySetting::KeyMgmt::WpaNone;

    if (capabilities.testFlag(AccessPoint::Capability::Privacy) && !wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmtPsk) && !wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmt8021x)) {
        keyMgmt = WirelessSecuritySetting::KeyMgmt::Wep;
    }

    // #ifdef USE_LINGMO_NMQT
    //     // 判断是否是wpa3加密的，因为wpa3加密方式，实际上是wpa2的扩展，所以其中会包含KeyMgmtPsk枚举值
    //     if (wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::keyMgmtSae) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::keyMgmtSae)) {
    //         keyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaSae;
    //     }
    // #else
    if (wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtSAE) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtSAE)) {
        keyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt::SAE;
    }
    // #endif

    if (wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmt8021x) || rsnFlags.testFlag(AccessPoint::WpaFlag::KeyMgmt8021x)) {
        keyMgmt = WirelessSecuritySetting::KeyMgmt::WpaEap;
    }

    if (wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmtEapSuiteB192) || rsnFlags.testFlag(AccessPoint::WpaFlag::KeyMgmtEapSuiteB192)) {
        keyMgmt = WirelessSecuritySetting::KeyMgmt::WpaEapSuiteB192;
    }

    if (wpaFlags.testFlag(AccessPoint::WpaFlag::KeyMgmtPsk) || rsnFlags.testFlag(AccessPoint::WpaFlag::KeyMgmtPsk)) {
        keyMgmt = WirelessSecuritySetting::KeyMgmt::WpaPsk;
    }
    return keyMgmt;
}

NetType::NetDeviceStatus NetManagerThreadPrivate::toNetDeviceStatus(ConnectionStatus status)
{
    switch (status) {
    case ConnectionStatus::Deactivating:
    case ConnectionStatus::Activating:
        return NetType::NetDeviceStatus::DS_Connecting;
    case ConnectionStatus::Activated:
        return NetType::NetDeviceStatus::DS_Connected;
    default:
        break;
    }
    return NetType::NetDeviceStatus::DS_Disconnected;
}

NetType::NetConnectionStatus NetManagerThreadPrivate::toNetConnectionStatus(ConnectionStatus status)
{
    switch (status) {
    case ConnectionStatus::Activating:
        return NetType::NetConnectionStatus::CS_Connecting;
    case ConnectionStatus::Activated:
        return NetType::NetConnectionStatus::CS_Connected;
    default:
        break;
    }
    return NetType::NetConnectionStatus::CS_UnConnected;
}

NetType::NetDeviceStatus NetManagerThreadPrivate::deviceStatus(NetworkDeviceBase *device)
{
    // 如果当前网卡是有线网卡，且没有插入网线，那么就返回未插入网线
    if (device->deviceType() == ocean::network::DeviceType::Wired) {
        ocean::network::WiredDevice *wiredDevice = static_cast<ocean::network::WiredDevice *>(device);
        if (!wiredDevice->carrier())
            return NetType::NetDeviceStatus::DS_NoCable;
    }

    if (!device->available())
        return NetType::NetDeviceStatus::DS_Unavailable;

    // 如果当前网卡是禁用，直接返回禁用
    if (!device->isEnabled())
        return NetType::NetDeviceStatus::DS_Disabled;

    // ip 冲突
    if (device->ipConflicted())
        return NetType::NetDeviceStatus::DS_IpConflicted;

    // 网络是已连接，但是当前的连接状态不是Full，则认为网络连接成功，但是无法上网
    if (device->deviceStatus() == DeviceStatus::Activated && NetworkController::instance()->connectivity() != Connectivity::Full) {
        return NetType::NetDeviceStatus::DS_ConnectNoInternet;
    }

    // 获取IP地址失败
    if (!device->IPValid())
        return NetType::NetDeviceStatus::DS_ObtainIpFailed;

    // 根据设备状态来直接获取返回值
    switch (device->deviceStatus()) {
    case DeviceStatus::Unmanaged:
    case DeviceStatus::Unavailable:
        return NetType::NetDeviceStatus::DS_NoCable;
    case DeviceStatus::Disconnected:
        return NetType::NetDeviceStatus::DS_Disconnected;
    case DeviceStatus::Prepare:
    case DeviceStatus::Config:
        return NetType::NetDeviceStatus::DS_Connecting;
    case DeviceStatus::Needauth:
        return NetType::NetDeviceStatus::DS_Authenticating;
    case DeviceStatus::IpConfig:
    case DeviceStatus::IpCheck:
    case DeviceStatus::Secondaries:
        return NetType::NetDeviceStatus::DS_ObtainingIP;
    case DeviceStatus::Activated:
        return NetType::NetDeviceStatus::DS_Connected;
    case DeviceStatus::Deactivation:
    case DeviceStatus::Failed:
        return NetType::NetDeviceStatus::DS_ConnectFailed;
    case DeviceStatus::IpConflict:
        return NetType::NetDeviceStatus::DS_IpConflicted;
    default:
        return NetType::NetDeviceStatus::DS_Unknown;
    }

    Q_UNREACHABLE();
    return NetType::NetDeviceStatus::DS_Unknown;
}

} // namespace network
} // namespace ocean
