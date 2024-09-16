// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "networkinitialization.h"
#include "settingconfig.h"

#include <QApplication>

#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WiredSetting>

#include <com_deepin_daemon_accounts_user.h>

using namespace network::systemservice;

// the interface is enabled from the administrative point of view. Corresponds to kernel IFF_UP.
#define DEVICE_INTERFACE_FLAG_UP 0x1

Q_LOGGING_CATEGORY(DNC, "org.deepin.service.SystemNetwork");

#define NETWORKMANAGER_SERVICE "org.freedesktop.NetworkManager"

void NetworkInitialization::doInit()
{
    static NetworkInitialization init;
}

NetworkInitialization::NetworkInitialization(QObject *parent)
    : QObject(parent)
    , m_initilized(false)
{
    installTranslator(QLocale::system().name());
    initDeviceInfo();
    initConnection();
}

void NetworkInitialization::initDeviceInfo()
{
    // 处理无线网络
    if (QDBusConnection::systemBus().interface()->isServiceRegistered("org.desktopspec.ConfigManager")) {
        qDebug() << "ConfigManager is start";
        onInitDeviceConnection();
    } else {
        qWarning() << "ConfigManager is not start, wait for it start";
        QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(this);
        serviceWatcher->setConnection(QDBusConnection::sessionBus());
        serviceWatcher->addWatchedService("org.desktopspec.ConfigManager");
        connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &NetworkInitialization::onInitDeviceConnection);
    }
}

void NetworkInitialization::initConnection()
{
    QDBusMessage lock = QDBusMessage::createMethodCall("com.deepin.dde.LockService", "/com/deepin/dde/LockService", "com.deepin.dde.LockService", "CurrentUser");
    QDBusConnection::systemBus().callWithCallback(lock, this, SLOT(onUserChanged(QString)));
    QDBusConnection::systemBus().connect("com.deepin.dde.LockService", "/com/deepin/dde/LockService", "com.deepin.dde.LockService", "UserChanged", this, SLOT(onUserChanged(QString)));
}

void NetworkInitialization::addFirstConnection(const QSharedPointer<NetworkManager::WiredDevice> &device)
{
    if (!device)
        return;

    // 如果设备不被管理，而且被down掉，或者没有插入网线的情况下，是无需创建连接的
    if (!device->managed() || !(device->interfaceFlags() & DEVICE_INTERFACE_FLAG_UP)
            || !device->carrier()) {
        return;
    }

    // 先查找当前的设备下是否存在有线连接，如果不存在，则直接新建一个，因为按照要求是至少要有一个有线连接
    NetworkManager::Connection::List unSaveConnections;
    bool findConnection = hasConnection(device, unSaveConnections);
    // 按照需求，需要将未保存的连接删除
    for (const NetworkManager::Connection::Ptr &conn : unSaveConnections)
        conn->remove();

    if (findConnection || m_creatingDevices.contains(device->uni()))
        return;

    m_creatingDevices << device->uni();

    // 如果发现当前的连接的数量为空,则自动创建以当前语言为基础的连接
    QString matchName = connectionMatchName();
    m_newConnectionNames << matchName;
    NetworkManager::ConnectionSettings::Ptr conn(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::ConnectionType::Wired));
    conn->setId(matchName);
    conn->setUuid(conn->createNewUuid());
    conn->setInterfaceName(device->interfaceName());
    conn->setAutoconnect(!SettingConfig::instance()->enableAccountNetwork());
    NetworkManager::WiredSetting::Ptr wiredSetting = conn->setting(NetworkManager::Setting::Wired).staticCast<NetworkManager::WiredSetting>();
    QString macAddress = device->permanentHardwareAddress();
    macAddress.remove(":");
    wiredSetting->setMacAddress(QByteArray::fromHex(macAddress.toUtf8()));
    wiredSetting->setInitialized(true);
    QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::addConnection(conn->toMap());
    reply.waitForFinished();
}

bool NetworkInitialization::hasConnection(const QSharedPointer<NetworkManager::WiredDevice> &device, QList<QSharedPointer<NetworkManager::Connection> > &unSaveDevices)
{
    if (device.isNull())
        return false;

    bool hasConnection = false;
    QStringList uuids;
    NetworkManager::Connection::List connList = NetworkManager::listConnections();
    for (const NetworkManager::Connection::Ptr &conn : connList) {
        uuids << conn->uuid();
    }

    for (const NetworkManager::Connection::Ptr &conn : device->availableConnections()) {
        if (uuids.contains(conn->uuid()))
            continue;

        connList << conn;
    }

    for (const NetworkManager::Connection::Ptr &conn : connList) {
        NetworkManager::WiredSetting::Ptr settings = conn->settings()->setting(NetworkManager::Setting::Wired).staticCast<NetworkManager::WiredSetting>();
        // 如果当前连接的MAC地址不为空且连接的MAC地址不等于当前设备的MAC地址，则认为不是当前的连接，跳过
        if (settings.isNull() || (!settings->macAddress().isEmpty() && device->permanentHardwareAddress().compare(settings->macAddress().toHex(':'), Qt::CaseInsensitive) != 0))
            continue;

        // 将未保存的连接放入到列表中，供外面调用删除
        if (conn->isUnsaved()) {
            unSaveDevices << conn;
            continue;
        }

        // 只要走到这里，就认为当前网卡存在有线连接，就不再继续
        hasConnection = true;
    }

    return hasConnection;
}

