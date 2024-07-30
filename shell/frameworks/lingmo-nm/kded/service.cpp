/*
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf54321@gmail.com>
    SPDX-FileCopyrightText: 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2012 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "service.h"

#include <KPluginFactory>

#include "connectivitymonitor.h"
#include "monitor.h"
#include "notification.h"
#include "secretagent.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusServiceWatcher>

K_PLUGIN_CLASS_WITH_JSON(NetworkManagementService, "networkmanagement.json")

class NetworkManagementServicePrivate
{
public:
    SecretAgent *agent = nullptr;
    Notification *notification = nullptr;
    Monitor *monitor = nullptr;
    ConnectivityMonitor *connectivityMonitor = nullptr;
};

NetworkManagementService::NetworkManagementService(QObject *parent, const QVariantList &)
    : KDEDModule(parent)
    , d_ptr(new NetworkManagementServicePrivate)
{
    Q_D(NetworkManagementService);

    connect(this, &KDEDModule::moduleRegistered, this, &NetworkManagementService::init);

    d->agent = new SecretAgent(this);
    connect(d->agent, &SecretAgent::secretsError, this, &NetworkManagementService::secretsError);
}

NetworkManagementService::~NetworkManagementService()
{
    delete d_ptr;
}

void NetworkManagementService::init()
{
    Q_D(NetworkManagementService);

    if (!d->notification) {
        d->notification = new Notification(this);
    }

    if (!d->monitor) {
        d->monitor = new Monitor(this);
    }

    if (!d->connectivityMonitor) {
        d->connectivityMonitor = new ConnectivityMonitor(this);
    }
}

#include "service.moc"

#include "moc_service.cpp"
