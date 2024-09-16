// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mouseservice.h"
#include "mouseadaptor.h"
#include "trackpointadaptor.h"
#include <QtDBus>
DDEVICE_BEGIN_NAMESPACE

MouseService::MouseService(const QString &service, const QString &path, QObject *parent)
    : QObject(parent)
    , m_service(service)
    , m_path(path)
    , m_mouseAdaptor(new MouseAdaptor(this))
    , m_trackPointAdaptor(new TrackPointAdaptor(this))
{
    registerService();
}

MouseService::~MouseService()
{
    unregisterService();
}

bool MouseService::registerService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(m_service)) {
        QString errorMsg = connection.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.").arg(m_service).arg(errorMsg);
        return false;
    }
    if (!connection.registerObject(m_path, this, QDBusConnection::ExportAdaptors)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(m_path);
        return false;
    }
    return true;
}

void MouseService::unregisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject(m_path);
    connection.unregisterService(m_service);
}

DDEVICE_END_NAMESPACE