QString NetworkInitialization::connectionMatchName() const
{
    NetworkManager::Connection::List connList = NetworkManager::listConnections();
    QStringList connNameList = m_newConnectionNames;

    for (const NetworkManager::Connection::Ptr &conn : connList) {
        if (conn->settings()->connectionType() == NetworkManager::ConnectionSettings::ConnectionType::Wired
                && !connNameList.contains(conn->name()))
            connNameList.append(conn->name());
    }

    QString matchConnName = tr("Wired Connection");
    if (!connNameList.contains(matchConnName))
        return matchConnName;

    int connSuffixNum = 1;
    matchConnName = QString(tr("Wired Connection")) + QString(" %1");
    for (int i = 1; i <= connNameList.size(); ++i) {
        if (!connNameList.contains(matchConnName.arg(i))) {
            connSuffixNum = i;
            break;
        }
        if (i == connNameList.size())
            connSuffixNum = i + 1;
    }

    return matchConnName.arg(connSuffixNum);
}

void NetworkInitialization::installTranslator(const QString &locale)
{
    static QTranslator translator;
    static QString localTmp;
    if (locale.isEmpty() || localTmp == locale) {
        return;
    }
    localTmp = locale;
    QApplication::removeTranslator(&translator);
    const QString qmFile = QString("%1/network-service-plugin_%2.qm").arg(QM_FILES_DIR).arg(locale);
    translator.load(qmFile);
    QApplication::installTranslator(&translator);
}

void NetworkInitialization::hideWirelessDevice(const QSharedPointer<NetworkManager::Device> &device, bool disableNetwork)
{
    if (!disableNetwork)
        return;

    bool managed = !disableNetwork;
    qDebug() << "device" << device->interfaceName() << "manager" << device->managed();
    if (device->managed() != managed) {
        QDBusInterface dbusInter("org.freedesktop.NetworkManager", device->uni(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
        dbusInter.setProperty("Managed", managed);
    }
    connect(device.data(), &NetworkManager::Device::managedChanged, this, [ device, managed ] {
        if (device->managed() == managed)
            return ;

        qDebug() << "device" << device->interfaceName() << "managed changed" << device->managed() << ", will set it managed" << managed;
        QDBusInterface dbusInter("org.freedesktop.NetworkManager", device->uni(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
        dbusInter.setProperty("Managed", managed);
    }, Qt::UniqueConnection);
}

void NetworkInitialization::initDeviceConnection(const QSharedPointer<NetworkManager::WiredDevice> &device)
{
    connect(device.data(), &NetworkManager::WiredDevice::interfaceFlagsChanged, this, [ this, device ] {
        addFirstConnection(device);
    }, Qt::UniqueConnection);
    connect(device.data(), &NetworkManager::WiredDevice::managedChanged, this, [ this, device ] {
        addFirstConnection(device);
    }, Qt::UniqueConnection);
    connect(device.data(), &NetworkManager::WiredDevice::carrierChanged, this, [ this, device ] {
        addFirstConnection(device);
    }, Qt::UniqueConnection);
    connect(device.data(), &NetworkManager::WiredDevice::availableConnectionAppeared, this, [ this, device ] {
        m_creatingDevices.removeAll(device->uni());
    }, Qt::UniqueConnection);
    connect(device.data(), &NetworkManager::WiredDevice::availableConnectionDisappeared, this, [ this, device ] {
        m_creatingDevices.removeAll(device->uni());
    }, Qt::UniqueConnection);
}

void NetworkInitialization::onInitDeviceConnection()
{
    QList<NetworkManager::WirelessDevice::Ptr> wirelessDevices;
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device: devices) {
        if (device->type() == NetworkManager::Device::Type::Wifi) {
            wirelessDevices << device.staticCast<NetworkManager::WirelessDevice>();
        } else if (device->type() == NetworkManager::Device::Type::Ethernet) {
            qDebug() << "Wired device" << device->interfaceName() << "initilized" << m_initilized << ",add first connection";
            if (m_initilized) {
                NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
                initDeviceConnection(wiredDevice);
                addFirstConnection(wiredDevice);
            }
        }
    }

    bool disableNetwork = SettingConfig::instance()->disableNetwork();
    qDebug() << "disbled wireless network" << disableNetwork;
    for (const NetworkManager::WirelessDevice::Ptr &device : wirelessDevices) {
        hideWirelessDevice(device, disableNetwork);
    }
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, [ disableNetwork, this ](const QString &uni) {
        NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(uni);
        // 这里只处理无线网卡
        if (!device)
            return;

        switch (device->type()) {
        case NetworkManager::Device::Type::Wifi:
            hideWirelessDevice(device, disableNetwork);
            break;
        case NetworkManager::Device::Type::Ethernet: {
            qDebug() << "new Wired device" << device->interfaceName() << "initilized" << m_initilized << ",add first connection";
            if (m_initilized) {
                NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
                initDeviceConnection(wiredDevice);
                addFirstConnection(wiredDevice);
            }
            break;
        }
        default:
            break;
        }
    }, Qt::UniqueConnection);
}

void NetworkInitialization::onUserChanged(const QString &json)
{
    do {
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        if (!doc.isObject())
            break;
        int uid = doc.object().value("Uid").toInt();
        QDBusInterface user("org.deepin.dde.Accounts1", QString("/org/deepin/dde/Accounts1/User%1").arg(uid), "org.deepin.dde.Accounts1.User", QDBusConnection::systemBus());
        installTranslator(user.property("Locale").toString().split(".").first());
    } while (0);

    if (!m_initilized) {
        m_initilized = true;
        qDebug() << "add wired device connection";
        NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
        for (NetworkManager::Device::Ptr device : devices) {
            if (device->type() != NetworkManager::Device::Type::Ethernet)
                continue;

            qDebug() << "device" << device->interfaceName() << "add first connection";
            NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
            initDeviceConnection(wiredDevice);
            addFirstConnection(wiredDevice);
        }
    }
}
