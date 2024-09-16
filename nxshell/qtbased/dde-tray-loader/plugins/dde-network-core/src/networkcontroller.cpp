// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkcontroller.h"

#include "configsetting.h"
#include "dslcontroller.h"
#include "hotspotcontroller.h"
#include "netutils.h"
#include "networkdetails.h"
#include "networkdevicebase.h"
#include "networkinterprocesser.h"
#include "networkmanagerprocesser.h"
#include "proxycontroller.h"
#include "vpncontroller.h"
#include "wireddevice.h"
#include "wirelessdevice.h"
#include "connectivityhandler.h"

#include <DLog>

const static QString networkService = "com.deepin.daemon.Network";
const static QString networkPath = "/com/deepin/daemon/Network";
static QString localeName;

using namespace dde::network;

NetworkController *NetworkController::m_networkController = nullptr;
// 默认是异步方式
bool NetworkController::m_sync = false;
// 只有任务栏需要检测IP冲突，因此任务栏需要调用相关的接口来检测，其他的应用是不需要检测冲突的
bool NetworkController::m_checkIpConflicted = false;
QTranslator *NetworkController::m_translator = nullptr;

NetworkController::NetworkController()
    : QObject(Q_NULLPTR)
    , m_processer(Q_NULLPTR)
    , m_proxyController(Q_NULLPTR)
    , m_vpnController(Q_NULLPTR)
    , m_dslController(Q_NULLPTR)
    , m_hotspotController(Q_NULLPTR)
    , m_connectivityHandler(new ConnectivityHandler(this))
{
    Dtk::Core::loggerInstance()->logToGlobalInstance(DNC().categoryName(), true);
    retranslate(QLocale().name());

    if (ConfigSetting::instance()->serviceFromNetworkManager())
        m_processer = new NetworkManagerProcesser(m_sync, this);
    else
        m_processer = new NetworkInterProcesser(m_sync, this);

    connect(m_connectivityHandler, &ConnectivityHandler::connectivityChanged, this, &NetworkController::connectivityChanged);
    connect(m_processer, &NetworkProcesser::deviceAdded, this, &NetworkController::deviceAdded);
    connect(m_processer, &NetworkProcesser::deviceRemoved, this, &NetworkController::deviceRemoved);
    connect(m_processer, &NetworkProcesser::connectionChanged, this, &NetworkController::connectionChanged);
    connect(m_processer, &NetworkProcesser::activeConnectionChange, this, &NetworkController::activeConnectionChange);
    if (m_checkIpConflicted) {
        // 如果当前需要处理IP冲突，则直接获取信号连接方式即可
        QDBusConnection::systemBus().connect("org.deepin.service.SystemNetwork", "/org/deepin/service/SystemNetwork",
                                    "org.deepin.service.SystemNetwork", "IpConflictChanged", m_processer, SLOT(onIpConflictChanged(const QString &, const QString &, bool)));
    }
}

NetworkController::~NetworkController() = default;

NetworkController *NetworkController::instance()
{
    static QMutex m;
    QMutexLocker locker(&m);
    if (!m_networkController) {
        m_networkController = new NetworkController;
    };
    return m_networkController;
}

void NetworkController::free()
{
    if (m_networkController) {
        m_networkController->deleteLater();
        m_networkController = nullptr;
    }
}

void NetworkController::setActiveSync(const bool sync)
{
    m_sync = sync;
}

void NetworkController::setIPConflictCheck(const bool &checkIp)
{
    m_checkIpConflicted = checkIp;
}

void NetworkController::alawaysLoadFromNM()
{
    ConfigSetting::instance()->alawaysLoadFromNM();
}

void NetworkController::installTranslator(const QString &locale)
{
    if (localeName == locale)
        return;

    localeName = locale;

    if (m_translator) {
        QCoreApplication::removeTranslator(m_translator);
    } else {
        m_translator = new QTranslator;
    }
    m_translator->load(QString("/usr/share/dde-tray-network-core/translations/dde-network-core_%1").arg(localeName));
    QCoreApplication::installTranslator(m_translator);
}

void NetworkController::updateSync(const bool sync)
{
    auto *processer = qobject_cast<NetworkInterProcesser *>(m_processer);
    if (processer)
        processer->updateSync(sync);
}

ProxyController *NetworkController::proxyController()
{
    return m_processer->proxyController();
}

VPNController *NetworkController::vpnController()
{
    return m_processer->vpnController();
}

DSLController *NetworkController::dslController()
{
    return m_processer->dslController();
}

HotspotController *NetworkController::hotspotController()
{
    return m_processer->hotspotController();
}

QList<NetworkDetails *> NetworkController::networkDetails()
{
    return m_processer->networkDetails();
}

QList<NetworkDeviceBase *> NetworkController::devices() const
{
    return m_processer->devices();
}

Connectivity NetworkController::connectivity()
{
    return m_connectivityHandler->connectivity();
}

void NetworkController::retranslate(const QString &locale)
{
    NetworkController::installTranslator(locale);
    if (m_processer)
        m_processer->retranslate();
}
