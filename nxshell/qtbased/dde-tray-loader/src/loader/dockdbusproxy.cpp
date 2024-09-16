//SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
//SPDX-License-Identifier: GPL-3.0-or-later
#include "dockdbusproxy.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusPendingCall>
#include <QDBusPendingReply>

const static QString DockService = "org.deepin.dde.Dock1";
const static QString DockPath = "/org/deepin/dde/Dock1";
const static QString DockInterface = "org.deepin.dde.Dock1";
static void registerDockItemType()
{
    qRegisterMetaType<DockItemInfo>("DockItemInfo");
    qDBusRegisterMetaType<DockItemInfo>();
    qRegisterMetaType<DockItemInfos>("DockItemInfos");
    qDBusRegisterMetaType<DockItemInfos>();
}
Q_CONSTRUCTOR_FUNCTION(registerDockItemType)

QDBusArgument &operator<<(QDBusArgument &arg, const DockItemInfo &info)
{
    arg.beginStructure();
    arg << info.name << info.displayName << info.itemKey << info.settingKey << info.dcc_icon << info.visible;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, DockItemInfo &info)
{
    arg.beginStructure();
    arg >> info.name >> info.displayName >> info.itemKey >> info.settingKey >> info.dcc_icon >> info.visible;
    arg.endStructure();
    return arg;
}

DockDBusProxy::DockDBusProxy(QObject *parent)
    : QObject(parent)
    , m_dockInter(new QDBusInterface(DockService, DockPath, DockInterface, QDBusConnection::sessionBus(), this))
{
    QDBusConnection::sessionBus().connect(DockService, DockPath, DockInterface, "pluginVisibleChanged", this, SLOT(pluginVisibleChanged(const QString &, bool)));
}

DockItemInfos DockDBusProxy::plugins()
{
    QDBusPendingReply<DockItemInfos> reply = m_dockInter->asyncCall(QStringLiteral("plugins"));
    reply.waitForFinished();
    return reply.value();
}

void DockDBusProxy::setItemOnDock(const QString &settingKey, const QString &itemKey, bool visible)
{
    QList<QVariant> argumengList;
    argumengList << settingKey << itemKey << QVariant::fromValue(visible);
    m_dockInter->asyncCallWithArgumentList("setItemOnDock", argumengList);
}
