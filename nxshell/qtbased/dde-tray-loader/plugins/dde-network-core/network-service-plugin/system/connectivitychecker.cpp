// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "connectivitychecker.h"

#include "settingconfig.h"
#include "systemipconfilct.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessNetwork>

#include <QTimer>
#include <QProcess>

// 当没有进行配置的时候, 则访问我们官网
static const QStringList CheckUrls{
    "https://www.uniontech.com",
};

using namespace network::systemservice;

ConnectivityChecker::ConnectivityChecker(QObject *parent)
    : QObject(parent)
{
}

ConnectivityChecker::~ConnectivityChecker() { }

// 通过本地检测网络连通性
LocalConnectionvityChecker::LocalConnectionvityChecker(SystemIPConflict *ipHandler, QObject *parent)
    : ConnectivityChecker(parent)
    , m_checkTimer(new QTimer)
    , m_timer(new QTimer)
    , m_connectivity(network::service::Connectivity::Unknownconnectivity)
    , m_checkCount(0)
    , m_ipConfilctHandler(ipHandler)
{
    initConnectivityChecker();
    connect(SettingConfig::instance(), &SettingConfig::connectivityCheckIntervalChanged, this, [this](int interval) {
        if (interval < 10000) {
            interval = 10000;
        }
        m_checkTimer->setInterval(interval);
    });
}

LocalConnectionvityChecker::~LocalConnectionvityChecker()
{
    for (QMetaObject::Connection connection : m_checkerConnection) {
        disconnect(connection);
    }
    m_checkerConnection.clear();

    m_checkTimer->stop();
    m_checkTimer->deleteLater();
    m_checkTimer = nullptr;

    if (m_timer->isActive())
        m_timer->stop();
    m_timer->deleteLater();
    m_timer = nullptr;

    clearProcess();
}

network::service::Connectivity LocalConnectionvityChecker::connectivity() const
{
    return m_connectivity;
}

QString LocalConnectionvityChecker::portalUrl() const
{
    return m_portalUrl;
}

void LocalConnectionvityChecker::initDeviceConnect(const QList<QSharedPointer<NetworkManager::Device>> &devices)
{
    for (const QSharedPointer<NetworkManager::Device> &device : devices) {
        if (device.isNull())
            continue;

        m_checkerConnection << connect(device.data(), &NetworkManager::Device::stateChanged, this, &LocalConnectionvityChecker::startCheck, Qt::UniqueConnection);
        m_checkerConnection << connect(device.data(), &NetworkManager::Device::activeConnectionChanged, this, [device, this] {
            m_checkCount = 0;
            initDefaultConnectivity();
            NetworkManager::ActiveConnection::Ptr activeConnection = device->activeConnection();
            onUpdataActiveState(activeConnection);
            if (!m_timer->isActive()) {
                m_timer->start();
            }
        });
        onUpdataActiveState(device->activeConnection());
    }
}

void LocalConnectionvityChecker::initConnectivityChecker()
{
    connect(SettingConfig::instance(), &SettingConfig::checkUrlsChanged, this, &LocalConnectionvityChecker::onUpdateUrls);
    onUpdateUrls(SettingConfig::instance()->networkCheckerUrls());
    // 定期检查网络的状态
    int interval = SettingConfig::instance()->connectivityCheckInterval() * 1000;
    if (interval < 10000) {
        interval = 10000;
    }
    m_checkTimer->setInterval(interval);
    m_checkerConnection << connect(m_checkTimer, &QTimer::timeout, this, &LocalConnectionvityChecker::startCheck, Qt::UniqueConnection);
    m_checkTimer->start();
    // 这个定时器用于在设备状态发生变化后开启，每间隔2秒做一次网络检测，连续做8次然后再停下，目的是为了在设备状态发生变化的一瞬间网络检测不准确，因此做了这个机制,这样的话保证了在设备状态变化后一定时间内状态始终正确
    m_timer->setInterval(2000);
    m_checkerConnection << connect(m_timer, &QTimer::timeout, this, &LocalConnectionvityChecker::startCheck, Qt::UniqueConnection);
    m_checkerConnection << connect(m_timer, &QTimer::timeout, this, [this] {
        if (m_checkCount >= 8) {
            m_checkCount = 0;
            if (m_timer->isActive())
                m_timer->stop();
        } else {
            m_checkCount++;
        }
    });

    // 当网络设备状态发生变化的时候需要检查网络状态
    initDeviceConnect(NetworkManager::networkInterfaces());
    initDefaultConnectivity();

    m_checkerConnection << connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, [this](const QString &uni) {
        initDeviceConnect(QList<QSharedPointer<NetworkManager::Device>>() << NetworkManager::findNetworkInterface(uni));
    });
    // 第一次进来的时候执行一次网络状态的检测
    QMetaObject::invokeMethod(this, &LocalConnectionvityChecker::startCheck, Qt::QueuedConnection);
}

