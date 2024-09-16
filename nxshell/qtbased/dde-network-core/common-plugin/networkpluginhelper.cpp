// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "networkpluginhelper.h"
#include "widgets/tipswidget.h"
#include "utils.h"
#include "item/devicestatushandler.h"
#include "networkdialog.h"

#include <DHiDPIHelper>
#include <DApplicationHelper>
#include <DMenu>

#include <QTimer>
#include <QVBoxLayout>
#include <QAction>
#include <QJsonDocument>
#include <QDBusMetaType>

#include <networkcontroller.h>
#include <networkdevicebase.h>
#include <wireddevice.h>
#include <wirelessdevice.h>

#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Setting>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>

#include "networkdbusproxy.h"

NETWORKPLUGIN_USE_NAMESPACE

NetworkPluginHelper::NetworkPluginHelper(NetworkDialog *networkDialog, QObject *parent)
    : QObject(parent)
    , m_pluginState(PluginState::Unknown)
    , m_tipsWidget(new TipsWidget(nullptr))
    , m_switchWire(true)
    , m_networkDialog(networkDialog)
    , m_isDarkIcon(true)
    , m_refreshIconTimer(new QTimer(this))
    , m_trayIcon(new QIcon(QIcon::fromTheme(":/light/wireless-disabled-symbolic")))
{
    qDBusRegisterMetaType<NMVariantMapMap>();
    initUi();
    initConnection();
}

NetworkPluginHelper::~NetworkPluginHelper()
{
    delete m_tipsWidget;
    delete m_trayIcon;
}

void NetworkPluginHelper::initUi()
{
    m_tipsWidget->setVisible(false);
    m_tipsWidget->setSpliter(" :  ");
}

void NetworkPluginHelper::initConnection()
{
    // 主题发生变化触发的信号
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &NetworkPluginHelper::onUpdatePlugView);

    // 连接信号
    NetworkController *networkController = NetworkController::instance();
    connect(networkController, &NetworkController::deviceAdded, this, &NetworkPluginHelper::onDeviceAdded);
    connect(networkController, &NetworkController::deviceRemoved, this, &NetworkPluginHelper::onUpdatePlugView);
    connect(networkController, &NetworkController::connectivityChanged, this, &NetworkPluginHelper::onUpdatePlugView);

    m_refreshIconTimer->setInterval(200);
    connect(m_refreshIconTimer, &QTimer::timeout, this, &NetworkPluginHelper::refreshIcon);

    refreshIcon();
    QTimer::singleShot(100, this, [ = ] {
        onDeviceAdded(networkController->devices());
        refreshIcon();
    });
}

void NetworkPluginHelper::updatePluginState()
{
    auto state = DeviceStatusHandler::pluginState();
    if (m_pluginState == state)
        return;
    m_pluginState = state;
    Q_EMIT pluginStateChanged(state);
}

PluginState NetworkPluginHelper::getPluginState()
{
    return m_pluginState;
}

QList<QPair<QString, QStringList>> NetworkPluginHelper::ipTipsMessage(const DeviceType &devType)
{
    DeviceType type = static_cast<DeviceType>(devType);
    QList<QPair<QString, QStringList>> tipMessage;
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    for (NetworkDeviceBase *device : devices) {
        if (device->deviceType() != type)
            continue;

        QStringList ipv4 = device->ipv4();
        if (ipv4.isEmpty() || ipv4[0].isEmpty())
            continue;

        QStringList ipv4Messages;
        for (int i = 0; i < ipv4.size(); i++) {
            ipv4Messages << (i < 3 ? ipv4[i] : "......");
            if (i >= 3)
                break;
        }

        tipMessage << QPair<QString, QStringList>({ device->deviceName(), ipv4Messages });
    }
    return tipMessage;
}

