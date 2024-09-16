// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "activeaccoutnetwork.h"
#include "account.h"
#include "networkactivator.h"
#include "accountnetworkconfig.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusMetaType>
#include <QDBusServiceWatcher>
#include <QDBusConnectionInterface>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>

using namespace accountnetwork::sessionservice;

#define SYSTEMNETWORKSERVICE "org.deepin.service.SystemNetwork"
#define SYSTEMNETWORKPATH "/org/deepin/service/SystemNetwork"
#define SYSTEMNETWORKINTERFACE "org.deepin.service.AccountSystemNetwork"

#define NETWORKMANAGERINTERFACE "org.freedesktop.NetworkManager"

#define DEAMON_NETWORK_SERVICE "com.deepin.daemon.Network"
#define SECRET_SERVICE "org.freedesktop.secrets"

#define MANULCONNECTION 1

ActiveAccountNetwork::ActiveAccountNetwork(QObject *parent)
    : QObject(parent)
    , m_account(new Account(this))
    , m_config(new AccountNetworkConfig(this))
    , m_networkActivator(new NetworkActivator(m_config, this))
    , m_networkregisted(false)
    , m_secretregisted(false)
{
    init();
    initConnection();
    initDevice();
}

void ActiveAccountNetwork::init()
{
    qDBusRegisterMetaType<QMap<QString, QString>>();
    activeNetwork(accountNetwork(), authenInfo());
}

void ActiveAccountNetwork::initConnection()
{
    // 连接SystemNetwork服务，获取请求连接的信号
    QDBusConnection::systemBus().connect(SYSTEMNETWORKSERVICE, SYSTEMNETWORKPATH, SYSTEMNETWORKINTERFACE, "requestAuthen", this, SLOT(onAuthen(const QVariantMap &)));
    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(this);
    serviceWatcher->setConnection(QDBusConnection::systemBus());
    serviceWatcher->addWatchedService(NETWORKMANAGERINTERFACE);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [](const QString &service) {
        if (service == NETWORKMANAGERINTERFACE) {
            // 网络服务停止后，删除本地配置文件
            QString configpath = QString("%1/network/manulconnection").arg(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation)[0]).path());
            if (QFile::exists(configpath)) {
                QFile::remove(configpath);
            }
        }
    });
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, [ this ](const QString &uni) {
        NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(uni);
        if (device && (device->type() == NetworkManager::Device::Type::Ethernet
                       || device->type() == NetworkManager::Device::Type::Wifi)) {
            // 这里只处理有线网络和无线网络
            addDevice(device);
        }
    });
    // 如果当前密钥环没有初始化，则等待它信号发送
    QDBusConnection::sessionBus().connect("org.freedesktop.secrets", "/org/freedesktop/secrets",
                                          "org.freedesktop.Secret.Service", "CollectionCreated",
                                          this, SLOT(onCollectionCreated(const QDBusObjectPath &)));
}

void ActiveAccountNetwork::initDevice()
{
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device : devices) {
        // 这里只处理有线网络和无线网络
        if (device->type() != NetworkManager::Device::Type::Wifi
                && device->type() != NetworkManager::Device::Type::Ethernet)
            continue;

        addDevice(device);
    }
}

void ActiveAccountNetwork::addDevice(const QSharedPointer<NetworkManager::Device> &device)
{
    connect(device.data(), &NetworkManager::Device::activeConnectionChanged, this, [ this, device ] {
        NetworkManager::ActiveConnection::Ptr activeConnection = device->activeConnection();
        if (activeConnection.isNull())
            return;

        connect(activeConnection.data(), &NetworkManager::ActiveConnection::stateChanged, activeConnection.data(), [ this, device ](NetworkManager::ActiveConnection::State state) {
            // 连接成功或者失败后，如果当前网络不是当前用户连接的网络，并且不是手动连接的网络，则断开并连接当前用户自己的网络
            if (state != NetworkManager::ActiveConnection::State::Activated
                    && state != NetworkManager::ActiveConnection::State::Deactivated)
                return;

            // 此处使用当前设备正在使用的活动的连接网络，这样避免了上一个网络在断开的时候，下方的判断中还是使用上一个网络的信息导致出错
            NetworkManager::ActiveConnection::Ptr activeConnection = device->activeConnection();
            if (activeConnection.isNull())
                return;

            onConnectionStateChanged(device, activeConnection);
        }, Qt::UniqueConnection);
    }, Qt::UniqueConnection);
    // 如果是无线网卡，则处理无线网络回复的重连
    if (device->type() == NetworkManager::Device::Type::Wifi) {
        NetworkManager::WirelessDevice::Ptr wirelessDevice = device.staticCast<NetworkManager::WirelessDevice>();
        connect(wirelessDevice.data(), &NetworkManager::WirelessDevice::networkAppeared, wirelessDevice.data(), [ wirelessDevice, this ](const QString &ssid) {
            // 如果当前账户不是活动账户，则无需处理
            if (m_account->activeAccount() != m_account->name())
                return;

            // 如果当前的网络不是用户的私有网络，则无需重连
            QMap<QString, QString> networks = accountNetwork();
            if (!networks.contains(wirelessDevice->interfaceName())
                    || networks.value(wirelessDevice->interfaceName()) != ssid)
                return;

            // 连接当前用户的网络
            m_networkActivator->activeNetwork(wirelessDevice, ssid, authenInfo());
        });
    }
}

