// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "networkinitialization.h"
#include "settingconfig.h"
#include "systemservice.h"

#include <QApplication>
#include <QEventLoop>

#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WiredSetting>

#include <com_lingmo_daemon_accounts_user.h>
#include <com_lingmo_daemon_accounts.h>

using namespace network::systemservice;

// the interface is enabled from the administrative point of view. Corresponds to kernel IFF_UP.
#define DEVICE_INTERFACE_FLAG_UP 0x1

#define NETWORKMANAGER_SERVICE "org.freedesktop.NetworkManager"

#define LOCKERVICE "com.lingmo.ocean.LockService"
#define LOCKPATH "/com/lingmo/ocean/LockService"
#define LOCKINTERFACE "com.lingmo.ocean.LockService"

#define DEAMONACCOUNTSERVICE "com.lingmo.daemon.Accounts"
#define DEAMONACCOUNTPATH "/com/lingmo/daemon/Accounts"
#define DEAMONACCOUNTINTERFACE "com.lingmo.daemon.Accounts"

void NetworkInitialization::doInit()
{
    static NetworkInitialization init;
}

NetworkInitialization::NetworkInitialization(QObject *parent)
    : QObject(parent)
    , m_initilized(false)
    , m_accountServiceRegister(false)
    , m_hasAddFirstConnection(false)
{
    initDeviceInfo();
    initConnection();
}

void NetworkInitialization::initDeviceInfo()
{
    // 处理无线网络
    if (QDBusConnection::systemBus().interface()->isServiceRegistered("org.desktopspec.ConfigManager")) {
        qCDebug(DSM) << "ConfigManager is start";
        onInitDeviceConnection();
    } else {
        qCWarning(DSM) << "ConfigManager is not start, wait for it start";
        QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(this);
        serviceWatcher->setConnection(QDBusConnection::sessionBus());
        serviceWatcher->addWatchedService("org.desktopspec.ConfigManager");
        connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &NetworkInitialization::onInitDeviceConnection);
    }
}

void NetworkInitialization::initConnection()
{
    QDBusMessage lock = QDBusMessage::createMethodCall(LOCKERVICE, LOCKPATH, LOCKINTERFACE, "CurrentUser");
    QDBusConnection::systemBus().callWithCallback(lock, this, SLOT(onUserChanged(QString)));
    QDBusConnection::systemBus().connect(LOCKERVICE, LOCKPATH, LOCKINTERFACE, "UserChanged", this, SLOT(onUserChanged(QString)));
    QDBusConnection::systemBus().connect(DEAMONACCOUNTSERVICE, DEAMONACCOUNTPATH, DEAMONACCOUNTINTERFACE, "UserAoceand", this, SLOT(onUserAoceand(QString)));

    m_accountServiceRegister = QDBusConnection::systemBus().interface()->isServiceRegistered(DEAMONACCOUNTINTERFACE);
    if (!m_accountServiceRegister) {
        // 如果服务未启动，则等待服务启动
        QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(this);
        serviceWatcher->setConnection(QDBusConnection::systemBus());
        qCWarning(DSM) << m_accountServiceRegister << "service is not register";
        serviceWatcher->addWatchedService(DEAMONACCOUNTINTERFACE);
        connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [ this ](const QString &service) {
            if (service != DEAMONACCOUNTSERVICE)
                return;

            qCDebug(DSM) << service << "register, initilized" << m_initilized;
            m_accountServiceRegister = true;

            // 如果需要初始化设备，则初始化设备
            checkAccountStatus();
            if (m_initilized) {
                addFirstConnection();
            }
        });
    }
    // 等待3秒过后再执行一次创建的动作，正常情况下，在500毫秒内系统肯定会发送当前账户变化的信号，
    // 但是在有的系统上没有当前账户的信息，一直不发送当前账户变化的信息，这种情况下就自己强制创建一个连接，默认就英文吧
    QTimer::singleShot(3000, this, [this] {
        // 3秒过后直接调用addFirstConnection函数，在这个函数中，如果连接已经创建了，就不再创建
        // 如果没有创建过连接，那么就执行创建的操作
        // 调用checkAccountStatus检查当前用户状态并安装当前用户的语言
        qCDebug(DSM) << "check connection status";
        checkAccountStatus();
        if (!m_initilized)
            qCWarning(DSM) << "can not found current user, used default lauguage to create connection";
        // 不管语言有没有安装上，直接添加新连接，如果语言没有安装上，这个时候肯定不会有当前用户的语言了，此时安装就安装默认的英文
        // 如果语言安装上了，此时就使用已经安装的语言来新建连接
        addFirstConnection();
    });
}

void NetworkInitialization::addFirstConnection()
{
    qCDebug(DSM) << "add wired device connection: has add" << m_hasAddFirstConnection;

    if (m_hasAddFirstConnection) {
        return;
    }
    m_hasAddFirstConnection = true;
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device : devices) {
        if (device->type() != NetworkManager::Device::Type::Ethernet)
            continue;

        qCDebug(DSM) << "device" << device->interfaceName() << "add first connection";
        NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
        initDeviceConnection(wiredDevice);
        addFirstConnection(wiredDevice);
    }
}