void NetworkPluginHelper::updateTooltips()
{
    switch (m_pluginState) {
    case PluginState::Connected: {
        QList<QPair<QString, QStringList>> textList;
        textList << ipTipsMessage(DeviceType::Wireless) << ipTipsMessage(DeviceType::Wired);
        m_tipsWidget->setContext(textList);
        break;
    }
    case PluginState::WirelessConnected:
        m_tipsWidget->setContext(ipTipsMessage(DeviceType::Wireless));
        break;
    case PluginState::WiredConnected:
        m_tipsWidget->setContext(ipTipsMessage(DeviceType::Wired));
        break;
    case PluginState::Disabled:
    case PluginState::WirelessDisabled:
    case PluginState::WiredDisabled: {
            QList<QPair<QString, QStringList>> tips;
            tips << QPair<QString, QStringList>({ tr("Device disabled"), QStringList() });
            m_tipsWidget->setContext(tips);
        }
        break;
    case PluginState::Unknown:
    case PluginState::Nocable: {
            QList<QPair<QString, QStringList>> tips;
            tips << QPair<QString, QStringList>({ tr("Network cable unplugged"), QStringList() });
            m_tipsWidget->setContext(tips);
        }
        break;
    case PluginState::Disconnected:
    case PluginState::WirelessDisconnected:
    case PluginState::WiredDisconnected: {
            QList<QPair<QString, QStringList>> tips;
            tips << QPair<QString, QStringList>({ tr("Not connected"), QStringList() });
            m_tipsWidget->setContext(tips);
        }
        break;
    case PluginState::Connecting:
    case PluginState::WirelessConnecting:
    case PluginState::WiredConnecting: {
            QList<QPair<QString, QStringList>> tips;
            tips << QPair<QString, QStringList>({ tr("Connecting"), QStringList() });
            m_tipsWidget->setContext(tips);
        }
        break;
    case PluginState::ConnectNoInternet:
    case PluginState::WirelessConnectNoInternet:
    case PluginState::WiredConnectNoInternet: {
            QList<QPair<QString, QStringList>> tips;
            tips << QPair<QString, QStringList>({ tr("Connected but no Internet access"), QStringList() });
            m_tipsWidget->setContext(tips);
        }
        break;
    case PluginState::Failed:
    case PluginState::WirelessFailed:
    case PluginState::WiredFailed: {
            QList<QPair<QString, QStringList>> tips;
            tips << QPair<QString, QStringList>({ tr("Connection failed"), QStringList() });
            m_tipsWidget->setContext(tips);
        }
        break;
    case PluginState::WiredIpConflicted:
    case PluginState::WirelessIpConflicted: {
            QList<QPair<QString, QStringList>> tips;
            tips << QPair<QString, QStringList>({ tr("IP conflict"), QStringList() });
            m_tipsWidget->setContext(tips);
        }
        break;
    }
}

QIcon *NetworkPluginHelper::trayIcon() const
{
    return m_trayIcon;
}

QIcon NetworkPluginHelper::icon(int colorType) const
{
    return QIcon::fromTheme(iconPath(colorType));
}

