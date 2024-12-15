// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inhibit.h"

#include <QLoggingCategory>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>

#include "request.h"

Q_LOGGING_CATEGORY(XdgDesktopOCEANInhibit, "xdg-ocean-inhibit")

const QString sessionManagerService = QStringLiteral("org.lingmo.ocean.SessionManager1");
const QString sessionManagerPath = QStringLiteral("/com/lingmo/SessionManager");
const QString sessionManagerInterface = QStringLiteral("com.lingmo.SessionManager");

enum { INHIBIT_LOGOUT = 1, INHIBIT_SWITCH = 2, INHIBIT_SUSPEND = 4, INHIBIT_IDLE = 8 };

InhibitPortal::InhibitPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(XdgDesktopOCEANInhibit) << "Inhibit init";
}

void InhibitPortal::Inhibit(
    const QDBusObjectPath &handle, const QString &app_id, const QString &window, uint flags, const QVariantMap &options)
{
    qCDebug(XdgDesktopOCEANInhibit) << "Handle: " << handle.path();
    qCDebug(XdgDesktopOCEANInhibit) << app_id << "request Inhibit";

    QString reason = options.value(QStringLiteral("reason")).toString();
    qCDebug(XdgDesktopOCEANInhibit) << "reason: " << reason;

    QDBusMessage message = QDBusMessage::createMethodCall(
        sessionManagerService, sessionManagerPath, sessionManagerInterface, QStringLiteral("Inhibit"));
    message << app_id << window.toUInt() << reason << flags;

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(watcher, &QDBusPendingCallWatcher::finished, [handle, this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<uint> reply = *watcher;
        if (reply.isError()) {
            qCDebug(XdgDesktopOCEANInhibit) << "Inhibition error: " << reply.error().message();
            return;
        }

        auto *request = new Request(handle, QVariant(reply.value()), this);
        connect(request, &Request::closeRequested, this, &InhibitPortal::onCloseRequested);
    });
}

void InhibitPortal::onCloseRequested(const QVariant &data)
{
    quint32 cookie = data.toUInt();

    QDBusMessage message = QDBusMessage::createMethodCall(
        sessionManagerService, sessionManagerPath, sessionManagerInterface, QStringLiteral("Uninhibit"));
    message << cookie;
    QDBusConnection::sessionBus().asyncCall(message);
}