void NetworkInitialization::addFirstConnection(const QSharedPointer<NetworkManager::WiredDevice> &device)
{
    if (!device)
        return;

    // 如果设备不被管理，而且被down掉，或者没有插入网线的情况下，是无需创建连接的
    qCDebug(DSM) << "device:" << device->interfaceName() << "managed:" << device->managed()
             << "interfaceFlags:" << device->interfaceFlags() << "carrier:" << device->carrier();
    if (!device->managed() || !(device->interfaceFlags() & DEVICE_INTERFACE_FLAG_UP)
            || !device->carrier()) {
        return;
    }

    static QMutex lock;
    QMutexLocker locker(&lock);

    auto createConnection = [this, device] {
        // 先查找当前的设备下是否存在有线连接，如果不存在，则直接新建一个，因为按照要求是至少要有一个有线连接
        NetworkManager::Connection::List unSaveConnections;
        bool findConnection = hasConnection(device, unSaveConnections);
        // 按照需求，需要将未保存的连接删除
        for (const NetworkManager::Connection::Ptr &conn : unSaveConnections)
            conn->remove();

        qCDebug(DSM) << "find connection :" << findConnection << "current device:" << device->uni();

        if (findConnection)
            return;

        // 记录当前设备创建连接的时间，在下一次为该设备创建连接的时候，会等待5秒，否则会出现创建的连接的
        m_lastCreateTime[device->interfaceName()] = QDateTime::currentDateTime();
        // 如果发现当前的连接的数量为空,则自动创建以当前语言为基础的连接
        QString matchName = connectionMatchName();
        m_newConnectionNames << matchName;
        qCDebug(DSM) << "device:" << device->interfaceName() << "start create first connection" << matchName;
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
        qCDebug(DSM) << "device" << device->interfaceName() << "create connection success,count:" << device->availableConnections().size();
    };
    if (m_lastCreateTime.contains(device->interfaceName())) {
        QDateTime lastCreateTime = m_lastCreateTime.value(device->interfaceName());
        qint64 passmseconds = lastCreateTime.msecsTo(QDateTime::currentDateTime());
        qCDebug(DSM) << "last create connection time" << lastCreateTime << ", pass time" << passmseconds << "millisecond";
        // 获取上一次为该设备创建的时间，如果是5秒钟之前，则直接调用创建的流程，否则，在上一次创建连接后等待5秒再进行创建
        // 这样做的目的是会出现这个连接同时间会多次进入这个函数进行创建，上一次的创建动作刚完成，此时设备上还没有新创建的连接
        // 但是就进行了下一次的创建，下一次的创建中此时无法检测上一次创建的连接，会导致重复创建连接，因此，
        // 此处等待至少5秒后再进行第二次连接的创建，这样有足够的时间保证了上一次创建的连接当前设备可以检测到
        if (passmseconds >= 5000) {
            createConnection();
        } else {
            QTimer::singleShot(5000 - passmseconds, this, createConnection);
        }
    } else {
        createConnection();
    }
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
        // 如果UUID为空，则认为是空连接，不是本网卡的连接
        if (conn->settings()->uuid().isEmpty())
            continue;

        // 如果网卡名称存在且不为空，那么则判断配置中的名称是不是网卡的名称
        QString interfaceName = conn->settings()->interfaceName();
        if (!interfaceName.isEmpty() && interfaceName != device->interfaceName())
            continue;

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

bool NetworkInitialization::installUserTranslator(const QString &json)
{
    if (m_initilized) {
        qCDebug(DSM) << "environment is initialized";
        return true;
    }

    qCDebug(DSM) << "user changed " << json;
    QString locale;
    if (json.startsWith("/")) {
        com::lingmo::daemon::accounts::User user("com.lingmo.daemon.Accounts", json, QDBusConnection::systemBus());
        locale = user.locale().split(".").first().trimmed();
        qCDebug(DSM) << "get locale: " << locale;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (!locale.isEmpty() ) {
        //do nothing
    } else if (error.error == QJsonParseError::NoError && doc.isObject()) {
        int uid = doc.object().value("Uid").toInt();
        com::lingmo::daemon::accounts::User user("com.lingmo.daemon.Accounts", QString("/com/lingmo/daemon/Accounts/User%1").arg(uid), QDBusConnection::systemBus());
        locale = user.locale().split(".").first();
    } else if (m_accountServiceRegister) {
        // 如果是非法的json，就直接从Accounts服务中获取
        com::lingmo::daemon::Accounts account("com.lingmo.daemon.Accounts", "/com/lingmo/daemon/Accounts", QDBusConnection::systemBus());
        const QStringList userList = account.userList();
        qCDebug(DSM) << "found users" << userList;
        if (userList.isEmpty())
            return false;

        com::lingmo::daemon::accounts::User user("com.lingmo.daemon.Accounts", userList.first(), QDBusConnection::systemBus());
        locale = user.locale().split(".").first();
    } else {
        return false;
    }

    qCDebug(DSM) << "account locale" << locale;
    if (locale.isEmpty()) {
        return false;
    }

    static QString localTmp;

    if (localTmp != locale) {
        localTmp = locale;
        static QTranslator translator;
        QApplication::removeTranslator(&translator);
        const QString qmFile = QString("%1/network-service-plugin_%2.qm").arg(QM_FILES_DIR).arg(locale);
        translator.load(qmFile);
        QApplication::installTranslator(&translator);
        qCDebug(DSM) << "install translation file" << qmFile;
    }

    return true;
}

void NetworkInitialization::hideWirelessDevice(const QSharedPointer<NetworkManager::Device> &device, bool disableNetwork)
{
    if (!disableNetwork)
        return;

    bool managed = !disableNetwork;
    qCDebug(DSM) << "device" << device->interfaceName() << "manager" << device->managed();
    if (device->managed() != managed) {
        QDBusInterface dbusInter("org.freedesktop.NetworkManager", device->uni(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
        dbusInter.setProperty("Managed", managed);
    }
    connect(device.data(), &NetworkManager::Device::managedChanged, this, [ device, managed ] {
        if (device->managed() == managed)
            return ;

        qCDebug(DSM) << "device" << device->interfaceName() << "managed changed" << device->managed() << ", will set it managed" << managed;
        QDBusInterface dbusInter("org.freedesktop.NetworkManager", device->uni(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
        dbusInter.setProperty("Managed", managed);
    }, Qt::UniqueConnection);
}

void NetworkInitialization::initDeviceConnection(const QSharedPointer<NetworkManager::WiredDevice> &device)
{
    connect(device.data(), &NetworkManager::WiredDevice::interfaceFlagsChanged, this, [ this, device ] {
        qCDebug(DSM) << "device" << device->interfaceName() << "interfaceFlags changed" << device->interfaceFlags();
        addFirstConnection(device);
    }, Qt::UniqueConnection);
    connect(device.data(), &NetworkManager::WiredDevice::managedChanged, this, [ this, device ] {
        qCDebug(DSM) << "device" << device->interfaceName() << "managed changed" << device->managed();
        addFirstConnection(device);
    }, Qt::UniqueConnection);
    connect(device.data(), &NetworkManager::WiredDevice::carrierChanged, this, [ this, device ] {
        qCDebug(DSM) << "device" << device->interfaceName() << "carrier changed" << device->carrier();
        addFirstConnection(device);
    }, Qt::UniqueConnection);
}

void NetworkInitialization::checkAccountStatus()
{
    QDBusInterface dbusInter(LOCKERVICE, LOCKPATH, LOCKINTERFACE, QDBusConnection::systemBus());
    QDBusPendingCall reply = dbusInter.asyncCall("CurrentUser");
    reply.waitForFinished();
    QDBusPendingReply<QString> replyResult = reply.reply();
    m_initilized = installUserTranslator(replyResult.value());
}

void NetworkInitialization::onInitDeviceConnection()
{
    QList<NetworkManager::WirelessDevice::Ptr> wirelessDevices;
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device: devices) {
        if (device->type() == NetworkManager::Device::Type::Wifi) {
            wirelessDevices << device.staticCast<NetworkManager::WirelessDevice>();
        } else if (device->type() == NetworkManager::Device::Type::Ethernet) {
            checkAccountStatus();
            qCDebug(DSM) << "Wired device" << device->interfaceName() << "initilized" << m_initilized << ",add first connection";
            if (m_initilized) {
                NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
                initDeviceConnection(wiredDevice);
                addFirstConnection(wiredDevice);
            }
        }
    }

    bool disableNetwork = SettingConfig::instance()->disableNetwork();
    qCDebug(DSM) << "disbled wireless network" << disableNetwork;
    for (const NetworkManager::WirelessDevice::Ptr &device : wirelessDevices) {
        hideWirelessDevice(device, disableNetwork);
    }
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAoceand, this, [ disableNetwork, this ](const QString &uni) {
        NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(uni);
        // 这里只处理无线网卡
        if (!device)
            return;

        switch (device->type()) {
        case NetworkManager::Device::Type::Wifi:
            hideWirelessDevice(device, disableNetwork);
            break;
        case NetworkManager::Device::Type::Ethernet: {
            checkAccountStatus();
            qCDebug(DSM) << "new Wired device" << device->interfaceName() << "initilized" << m_initilized << ",add first connection";
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
    qCDebug(DSM) << "onUserChanged:" << json << "initilized =" << m_initilized;
    m_initilized = installUserTranslator(json);

    if (!m_initilized)
        return;

    addFirstConnection();
}

void NetworkInitialization::onUserAoceand(const QString &user)
{
    qCDebug(DSM) << "onUserAoceand:" << user << "initilized =" << m_initilized;

    if (m_hasAddFirstConnection) {
        return;
    }

    //取第一个用户的locale
    m_initilized = installUserTranslator(user);

    if (!m_initilized)
        return;

    addFirstConnection();
}