QString NetworkPluginHelper::iconPath(int colorType) const
{
    QString stateString;
    QString iconString;
    QString localPath = (colorType == DGuiApplicationHelper::ColorType::DarkType ? ":/dark/" : ":/light/");

    switch (m_pluginState) {
    case PluginState::Disabled:
    case PluginState::WirelessDisabled:
        stateString = "disabled";
        iconString = QString("wireless-%1-symbolic").arg(stateString);
        break;
    case PluginState::WiredDisabled:
        stateString = "disabled";
        iconString = QString("network-%1-symbolic").arg(stateString);
        break;
    case PluginState::Connected:
    case PluginState::WirelessConnected: {
        bool isWlan6 = false;
        AccessPoints *activeAp = getStrongestAp();
        if (activeAp) {
            isWlan6 = (activeAp->type() == AccessPoints::WlanType::wlan6);
        }

        stateString = getStrengthStateString(activeAp ? activeAp->strength() : 0);
        if (isWlan6) {
            iconString = QString("wireless6-%1-symbolic").arg(stateString);
        } else {
            iconString = QString("wireless-%1-symbolic").arg(stateString);
        }
    }
    break;
    case PluginState::WiredConnected:
        stateString = "online";
        iconString = QString("network-%1-symbolic").arg(stateString);
        break;
    case PluginState::Disconnected:
    case PluginState::WirelessDisconnected:
        stateString = "0";
        iconString = QString("wireless-%1-symbolic").arg(stateString);
        break;
    case PluginState::WiredDisconnected:
        stateString = "none";
        iconString = QString("network-%1-symbolic").arg(stateString);
        break;
    case PluginState::Connecting: {
        if (QTime::currentTime().second() & 2) {
            stateString = getStrengthStateString(QTime::currentTime().msec() / 10 % 100);
            iconString = QString("wireless-%1-symbolic").arg(stateString);
        } else {
            const int index = QTime::currentTime().msec() / 200 % 10;
            const int num = index + 1;
            iconString = QString("network-wired-symbolic-connecting%1").arg(num);
        }
        break;
    }
    case PluginState::WirelessConnecting: {
        stateString = getStrengthStateString(QTime::currentTime().msec() / 10 % 100);
        iconString = QString("wireless-%1-symbolic").arg(stateString);
        break;
    }
    case PluginState::WiredConnecting: {
        const int index = QTime::currentTime().msec() / 200 % 10;
        const int num = index + 1;
        iconString = QString("network-wired-symbolic-connecting%1").arg(num);
        break;
    }
    case PluginState::ConnectNoInternet:
    case PluginState::WirelessConnectNoInternet: {
        // 无线已连接但无法访问互联网 offline
        bool isWlan6 = false;
        AccessPoints *connectedAp = getConnectedAp();
        if (connectedAp) {
            isWlan6 = (connectedAp->type() == AccessPoints::WlanType::wlan6);
        }

        stateString = "offline";
        if (isWlan6) {
            iconString = QString("wireless6-%1-symbolic").arg(stateString);
        } else {
            iconString = QString("wireless-%1-symbolic").arg(stateString);
        }
        break;
    }
    case PluginState::WiredConnectNoInternet: {
        stateString = "warning";
        iconString = QString("network-%1-symbolic").arg(stateString);
        break;
    }
    case PluginState::WiredFailed: {
        // 有线连接失败none变为offline
        stateString = "offline";
        iconString = QString("network-%1-symbolic").arg(stateString);
        break;
    }
    case PluginState::Unknown:
    case PluginState::Nocable: {
        stateString = "error";
        iconString = QString("network-%1-symbolic").arg(stateString);
        break;
    }
    case PluginState::WirelessIpConflicted: {
        stateString = "offline";
        iconString = QString("wireless-%1-symbolic").arg(stateString);
        break;
    }
    case PluginState::WiredIpConflicted: {
        stateString = "offline";
        iconString = QString("network-%1-symbolic").arg(stateString);
        break;
    }
    case PluginState::WirelessFailed:
    case PluginState::Failed: {
        // 无线连接失败改为 disconnect
        stateString = "disconnect";
        iconString = QString("wireless-%1").arg(stateString);
        break;
    }
    }
    return localPath + iconString;
}

void NetworkPluginHelper::refreshIcon()
{
    int colorType = m_isDarkIcon ?  DGuiApplicationHelper::ColorType::DarkType : DGuiApplicationHelper::ColorType::LightType;
    (*m_trayIcon) = icon(colorType);
    emit iconChanged();
}

void NetworkPluginHelper::setIconDark(bool isDark)
{
    m_isDarkIcon = isDark;
}

int NetworkPluginHelper::deviceCount(const DeviceType &devType) const
{
    // 获取指定的设备类型的设备数量
    int count = 0;
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    for (NetworkDeviceBase *dev : devices)
        if (dev->deviceType() == static_cast<DeviceType>(devType))
            count++;

    return count;
}

