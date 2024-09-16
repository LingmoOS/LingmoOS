// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessioncontainer.h"
#include "sessionipconfilct.h"
#include "constants.h"
#include "browserassist.h"

#include <QDBusConnection>
#include <QDBusInterface>

using namespace network::sessionservice;

static QString networkService = "org.deepin.service.SystemNetwork";
static QString networkPath = "/org/deepin/service/SystemNetwork";
static QString networkInterface = "org.deepin.service.SystemNetwork";

SessionContainer::SessionContainer(QObject *parent)
    : QObject (parent)
    , m_ipConflictHandler(new SessionIPConflict(this))
{
    initMember();
    initConnection();
}

SessionContainer::~SessionContainer()
{
}

SessionIPConflict *SessionContainer::ipConfilctedChecker() const
{
    return m_ipConflictHandler;
}

void SessionContainer::initConnection()
{
    QDBusConnection::systemBus().connect(networkService, networkPath, networkInterface, "IpConflictChanged", this, SLOT(onIPConflictChanged(const QString &, const QString &, bool)));
    QDBusConnection::systemBus().connect(networkService, networkPath, networkInterface, "PortalDetected", this, SLOT(onPortalDetected(const QString &)));
}

void SessionContainer::initMember()
{
    // 检测初始化的状态是否为门户认证，这种情况下需要先打开Url
    QDBusInterface dbusInter("org.deepin.service.SystemNetwork", "/org/deepin/service/SystemNetwork", "org.deepin.service.SystemNetwork", QDBusConnection::systemBus());
    network::service::Connectivity connectivity = static_cast<network::service::Connectivity>(dbusInter.property("Connectivity").toInt());
    if (connectivity == network::service::Connectivity::Portal) {
        // 获取需要认证的网站的信息，并打开网页
        QString url = dbusInter.property("PortalUrl").toString();
        qDebug() << "check portal url:" << url;
        onPortalDetected(url);
    }
}

void SessionContainer::onIPConflictChanged(const QString &devicePath, const QString &ip, bool conflicted)
{
    // TODO: 这里用于处理IP冲突，例如检测到IP冲突后，会给出提示的消息，等后期将任务栏种的相关处理删除后，再到这里来处理，目前此处暂时保留
}

void SessionContainer::onPortalDetected(const QString &url)
{
    BrowserAssist::openUrl(url);
}