QMap<QString, QString> ActiveAccountNetwork::accountNetwork() const
{
    QDBusInterface dbusInter(SYSTEMNETWORKSERVICE, SYSTEMNETWORKPATH, SYSTEMNETWORKINTERFACE, QDBusConnection::systemBus());
    QDBusPendingCall reply = dbusInter.asyncCall("accountNetwork");
    reply.waitForFinished();
    QDBusPendingReply<QMap<QString, QString>> replyResult = reply.reply();
    return replyResult.value();
}

QVariantMap ActiveAccountNetwork::authenInfo() const
{
    QDBusInterface dbusInter(SYSTEMNETWORKSERVICE, SYSTEMNETWORKPATH, SYSTEMNETWORKINTERFACE, QDBusConnection::systemBus());
    QDBusPendingCall reply = dbusInter.asyncCall("getAuthenInfo");
    reply.waitForFinished();
    QDBusPendingReply<QVariantMap> replyResult = reply.reply();
    return replyResult.value();
}

bool ActiveAccountNetwork::canResetCurrentNetwork(const QSharedPointer<NetworkManager::Device> &device, const QMap<QString, QString> &network, const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection) const
{
    // 如果当前用户没有指定连接网络，则无需重连
    if (!network.contains(device->interfaceName()))
        return false;
    // 如果该连接就是当前的连接，则无需再次连接
    QString networkId = network.value(device->interfaceName());
    if (device->type() == NetworkManager::Device::Type::Ethernet) {
        // 有线网络，判断连接的名称，如果连接名称相同就代表当前连接的是相同的网络，无需重连
        if (networkId == activeConnection->connection()->settings()->id())
            return false;
    } else if (device->type() == NetworkManager::Device::Type::Wifi) {
        // 无线网络，判断ssid是否相同，如果ssid相同，则无需重新连接
        NetworkManager::WirelessSetting::Ptr wirelessSetting =
                activeConnection->connection()->settings()->setting(NetworkManager::Setting::SettingType::Wireless)
                .dynamicCast<NetworkManager::WirelessSetting>();
        if (wirelessSetting && wirelessSetting->ssid() == networkId)
            return false;
    } else {
        return false;
    }

    // 如果标记为手动，则无需自动连接(手动连接的时候，这个flags值为1)
    return activeConnection->flags() != MANULCONNECTION;
}

void ActiveAccountNetwork::onConnectionStateChanged(const QSharedPointer<NetworkManager::Device> &device, const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection)
{
    // 如果当前活动用户不是当前用户，则不做任何处理
    if (m_account->name() != m_account->activeAccount())
        return;

    // 如果当前活动连接不允许断开，则无需连接
    QMap<QString, QString> network = accountNetwork();
    qDebug() << device->interfaceName() << "connection state changed:" << activeConnection->connection()->name()
             << "current network:" << network;
    // 处理连接成功的情况，在连接成功后，需要告诉system服务当前的连接的用户名和密码，方便下次回来的时候恢复当前的用户信息
    handlerConnectionSuccess(device, activeConnection);
    if (!canResetCurrentNetwork(device, network, activeConnection))
        return;

    // 连接当前用户的网络
    QString ssid = network[device->interfaceName()];
    qDebug() << "active network" << ssid;
    m_networkActivator->activeNetwork(device, ssid, authenInfo());
}