void NetworkPluginHelper::onDeviceAdded(QList<NetworkDeviceBase *> devices)
{
    // 处理新增设备的信号
    for (NetworkDeviceBase *device : devices) {
        // 当网卡连接状态发生变化的时候重新绘制任务栏的图标
        connect(device, &NetworkDeviceBase::deviceStatusChanged, this, &NetworkPluginHelper::onUpdatePlugView);

        emit addDevice(device->path());
        switch (device->deviceType()) {
        case DeviceType::Wired: {
            WiredDevice *wiredDevice = static_cast<WiredDevice *>(device);

            connect(wiredDevice, &WiredDevice::connectionAdded, this, &NetworkPluginHelper::onUpdatePlugView);
            connect(wiredDevice, &WiredDevice::connectionRemoved, this, &NetworkPluginHelper::onUpdatePlugView);
            connect(wiredDevice, &WiredDevice::connectionPropertyChanged, this, &NetworkPluginHelper::onUpdatePlugView);
            connect(wiredDevice, &NetworkDeviceBase::enableChanged, this, &NetworkPluginHelper::onUpdatePlugView);
            connect(wiredDevice, &NetworkDeviceBase::connectionChanged, this, &NetworkPluginHelper::onUpdatePlugView);
            connect(wiredDevice, &WiredDevice::activeConnectionChanged, this, &NetworkPluginHelper::onUpdatePlugView);
        } break;
        case DeviceType::Wireless: {
            WirelessDevice *wirelessDevice = static_cast<WirelessDevice *>(device);

            connect(wirelessDevice, &WirelessDevice::networkAdded, this, &NetworkPluginHelper::onUpdatePlugView);
            connect(wirelessDevice, &WirelessDevice::networkAdded, this, &NetworkPluginHelper::onAccessPointsAdded);
            connect(wirelessDevice, &WirelessDevice::networkRemoved, this, &NetworkPluginHelper::onUpdatePlugView);
            connect(wirelessDevice, &WirelessDevice::enableChanged, this, &NetworkPluginHelper::onUpdatePlugView);
            connect(wirelessDevice, &WirelessDevice::connectionChanged, this, &NetworkPluginHelper::onUpdatePlugView);
            connect(wirelessDevice, &WirelessDevice::hotspotEnableChanged, this, &NetworkPluginHelper::onUpdatePlugView);
            connect(wirelessDevice, &WirelessDevice::activeConnectionChanged, this, &NetworkPluginHelper::onActiveConnectionChanged);

            wirelessDevice->scanNetwork();
        } break;
        default:
            break;
        }
    }

    onUpdatePlugView();
}

void NetworkPluginHelper::invokeMenuItem(const QString &menuId)
{
    switch (menuId.toInt()) {
    case MenuItemKey::MenuEnable:
        setDeviceEnabled(DeviceType::Wired, true);
        if (wirelessIsActive())
            setDeviceEnabled(DeviceType::Wireless, true);
        break;
    case MenuItemKey::MenuDisable:
        setDeviceEnabled(DeviceType::Wired, false);
        if (wirelessIsActive())
            setDeviceEnabled(DeviceType::Wireless, false);
        break;
    case MenuItemKey::MenuWiredEnable:
        setDeviceEnabled(DeviceType::Wired, true);
        break;
    case MenuItemKey::MenuWiredDisable:
        setDeviceEnabled(DeviceType::Wired, false);
        break;
    case MenuItemKey::MenuWirelessEnable:
        if (wirelessIsActive())
            setDeviceEnabled(DeviceType::Wireless, true);
        break;
    case MenuItemKey::MenuWirelessDisable:
        if (wirelessIsActive())
            setDeviceEnabled(DeviceType::Wireless, false);
        break;
    case MenuItemKey::MenuSettings:
        NetworkDBusProxy::ShowPage("network");
        break;
    default:
        break;
    }
}

bool NetworkPluginHelper::needShowControlCenter()
{
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    // 如果没有网络设备，则直接唤起控制中心
    if (devices.size() == 0)
        return true;

    for (NetworkDeviceBase *device : devices) {
        if (!device->isEnabled())
            continue;

        if (device->deviceType() == DeviceType::Wired) {
            WiredDevice *wiredDevice = static_cast<WiredDevice *>(device);
            // 只要有一个有线网卡存在连接列表，就让其弹出网络列表
            if (!wiredDevice->items().isEmpty())
                return false;
        } else if (device->deviceType() == DeviceType::Wireless) {
            WirelessDevice *wirelessDevice = static_cast<WirelessDevice *>(device);
            if (!wirelessDevice->accessPointItems().isEmpty())
                return false;
        }
    }

    return true;
}

bool NetworkPluginHelper::deviceEnabled(const DeviceType &deviceType) const
{
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    for (NetworkDeviceBase *device : devices)
        if (device->deviceType() == deviceType && device->isEnabled())
            return true;

    return false;
}

void NetworkPluginHelper::setDeviceEnabled(const DeviceType &deviceType, bool enabeld)
{
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    for (NetworkDeviceBase *device : devices)
        if (device->deviceType() == deviceType)
            device->setEnabled(enabeld);
}

bool NetworkPluginHelper::wirelessIsActive() const
{
    dde::network::NetworkDBusProxy *networkInter = new dde::network::NetworkDBusProxy();
    networkInter->deleteLater();
    return !networkInter->enabled();
}

QString NetworkPluginHelper::getStrengthStateString(int strength) const
{
    if (5 >= strength)
        return "0";

    if (30 >= strength)
        return "20";

    if (55 >= strength)
        return "40";

    if (65 >= strength)
        return "60";

    return "80";
}