network::service::Connectivity LocalConnectionvityChecker::getDefaultLimitConnectivity() const
{
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device : devices) {
        if (device->state() != NetworkManager::Device::State::Activated)
            continue;

        if (m_connectivity == network::service::Connectivity::Portal)
            return network::service::Connectivity::Portal;

        return network::service::Connectivity::Limited;
    }

    return network::service::Connectivity::Noconnectivity;
}

void LocalConnectionvityChecker::onUpdataActiveState(const QSharedPointer<NetworkManager::ActiveConnection> &networks)
{
    if (networks.isNull())
        return;

    connect(networks.data(), &NetworkManager::ActiveConnection::stateChanged, this, [this](NetworkManager::ActiveConnection::State state) {
        if (state == NetworkManager::ActiveConnection::State::Activated || state == NetworkManager::ActiveConnection::State::Deactivated) {
            m_checkCount = 0;
            initDefaultConnectivity();
            if (!m_timer->isActive()) {
                m_timer->start();
            }
        }
    });
}

void LocalConnectionvityChecker::onUpdateUrls(const QStringList &urls)
{
    clearProcess();
    m_checkUrls.clear();
    const QStringList &checkUrls = urls.isEmpty() ? CheckUrls : urls;
    for (auto &&url : checkUrls) {
        m_checkUrls.insert(url, nullptr);
    }
}

void LocalConnectionvityChecker::startCheck()
{
    // 有正在检测的测不再检测
    for (auto process : m_checkUrls) {
        if (process) {
            return;
        }
    }
    for (auto it = m_checkUrls.begin(); it != m_checkUrls.end(); ++it) {
        QProcess *process = new QProcess(this);
        connect(process, QOverload<int>::of(&QProcess::finished), this, &LocalConnectionvityChecker::onFinished);
        it.value() = process;
        process->start("curl", { "-LiI", "--connect-timeout", "5", it.key() });
    }
}

void LocalConnectionvityChecker::onFinished(int exitCode)
{

    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;
    for (auto it = m_checkUrls.begin(); it != m_checkUrls.end(); ++it) {
        if (it.value() == process) {
            qDebug() << "check Url:" << it.key() << "exitCode:" << exitCode;
            if (exitCode == 0) {
                QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
                QStringList lines = output.split('\n');
                int httpCode = 0;
                QString portalUrl;
                for (auto &&line : lines) {
                    if (line.startsWith("HTTP") && httpCode == 0) {
                        QStringList httpOut = line.split(' ');
                        if (httpOut.size() >= 2) {
                            httpCode = httpOut.at(1).at(0).digitValue();
                        }
                    } else if (line.startsWith("Location: ")) {
                        portalUrl = line.split(' ').at(1).trimmed();
                    }
                }
                qDebug() << "code:" << httpCode << "portalUrl:" << portalUrl;
                switch (httpCode) {
                case 2: // 2xx（成功状态码）：表示请求已经被成功接收、理解、并处理
                case 3: // 3xx（重定向状态码）：表示请求需要进一步操作，以完成请求
                case 5: // 5xx（服务器错误状态码）：表示服务器处理请求时出现了错误
                    setPortalUrl(portalUrl);
                    setConnectivity(portalUrl.isEmpty() ? network::service::Connectivity::Full : network::service::Connectivity::Portal);
                    clearProcess();
                    return;
                default:
                    break;
                }
            }
            it.value() = nullptr;
            process->deleteLater();
            process = nullptr;
            break;
        }
    }
    if (process) { // process未被置空表示未找到
        process->deleteLater();
        return;
    }
    bool isFinished = true;
    for (auto &&proc : m_checkUrls) {
        if (proc) { // 返回的都已置空，还有没返回的则等待
            isFinished = false;
            break;
        }
    }
    if (isFinished) { // 都处理完成还是没连上网，则为不可上网
        NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
        int disconnectCount = 0;
        for (NetworkManager::Device::Ptr device : devices) {
            if (device->state() == NetworkManager::Device::Disconnected || device->state() == NetworkManager::Device::Failed || device->state() == NetworkManager::Device::Unmanaged
                || device->state() == NetworkManager::Device::Unavailable) {
                disconnectCount++;
            }
        }
        setPortalUrl(QString());
        if (disconnectCount == devices.size()) {
            // 如果所有的网络设备都断开了，就默认让其变为断开的状态
            setConnectivity(network::service::Connectivity::Noconnectivity);
        } else {
            setConnectivity(network::service::Connectivity::Limited);
        }
    }
}

