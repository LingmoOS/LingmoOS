// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "netmanagerthreadprivate.h"

#include "configsetting.h"
#include "netsecretagent.h"
#include "netsecretagentforui.h"
#include "netwirelessconnect.h"
#include "networkcontroller.h"
#include "networkdevicebase.h"
#include "networkmanagerqt/manager.h"
#include "wireddevice.h"
#include "wirelessdevice.h"
#include "configwatcher.h"

#include <proxycontroller.h>
#include <vpncontroller.h>
#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>

#include <QDBusConnection>
#include <QThread>

using namespace NetworkManager;

namespace dde {
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

NetManagerThreadPrivate::NetManagerThreadPrivate()
    : QObject()
    , m_thread(new QThread(this))
    , m_parentThread(QThread::currentThread())
    , m_loadForNM(false)
    , m_monitorNetworkNotify(false)
    , m_useSecretAgent(true)
    , m_network8021XMode(NetManager::ToControlCenter)
    , m_autoUpdateHiddenConfig(true)
    , m_isInitialized(false)
    , m_enabled(true)
    , m_autoScanInterval(0)
    , m_autoScanEnabled(false)
    , m_autoScanTimer(nullptr)
    , m_lastThroughTime(0)
    , m_lastState(NetworkManager::Device::State::UnknownState)
    , m_secretAgent(nullptr)
    , m_netCheckAvailable(false)
    , m_serverKey("dock")
{
    moveToThread(m_thread);
    m_thread->start();
}

NetManagerThreadPrivate::~NetManagerThreadPrivate()
{
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
}

// 检查参数,参数有错误的才在QVariantMap里,value暂时为空(预留以后要显示具体错误)
QVariantMap NetManagerThreadPrivate::CheckParamValid(const QVariantMap &param)
{
    QVariantMap validMap;
    bool isValid = true;
    for (auto &&it = param.cbegin(); it != param.cend(); ++it) {
        const QString &key = it.key();
        if (key == "psk") {
            isValid = NetworkManager::wpaPskIsValid(it.value().toString());
        } else if (key == "wep-key0" || key == "wep-key1" || key == "wep-key2" || key == "wep-key3") {
            isValid = NetworkManager::wepKeyIsValid(it.value().toString(), WirelessSecuritySetting::WepKeyType::Passphrase);
        } else {
            isValid = !it.value().toString().isEmpty();
        }
        if (!isValid) {
            validMap.insert(key, QString());
        }
    }
    return validMap;
}

void NetManagerThreadPrivate::getNetCheckAvailableFromDBus()
{
    QDBusMessage message = QDBusMessage::createMethodCall("com.deepin.defender.netcheck",
                                                          "/com/deepin/defender/netcheck",
                                                          "org.freedesktop.DBus.Properties",
                                                          "Get");
    message << "com.deepin.defender.netcheck"
            << "Availabled";
    QDBusConnection::systemBus().callWithCallback(message, this, SLOT(updateNetCheckAvailabled(QDBusVariant)));
}

void NetManagerThreadPrivate::getAirplaneModeEnabled()
{
    QDBusMessage message = QDBusMessage::createMethodCall("com.deepin.daemon.AirplaneMode",
                                                          "/com/deepin/daemon/AirplaneMode",
                                                          "org.freedesktop.DBus.Properties",
                                                          "Get");
    message << "com.deepin.daemon.AirplaneMode"
            << "Enabled";
    QDBusConnection::systemBus().callWithCallback(message, this, SLOT(updateAirplaneModeEnabled(QDBusVariant)));
}

void NetManagerThreadPrivate::setServiceLoadForNM(bool isNM)
{
    if (m_isInitialized)
        return;
    m_loadForNM = isNM;
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

void NetManagerThreadPrivate::setAutoUpdateHiddenConfig(bool autoUpdate)
{
    m_autoUpdateHiddenConfig = autoUpdate;
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

void NetManagerThreadPrivate::init()
{
    // 在主线程中先安装翻译器，因为直接在子线程中安装翻译器可能会引起崩溃
    NetworkController::installTranslator(QLocale().name());
    QMetaObject::invokeMethod(this, "doInit", Qt::QueuedConnection);
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

void NetManagerThreadPrivate::connectHidden(const QString &id, const QString &ssid)
{
    if (m_isInitialized)
        QMetaObject::invokeMethod(this, "doConnectHidden", Qt::QueuedConnection, Q_ARG(QString, id), Q_ARG(QString, ssid));
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

void NetManagerThreadPrivate::onNetCheckPropertiesChanged(QString, QVariantMap properties, QStringList)
{
    if(properties.contains("Availabled")){
        updateNetCheckAvailabled(properties.value("Availabled").value<QDBusVariant>());
    }
}

void NetManagerThreadPrivate::onAirplaneModeEnabledPropertiesChanged(QString, QVariantMap properties, QStringList)
{
    if(properties.contains("Enabled")){
        updateAirplaneModeEnabled(QDBusVariant(properties.value("Enabled").value<bool>()));
    }
}

// clang-format on

void NetManagerThreadPrivate::doInit()
{
    if (m_isInitialized)
        return;
    m_isInitialized = true;

    qRegisterMetaType<NetworkManager::Device::State>("NetworkManager::Device::State");
    qRegisterMetaType<NetworkManager::Device::StateChangeReason>("NetworkManager::Device::StateChangeReason");
    qRegisterMetaType<Connectivity>("Connectivity");

    if (m_loadForNM) {
        NetworkController::alawaysLoadFromNM();
    }
    NetworkController::setIPConflictCheck(true);
    NetworkController *networkController = NetworkController::instance();

    connect(m_thread, &QThread::finished, this, &NetManagerThreadPrivate::clearData);
    connect(networkController, &NetworkController::deviceAdded, this, &NetManagerThreadPrivate::onDeviceAdded);
    connect(networkController, &NetworkController::deviceRemoved, this, &NetManagerThreadPrivate::onDeviceRemoved);
    connect(networkController, &NetworkController::connectivityChanged, this, &NetManagerThreadPrivate::onConnectivityChanged);

    if (m_useSecretAgent) {
        // 密码代理按设置来，不与ConfigSetting::instance()->serviceFromNetworkManager()同步
        if (m_loadForNM) {
            m_secretAgent = new NetSecretAgent(std::bind(&NetManagerThreadPrivate::requestPassword, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), true, this);
        } else {
            m_secretAgent = new NetSecretAgentForUI(std::bind(&NetManagerThreadPrivate::requestPassword, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), m_serverKey, this);
        }
    }

    onDeviceAdded(networkController->devices());
    if (m_autoScanInterval == 0) { // 没有设置则以配置中值设置下
        m_autoScanInterval = ConfigSetting::instance()->wirelessScanInterval();
        connect(ConfigSetting::instance(), &ConfigSetting::wirelessScanIntervalChanged, this, &NetManagerThreadPrivate::setAutoScanInterval);
    }
    updateAutoScan();

    // VPN
    auto vpnConnectionStateChanged = [this, networkController] {
        auto itemList = networkController->vpnController()->items();
        NetDeviceStatus state = NetDeviceStatus::Disconnected;
        // 有一个连接或者正在连接中，则不显示提示项
        for (const auto item : itemList) {
            if (item->status() == ConnectionStatus::Activated || item->status() == ConnectionStatus::Activating || item->status() == ConnectionStatus::Deactivating) {
                state = toNetDeviceStatus(item->status());
                if (item->status() == ConnectionStatus::Activated)
                    Q_EMIT dataChanged(DataChanged::IPChanged, "NetVPNControlItem", QVariant::fromValue(item->connection()->id()));
                break;
            }
            continue;
        }
        Q_EMIT dataChanged(DataChanged::VPNConnectionStateChanged, "NetVPNControlItem", QVariant::fromValue(state));
    };

    auto vpnItemChanged = [this, networkController, vpnConnectionStateChanged] {
        auto itemList = networkController->vpnController()->items();
        Q_EMIT dataChanged(DataChanged::DeviceAvailableChanged, "NetVPNControlItem", itemList.size() > 0);
        vpnConnectionStateChanged();
    };

    auto vpnEnableChanged = [this](const bool enabled) {
        Q_EMIT dataChanged(DataChanged::EnabledChanged, "NetVPNControlItem", enabled);
    };

    connect(networkController->vpnController(), &VPNController::enableChanged, this, vpnEnableChanged);
    connect(networkController->vpnController(), &VPNController::itemChanged, this, vpnItemChanged);
    connect(networkController->vpnController(), &VPNController::itemAdded, this, vpnItemChanged);
    connect(networkController->vpnController(), &VPNController::itemRemoved, this, vpnItemChanged);
    connect(networkController->vpnController(), &VPNController::enableChanged, this, [this, vpnConnectionStateChanged] {
        // 开启vpn后，vpn连接状态开始是Deactive状态，会导致界面闪现提示语
        QTimer::singleShot(50, this, vpnConnectionStateChanged);
    });
    vpnItemChanged();
    vpnEnableChanged(networkController->vpnController()->enabled());
    connect(networkController->vpnController(), &VPNController::activeConnectionChanged, this, vpnConnectionStateChanged);
    vpnConnectionStateChanged();

    // 系统代理
    auto proxyMethodChanged = [this, networkController] {
        networkController->proxyController()->querySysProxyData();
        ProxyMethod method = networkController->proxyController()->proxyMethod();
        const bool enabled = method == ProxyMethod::Auto || method == ProxyMethod::Manual;
        if (enabled) {
            ConfigWatcher::instance()->setProxyMethod(method);
        }
        Q_EMIT dataChanged(DataChanged::EnabledChanged, "NetSystemProxyControlItem", enabled);
    };
    auto systemProxyExistChanged = [this, networkController] {
        networkController->proxyController()->querySysProxyData();
        Q_EMIT dataChanged(DataChanged::DeviceAvailableChanged, "NetSystemProxyControlItem", networkController->proxyController()->systemProxyExist());
    };
    connect(networkController->proxyController(), &ProxyController::proxyMethodChanged, this, proxyMethodChanged);
    connect(networkController->proxyController(), &ProxyController::systemProxyExistChanged, this, systemProxyExistChanged);
    proxyMethodChanged();
    systemProxyExistChanged();

    m_isInitialized = true;
    m_netCheckAvailable = false;
    getNetCheckAvailableFromDBus();

    QDBusConnection::systemBus().connect("com.deepin.defender.netcheck",
                                         "/com/deepin/defender/netcheck",
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(onNetCheckPropertiesChanged(QString, QVariantMap, QStringList)));

    m_airplaneModeEnabled = true;
    getAirplaneModeEnabled();
    QDBusConnection::systemBus().connect("com.deepin.daemon.AirplaneMode",
                                         "/com/deepin/daemon/AirplaneMode",
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(onAirplaneModeEnabledPropertiesChanged(QString, QVariantMap, QStringList)));

    // 优先网络
    auto updadePrimaryConnectionType = [this] {
        Q_EMIT dataChanged(DataChanged::primaryConnectionTypeChanged, "", NetworkManager::primaryConnectionType());
    };
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionTypeChanged, this, updadePrimaryConnectionType);
    updadePrimaryConnectionType();

    // 初始化的关键参数,保留格式
    qCInfo(DNC) << "Interface Version :" << INTERFACE_VERSION;
    qCInfo(DNC) << "Service From NM   :" << m_loadForNM << "Config:" << ConfigSetting::instance()->serviceFromNetworkManager();
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
    }
    if (id == "NetSystemProxyControlItem") {
        NetworkController::instance()->proxyController()->setProxyMethod(enabled ? ConfigWatcher::instance()->proxyMethod() : ProxyMethod::None);
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

void NetManagerThreadPrivate::doConnectHidden(const QString &id, const QString &ssid)
{
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    auto it = std::find_if(devices.begin(), devices.end(), [id](NetworkDeviceBase *dev) {
        return dev->path() == id;
    });
    if (it == devices.end())
        return;

    WirelessDevice *wirelessDevice = qobject_cast<WirelessDevice *>(*it);
    qCInfo(DNC) << "Wireless connect hidden, id: " << id << "ssid: " << ssid << "wireless device: " << wirelessDevice;
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
    if (m_secretAgent->hasTask()) {
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

void NetManagerThreadPrivate::doGotoControlCenter(const QString &page)
{
    if (!m_enabled)
        return;
    QDBusMessage message = QDBusMessage::createMethodCall("com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter", "com.deepin.dde.ControlCenter", "ShowPage");
    message << "network" << page;
    QDBusConnection::sessionBus().asyncCall(message);
    Q_EMIT toControlCenter();
}

void NetManagerThreadPrivate::doGotoSecurityTools(const QString &page)
{
    if (!m_enabled)
        return;

    QDBusMessage message = QDBusMessage::createMethodCall("com.deepin.defender.hmiscreen", "/com/deepin/defender/hmiscreen", "com.deepin.defender.hmiscreen", "ShowPage");
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
    Q_EMIT dataChanged(DataChanged::AirplaneModeEnabledChanged, "Airplane Mode", QVariant(m_airplaneModeEnabled));
}

bool NetManagerThreadPrivate::supportAirplaneMode() const
{
    // dde-dconfig配置优先级高于设备优先级
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

void NetManagerThreadPrivate::sendRequest(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    if (!m_enabled)
        return;
    Q_EMIT request(cmd, id, param);
}

void NetManagerThreadPrivate::onDeviceAdded(QList<NetworkDeviceBase *> devices)
{
    for (NetworkDeviceBase *device : devices) {
        qCInfo(DNC) << "On device added, new device:" << device->deviceName();
        switch (device->deviceType()) {
        case DeviceType::Wired: {
            WiredDevice *wiredDevice = static_cast<WiredDevice *>(device);
            NetWiredDeviceItem *wiredDeviceItem = new NetWiredDeviceItem(wiredDevice->path());
            addDevice(wiredDeviceItem, wiredDevice);
            wiredDeviceItem->moveToThread(m_parentThread);
            Q_EMIT itemAdded("ROOT", wiredDeviceItem);
            addConnection(wiredDevice, wiredDevice->items());
            connect(wiredDevice, &WiredDevice::connectionAdded, this, &NetManagerThreadPrivate::onConnectionAdded);
            connect(wiredDevice, &WiredDevice::connectionRemoved, this, &NetManagerThreadPrivate::onConnectionRemoved);
            connect(wiredDevice, &WiredDevice::carrierChanged, this, &NetManagerThreadPrivate::onDeviceStatusChanged);
        } break;
        case DeviceType::Wireless: {
            WirelessDevice *wirelessDevice = static_cast<WirelessDevice *>(device);
            NetWirelessDeviceItem *wirelessDeviceItem = new NetWirelessDeviceItem(wirelessDevice->path());
            addDevice(wirelessDeviceItem, wirelessDevice);
            wirelessDeviceItem->updateApMode(wirelessDevice->hotspotEnabled());
            wirelessDeviceItem->moveToThread(m_parentThread);
            Q_EMIT itemAdded("ROOT", wirelessDeviceItem);
            getAirplaneModeEnabled();
            addNetwork(wirelessDevice, wirelessDevice->accessPointItems());
            connect(wirelessDevice, &WirelessDevice::networkAdded, this, &NetManagerThreadPrivate::onNetworkAdded);
            connect(wirelessDevice, &WirelessDevice::networkRemoved, this, &NetManagerThreadPrivate::onNetworkRemoved);
            connect(wirelessDevice, &WirelessDevice::hotspotEnableChanged, this, &NetManagerThreadPrivate::onHotspotEnabledChanged);
            connect(wirelessDevice, &WirelessDevice::wirelessConnectionAdded, this, &NetManagerThreadPrivate::onAvailableConnectionsChanged);
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
}

void NetManagerThreadPrivate::onConnectivityChanged()
{
    for (auto &&dev : NetworkController::instance()->devices())
        Q_EMIT dataChanged(DataChanged::DeviceStatusChanged, dev->path(), QVariant::fromValue(deviceStatus(dev)));
}

void NetManagerThreadPrivate::onConnectionAdded(const QList<WiredConnection *> &conns)
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
        NetWiredItem *item = new NetWiredItem(device->path() + ":" + conn->connection()->path());
        item->updateName(conn->connection()->id());
        item->updateStatus(toNetConnectionStatus(conn->status()));
        item->moveToThread(m_parentThread);
        Q_EMIT itemAdded(device->path(), item);
    }
}

void NetManagerThreadPrivate::onNetworkAdded(const QList<AccessPoints *> &aps)
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
        NetWirelessItem *item = new NetWirelessItem(apID(ap)); // ap->path());
        item->updateName(ap->ssid());
        item->updateFlags(static_cast<uint>(ap->type()));
        item->updateStrength(ap->strength());
        item->updateSecure(ap->secured());
        item->updateStatus(toNetConnectionStatus(ap->status()));
        item->moveToThread(m_parentThread);

        NetworkManager::Connection::List::iterator itConnection = std::find_if(connections.begin(), connections.end(), [ap](NetworkManager::Connection::Ptr connection) {
            NetworkManager::WirelessSetting::Ptr wirelessSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
            if (wirelessSetting.isNull())
                return false;
            return wirelessSetting->ssid() == ap->ssid() && !connection->isUnsaved();
        });
        item->updateHasConnection(itConnection != connections.end());
        Q_EMIT itemAdded(device->path(), item);

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
            Q_EMIT dataChanged(DataChanged::WiredStatusChanged, wiredDev->path() + ":" + conn->connection()->path(), QVariant::fromValue(toNetConnectionStatus(conn->status())));
        }
    } break;
    case DeviceType::Wireless:
        updateHiddenNetworkConfig(qobject_cast<WirelessDevice *>(dev));
        break;
    default:
        break;
    }
}

void NetManagerThreadPrivate::onIpV4Changed()
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;
    Q_EMIT dataChanged(DataChanged::IPChanged, dev->path(), QVariant::fromValue(dev->ipv4()));
}

void NetManagerThreadPrivate::onDeviceStatusChanged()
{
    NetworkDeviceBase *dev = qobject_cast<NetworkDeviceBase *>(sender());
    if (!dev)
        return;
    Q_EMIT dataChanged(DataChanged::DeviceStatusChanged, dev->path(), QVariant::fromValue(deviceStatus(dev)));
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

void NetManagerThreadPrivate::updateHiddenNetworkConfig(WirelessDevice *wireless)
{
    if (!m_autoUpdateHiddenConfig || !m_enabled)
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
            if ((wSetting) && wSetting->hidden()) {
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
    if (!accessPoint->hidden() || !accessPoint->secured() || accessPoint->status() != ConnectionStatus::Activating)
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
        qWarning() << "get WirelessSecuritySetting failed";
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
    if (!m_autoUpdateHiddenConfig || !m_enabled)
        return;

    if (needSetPassword(accessPoint)) {
        if (accessPoint->hidden()) {
            // 隐藏网络逻辑是要输入密码重连，所以后端无等待，前端重连
            // wpa2企业版ap,第一次连接时,需要先删除之前默认创建的conn,然后跳转控制中心完善设置.
            qCInfo(DNC) << "Reconnect hidden wireless, access point path: " << accessPoint->path();
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

void NetManagerThreadPrivate::addDevice(NetDeviceItem *deviceItem, NetworkDeviceBase *dev)
{
    deviceItem->updateName(dev->deviceName());
    deviceItem->updateEnabled(dev->isEnabled() && dev->available());
    deviceItem->updateEnabledable(dev->available());
    deviceItem->updateIps(dev->ipv4());
    deviceItem->updateStatus(static_cast<NetDeviceStatus>(deviceStatus(dev)));
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

NetDeviceStatus NetManagerThreadPrivate::toNetDeviceStatus(ConnectionStatus status)
{
    switch (status) {
    case ConnectionStatus::Deactivating:
    case ConnectionStatus::Activating:
        return NetDeviceStatus::Connecting;
    case ConnectionStatus::Activated:
        return NetDeviceStatus::Connected;
    default:
        break;
    }
    return NetDeviceStatus::Disconnected;
}

NetConnectionStatus NetManagerThreadPrivate::toNetConnectionStatus(ConnectionStatus status)
{
    switch (status) {
    case ConnectionStatus::Activating:
        return NetConnectionStatus::Connecting;
    case ConnectionStatus::Activated:
        return NetConnectionStatus::Connected;
    default:
        break;
    }
    return NetConnectionStatus::UnConnected;
}

NetDeviceStatus NetManagerThreadPrivate::deviceStatus(NetworkDeviceBase *device)
{
    // 如果当前网卡是有线网卡，且没有插入网线，那么就返回未插入网线
    if (device->deviceType() == dde::network::DeviceType::Wired) {
        dde::network::WiredDevice *wiredDevice = static_cast<dde::network::WiredDevice *>(device);
        if (!wiredDevice->carrier())
            return NetDeviceStatus::NoCable;
    }

    if (!device->available())
        return NetDeviceStatus::Unavailable;

    // 如果当前网卡是禁用，直接返回禁用
    if (!device->isEnabled())
        return NetDeviceStatus::Disabled;

    // ip 冲突
    if (device->ipConflicted())
        return NetDeviceStatus::IpConflicted;

    // 网络是已连接，但是当前的连接状态不是Full，则认为网络连接成功，但是无法上网
    if (device->deviceStatus() == DeviceStatus::Activated && NetworkController::instance()->connectivity() != Connectivity::Full) {
        return NetDeviceStatus::ConnectNoInternet;
    }

    // 获取IP地址失败
    if (!device->IPValid())
        return NetDeviceStatus::ObtainIpFailed;

    // 根据设备状态来直接获取返回值
    switch (device->deviceStatus()) {
    case DeviceStatus::Unmanaged:
    case DeviceStatus::Unavailable:
        return NetDeviceStatus::NoCable;
    case DeviceStatus::Disconnected:
        return NetDeviceStatus::Disconnected;
    case DeviceStatus::Prepare:
    case DeviceStatus::Config:
        return NetDeviceStatus::Connecting;
    case DeviceStatus::Needauth:
        return NetDeviceStatus::Authenticating;
    case DeviceStatus::IpConfig:
    case DeviceStatus::IpCheck:
    case DeviceStatus::Secondaries:
        return NetDeviceStatus::ObtainingIP;
    case DeviceStatus::Activated:
        return NetDeviceStatus::Connected;
    case DeviceStatus::Deactivation:
    case DeviceStatus::Failed:
        return NetDeviceStatus::ConnectFailed;
    case DeviceStatus::IpConflict:
        return NetDeviceStatus::IpConflicted;
    default:
        return NetDeviceStatus::Unknown;
    }

    Q_UNREACHABLE();
    return NetDeviceStatus::Unknown;
}

} // namespace network
} // namespace dde
