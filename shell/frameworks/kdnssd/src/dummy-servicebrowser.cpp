/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "domainbrowser.h"
#include "servicebrowser.h"
#include <QHash>
#include <QStringList>

namespace KDNSSD
{
class ServiceBrowserPrivate
{
};

ServiceBrowser::ServiceBrowser(const QString &, bool, const QString &, const QString &)
    : d(nullptr)
{
}

bool ServiceBrowser::isAutoResolving() const
{
    return false;
}

ServiceBrowser::State ServiceBrowser::isAvailable()
{
    return Unsupported;
}
ServiceBrowser::~ServiceBrowser()
{
}

void ServiceBrowser::startBrowse()
{
    Q_EMIT finished();
}

QList<RemoteService::Ptr> ServiceBrowser::services() const
{
    return QList<RemoteService::Ptr>();
}

void ServiceBrowser::virtual_hook(int, void *)
{
}

QHostAddress ServiceBrowser::resolveHostName(const QString &hostname)
{
    Q_UNUSED(hostname);
    return QHostAddress();
}

QString ServiceBrowser::getLocalHostName()
{
    return QString();
}

}

#include "moc_servicebrowser.cpp"