void LocalConnectionvityChecker::clearProcess()
{
    for (auto it = m_checkUrls.begin(); it != m_checkUrls.end(); ++it) {
        if (it.value()) {
            it.value()->terminate();
            it.value()->deleteLater();
            it.value() = nullptr;
        }
    }
}

void LocalConnectionvityChecker::setConnectivity(const network::service::Connectivity &connectivity)
{
    if (m_connectivity == connectivity)
        return;

    m_connectivity = connectivity;
    // 更新每个设备的联通性
    emit connectivityChanged(m_connectivity);
}

void LocalConnectionvityChecker::setPortalUrl(const QString &portalUrl)
{
    if (m_portalUrl == portalUrl)
        return;

    m_portalUrl = portalUrl;
    emit portalDetected(m_portalUrl);
}

void LocalConnectionvityChecker::initDefaultConnectivity()
{
    // 如果上一次检测的是网络连接受限或者需要认证，则无需设置初始值，否则会出现在使用过程种，本来是无法上网的状态，而设置了错误的初始值导致状态错误
    // 这个函数是为了在断开所有的连接后，再开启连接后设置其初始值
    if (m_connectivity != network::service::Connectivity::Noconnectivity && m_connectivity != network::service::Connectivity::Unknownconnectivity)
        return;

    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    if (devices.isEmpty())
        return;

    bool isFull = false;
    int disconnectCount = 0;
    for (NetworkManager::Device::Ptr device : devices) {
        if (device->state() == NetworkManager::Device::Activated) {
            isFull = true;
        } else if (device->state() == NetworkManager::Device::Disconnected || device->state() == NetworkManager::Device::Failed || device->state() == NetworkManager::Device::Unmanaged
                   || device->state() == NetworkManager::Device::Unavailable) {
            disconnectCount++;
        }
    }
    if (isFull) {
        // 只要有一个网络已连接，就默认让其状态变为full状态
        setConnectivity(network::service::Connectivity::Full);
    } else if (disconnectCount == devices.size()) {
        // 如果所有的网络设备都断开了，就默认让其变为断开的状态
        setConnectivity(network::service::Connectivity::Noconnectivity);
    }
}

// 通过使用NM的方式来获取网络连通性
NMConnectionvityChecker::NMConnectionvityChecker(QObject *parent)
    : ConnectivityChecker(parent)
    , m_connectivity(network::service::Connectivity::Full)
{
    initMember();
    initConnection();
}

NMConnectionvityChecker::~NMConnectionvityChecker() { }

static network::service::Connectivity convertConnectivity(const NetworkManager::Connectivity &connectivity)
{
    switch (connectivity) {
    case NetworkManager::Connectivity::Full:
        return network::service::Connectivity::Full;
    case NetworkManager::Connectivity::Portal:
        return network::service::Connectivity::Portal;
    case NetworkManager::Connectivity::Limited:
        return network::service::Connectivity::Limited;
    case NetworkManager::Connectivity::NoConnectivity:
        return network::service::Connectivity::Noconnectivity;
    case NetworkManager::Connectivity::UnknownConnectivity:
        return network::service::Connectivity::Unknownconnectivity;
    }

    return network::service::Connectivity::Unknownconnectivity;
}

network::service::Connectivity NMConnectionvityChecker::connectivity() const
{
    return m_connectivity;
}

void NMConnectionvityChecker::initMember()
{
    m_connectivity = convertConnectivity(NetworkManager::connectivity());
}

void NMConnectionvityChecker::initConnection()
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, [this](NetworkManager::Connectivity connectivity) {
        m_connectivity = convertConnectivity(connectivity);
        emit connectivityChanged(m_connectivity);
    });
}