void ActiveAccountNetwork::handlerConnectionSuccess(const QSharedPointer<NetworkManager::Device> &device, const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection)
{
    // 域账户无需保存当前的验证信息
    if (m_account->isIam())
        return;

    // 此处只处理连接成功的情况
    if (device->state() != NetworkManager::Device::State::Activated)
        return;

    QString networkId;
    QString identity;
    QString password;
    if (device->type() == NetworkManager::Device::Type::Ethernet) {
        // 有线网络，判断连接的名称，如果连接名称相同就代表当前连接的是相同的网络，无需重连
        NetworkManager::Connection::Ptr connection = activeConnection->connection();
        NetworkManager::Security8021xSetting::Ptr sxSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
        if (sxSetting && !sxSetting->eapMethods().isEmpty()) {
            QDBusPendingReply<NMVariantMapMap> reply = connection->secrets(connection->settings()->setting(NetworkManager::Setting::SettingType::Security8021x)->name());
            reply.waitForFinished();
            NMVariantMapMap sSecretsMap = reply.value();
            NetworkManager::Security8021xSetting::Ptr setting = connection->settings()->setting(NetworkManager::Setting::SettingType::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
            if (setting) {
                networkId = connection->settings()->id();
                setting->secretsFromMap(sSecretsMap.value(setting->name()));
                identity = setting->identity();
                password = setting->password();
            }
        }
    } else if (device->type() == NetworkManager::Device::Type::Wifi) {
        // 无线网络，判断ssid是否相同，如果ssid相同，则获取连接的密码
        NetworkManager::WirelessSetting::Ptr wirelessSetting =
                activeConnection->connection()->settings()->setting(NetworkManager::Setting::SettingType::Wireless)
                .dynamicCast<NetworkManager::WirelessSetting>();
        // 获取密码
        NetworkManager::Connection::Ptr connection = activeConnection->connection();
        NetworkManager::WirelessSecuritySetting::Ptr sxSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
        if (sxSetting && sxSetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap) {
            QDBusPendingReply<NMVariantMapMap> reply = connection->secrets(connection->settings()->setting(NetworkManager::Setting::SettingType::Security8021x)->name());
            reply.waitForFinished();
            NMVariantMapMap sSecretsMap = reply.value();
            NetworkManager::Security8021xSetting::Ptr setting = connection->settings()->setting(NetworkManager::Setting::SettingType::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
            if (setting) {
                networkId = wirelessSetting->ssid();
                setting->secretsFromMap(sSecretsMap.value(setting->name()));
                identity = setting->identity();
                password = setting->password();
            }
        }
    } else {
        return;
    }

    // 调用这个函数用来保存当前非域账户（认为它是管理员账户）下已经连接成功的网络的认证信息，包含用户名和密码，密码以密文的形式保存
    m_config->saveNetworkCertity(device->interfaceName(), networkId, identity, password);
}

void ActiveAccountNetwork::activeNetwork(const QMap<QString, QString> &network, const QVariantMap &authenInfo)
{
    m_networkregisted = QDBusConnection::sessionBus().interface()->isServiceRegistered(DEAMON_NETWORK_SERVICE);
    m_secretregisted = QDBusConnection::sessionBus().interface()->isServiceRegistered(SECRET_SERVICE);
    if (m_networkregisted && m_secretregisted) {
        qDebug() << "Network and secret service is start";
        if (secretIsPrepare()) {
            // 获取需要连接的网络的信息
            qDebug() << "secret is prepare,start to authen";
            m_networkActivator->activeNetwork(network, authenInfo);
        } else {
            // 如果服务尚未准备完毕，则临时记录认证信息，等待服务准备完成
            qWarning() << "secret is not prepare, wait for it prepared";
            m_network = network;
            m_authenInfo = authenInfo;
        }
    } else {
        QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(this);
        serviceWatcher->setConnection(QDBusConnection::sessionBus());
        if (!m_networkregisted) {
            qWarning() << "com.deepin.daemon.Network not start,wait to it start";
            serviceWatcher->addWatchedService(DEAMON_NETWORK_SERVICE);
        }
        if (!m_secretregisted) {
            qWarning() << "org.freedesktop.secrets not start,wait to it start";
            serviceWatcher->addWatchedService(SECRET_SERVICE);
        }
        connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [ network, authenInfo, this ](const QString &service) {
            qDebug() << service << "is start";
            if (service == DEAMON_NETWORK_SERVICE) {
                m_networkregisted = true;
            } else if (service == SECRET_SERVICE) {
                m_secretregisted = true;
            }
            // 如果secret服务和network服务已经准备完毕，则无需等待该服务
            if (m_networkregisted && m_secretregisted) {
                qDebug() << "network service and secret service is registed";
                // 如果密钥环获取的default值不为/，则认为准备就绪，可以进行网络连接了(如果密钥环中的值为/，则无法保存密码导致无法连接)
                if (secretIsPrepare()) {
                    qDebug() << "active network";
                    m_networkActivator->activeNetwork(network, authenInfo);
                } else {
                    // 如果服务尚未准备完毕，则临时记录认证信息，等待服务准备完成
                    qWarning() << "secret service is not prepare, wait for it prepared";
                    m_network = network;
                    m_authenInfo = authenInfo;
                }
            }
        });
    }
}

bool ActiveAccountNetwork::secretIsPrepare() const
{
    QDBusInterface dbusInterface("org.freedesktop.secrets", "/org/freedesktop/secrets", "org.freedesktop.Secret.Service", QDBusConnection::sessionBus());
    QDBusPendingCall replyCall = dbusInterface.asyncCall("ReadAlias", "default");
    replyCall.waitForFinished();
    QDBusPendingReply<QDBusObjectPath> replyValue = replyCall.reply();
    return replyValue.value().path() != "/";
}

void ActiveAccountNetwork::onAuthen(const QVariantMap &identity)
{
    if (m_account->activeAccount() != m_account->name())
        return;

    // 连接当前用户的网络
    QMap<QString, QString> network = accountNetwork();
    qDebug() << "start authen network" << network;
    activeNetwork(network, identity);
}

void ActiveAccountNetwork::onCollectionCreated(const QDBusObjectPath &path)
{
    qDebug() << "secret service collection created,networkregisted" << m_networkregisted << "path" << path.path();
    if (secretIsPrepare() && m_networkregisted) {
        m_networkActivator->activeNetwork(m_network, m_authenInfo);
        m_network.clear();
        m_authenInfo.clear();
    }
}
