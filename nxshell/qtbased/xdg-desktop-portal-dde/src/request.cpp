// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "request.h"

#include <QLoggingCategory>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

Q_LOGGING_CATEGORY(XdgDesktopDDERequest, "xdg-dde-request")

Request::Request(const QDBusObjectPath &handle, const QVariant &data, QObject *parent)
    : QObject(parent)
    , m_handle(handle)
    , m_data(data)
{
    auto sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject(m_handle.path(), this, QDBusConnection::ExportScriptableSlots)) {
        qCDebug(XdgDesktopDDERequest) << sessionBus.lastError().message();
        qCDebug(XdgDesktopDDERequest) << "Failed to register request object for" << m_handle.path();
        deleteLater();
    }
}

Request::~Request()
{
    QDBusConnection::sessionBus().unregisterObject(m_handle.path());
}

void Request::Close(const QDBusMessage &message)
{
    QDBusMessage messageReply = message.createReply();
    QDBusConnection::sessionBus().send(messageReply);

    emit closeRequested(m_data);
    deleteLater();
}