AccessPoints *NetworkPluginHelper::getStrongestAp() const
{
    AccessPoints *maxAps = nullptr;
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    for (NetworkDeviceBase *device : devices) {
        if (device->deviceType() != DeviceType::Wireless)
            continue;

        WirelessDevice *dev = static_cast<WirelessDevice *>(device);
        AccessPoints *ap = dev->activeAccessPoints();
        if (ap && (!maxAps || maxAps->strength() < ap->strength()))
            maxAps = ap;
    }

    return maxAps;
}

AccessPoints *NetworkPluginHelper::getConnectedAp() const
{
    AccessPoints *connectedAp = nullptr;
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    for (NetworkDeviceBase *device : devices) {
        if (device->deviceType() != DeviceType::Wireless)
            continue;

        WirelessDevice *dev = static_cast<WirelessDevice *>(device);
        AccessPoints *ap = dev->activeAccessPoints();
        if (ap && ap->connected() && (!connectedAp || connectedAp->strength() < ap->strength()))
            connectedAp = ap;
    }

    return connectedAp;
}

const QString NetworkPluginHelper::contextMenu(bool hasSetting) const
{
    int wiredCount = deviceCount(DeviceType::Wired);
    int wirelessCount = deviceCount(DeviceType::Wireless);
    bool wiredEnabled = deviceEnabled(DeviceType::Wired);
    bool wirelessEnabeld = deviceEnabled(DeviceType::Wireless);
    QList<QVariant> items;
    if (wiredCount && wirelessCount) {
        items.reserve(3);
        QMap<QString, QVariant> wireEnable;
        if (wiredEnabled) {
            wireEnable["itemId"] = QString::number(MenuWiredDisable);
            wireEnable["itemText"] = tr("Disable wired connection");
        } else {
            wireEnable["itemId"] = QString::number(MenuWiredEnable);
            wireEnable["itemText"] = tr("Enable wired connection");
        }

        wireEnable["isActive"] = true;
        items.push_back(wireEnable);

        QMap<QString, QVariant> wirelessEnable;
        if (wirelessEnabeld) {
            wirelessEnable["itemText"] = tr("Disable wireless connection");
            wirelessEnable["itemId"] = QString::number(MenuWirelessDisable);
        } else {
            wirelessEnable["itemText"] = tr("Enable wireless connection");
            wirelessEnable["itemId"] = QString::number(MenuWirelessEnable);
        }

        wirelessEnable["isActive"] = wirelessIsActive();
        items.push_back(wirelessEnable);
    } else if (wiredCount || wirelessCount) {
        items.reserve(2);
        QMap<QString, QVariant> enable;
        if (wiredEnabled || wirelessEnabeld) {
            enable["itemId"] = QString::number(MenuDisable);
            enable["itemText"] = tr("Disable network");
        } else {
            enable["itemId"] = QString::number(MenuEnable);
            enable["itemText"] = tr("Enable network");
        }

        enable["isActive"] = (wirelessCount > 0 ? wirelessIsActive() : true);
        items.push_back(enable);
    }
    if (hasSetting) {
        QMap<QString, QVariant> settings;
        settings["itemId"] = QString::number(MenuSettings);
        settings["itemText"] = tr("Network settings");
        settings["isActive"] = true;
        items.push_back(settings);
    }
    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

QWidget *NetworkPluginHelper::itemTips()
{
    return m_tipsWidget->height() == 0 ? nullptr : m_tipsWidget;
}

void NetworkPluginHelper::onUpdatePlugView()
{
    updatePluginState();
    updateTooltips();
    refreshIcon();
    emit viewUpdate();
}

void NetworkPluginHelper::onActiveConnectionChanged()
{
    WirelessDevice *wireless = static_cast<WirelessDevice *>(sender());
    DeviceStatus status = wireless->deviceStatus();
    if (status == DeviceStatus::Disconnected
        || status == DeviceStatus::Deactivation)
        return;

    QString wirelessPath = wireless->path();
    for (auto conn : NetworkManager::activeConnections()) {
        if (!conn->id().isEmpty() && conn->devices().contains(wirelessPath)) {
            NetworkManager::ConnectionSettings::Ptr connSettings = conn->connection()->settings();
            NetworkManager::WirelessSetting::Ptr wSetting = connSettings->setting(NetworkManager::Setting::SettingType::Wireless).staticCast<NetworkManager::WirelessSetting>();
            if (wSetting.isNull())
                continue;

            const QString settingMacAddress = wSetting->macAddress().toHex().toUpper();
            const QString deviceMacAddress = wireless->realHwAdr().remove(":");
            if (!settingMacAddress.isEmpty() && settingMacAddress != deviceMacAddress)
                continue;

            // 隐藏网络配置错误时提示重连
            if (wSetting && wSetting->hidden()) {
                NetworkManager::WirelessSecuritySetting::Ptr wsSetting = connSettings->setting(NetworkManager::Setting::SettingType::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
                if (wsSetting && NetworkManager::WirelessSecuritySetting::KeyMgmt::Unknown == wsSetting->keyMgmt()) {
                    for (auto ap : wireless->accessPointItems()) {
                        if (ap->ssid() == wSetting->ssid() && ap->secured() && ap->strength() > 0) {
                            // 隐藏网络逻辑是要输入密码重连，所以后端无等待，前端重连
                            m_networkDialog->setConnectWireless(wireless->path(), ap->ssid(), false);
                            break;
                        }
                    }
                }
            }
        }
    }

    onUpdatePlugView();
}

bool NetworkPluginHelper::needSetPassword(AccessPoints *accessPoint) const
{
    // 如果当前热点不是隐藏热点，或者当前热点不是加密热点，则需要设置密码（因为这个函数只是处理隐藏且加密的热点）
    if (!accessPoint->hidden() || !accessPoint->secured())
        return false;

    WirelessDevice *wirelessDevice = nullptr;
    QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
    for (NetworkDeviceBase *device : devices) {
        if (device->deviceType() == DeviceType::Wireless && device->path() == accessPoint->devicePath()) {
            wirelessDevice = static_cast<WirelessDevice *>(device);
            break;
        }
    }

    // 如果连这个连接的设备都找不到，则无需设置密码
    if (!wirelessDevice)
        return false;

    // 查找该热点对应的连接的UUID
    NetworkManager::Connection::Ptr connection;
    NetworkManager::WirelessDevice::Ptr device(new NetworkManager::WirelessDevice(wirelessDevice->path()));
    NetworkManager::Connection::List connectionlist = device->availableConnections();
    for (NetworkManager::Connection::Ptr conn : connectionlist) {
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

    NetworkManager::WirelessSecuritySetting::Ptr securitySetting =
            settings->setting(NetworkManager::Setting::SettingType::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();

    NetworkManager::WirelessSecuritySetting::KeyMgmt keyMgmt = securitySetting->keyMgmt();
    if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaNone || keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::Unknown)
        return true;

    NetworkManager::Setting::SettingType sType = NetworkManager::Setting::SettingType::WirelessSecurity;
    if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap)
        sType = NetworkManager::Setting::SettingType::Security8021x;

    QDBusPendingReply<NMVariantMapMap> reply;
    reply = connection->secrets(settings->setting(sType)->name());

    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
        return true;

    NMVariantMapMap sSecretsMapMap = reply.value();
    QSharedPointer<NetworkManager::WirelessSecuritySetting> setting = settings->setting(sType).staticCast<NetworkManager::WirelessSecuritySetting>();
    setting->secretsFromMap(sSecretsMapMap.value(setting->name()));

    if (securitySetting.isNull())
        return true;

    QString psk;
    switch (keyMgmt) {
    case NetworkManager::WirelessSecuritySetting::KeyMgmt::Wep:
        psk = securitySetting->wepKey0();
        break;
    case NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaPsk:
    default:
        psk = securitySetting->psk();
        break;
    }

    // 如果该密码存在，则无需调用设置密码信息
    return psk.isEmpty();
}

void NetworkPluginHelper::handleAccessPointSecure(AccessPoints *accessPoint)
{
    if (needSetPassword(accessPoint))
        m_networkDialog->setConnectWireless(accessPoint->devicePath(), accessPoint->ssid());
}

void NetworkPluginHelper::onAccessPointsAdded(QList<AccessPoints *> newAps)
{
    for (AccessPoints *newAp : newAps) {
        connect(newAp, &AccessPoints::securedChanged, this, [ this, newAp ] {
            handleAccessPointSecure(newAp);
        });
        handleAccessPointSecure(newAp);
    }
}
