/*
    SPDX-FileCopyrightText: 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "notificationaction.h"

#include "server.h"

#include <klocalizedstring.h>

#include "debug.h"

using namespace NotificationManager;

void NotificationAction::start()
{
    qCDebug(NOTIFICATIONS) << "Trying to perform the action " << operationName() << " on " << destination();
    qCDebug(NOTIFICATIONS) << "actionId: " << parameters()["actionId"].toString();
    qCDebug(NOTIFICATIONS) << "params: " << parameters();

    if (!m_engine) {
        setErrorText(i18n("The notification dataEngine is not set."));
        setError(-1);
        emitResult();
        return;
    }

    const QStringList dest = destination().split(' ');

    uint id = 0;
    if (dest.count() > 1 && !dest[1].toInt()) {
        setErrorText(i18n("Invalid destination: %1", destination()));
        setError(-2);
        emitResult();
        return;
    } else if (dest.count() > 1) {
        id = dest[1].toUInt();
    }

    if (operationName() == QLatin1String("invokeAction")) {
        qCDebug(NOTIFICATIONS) << "invoking action on " << id;
        Server::self().invokeAction(id, parameters()[QStringLiteral("actionId")].toString(), {}, Notifications::None, nullptr);
    } else if (operationName() == QLatin1String("userClosed")) {
        // userClosedNotification deletes the job, so we have to invoke it queued, in this case emitResult() can be called
        m_engine->metaObject()->invokeMethod(m_engine, "removeNotification", Qt::QueuedConnection, Q_ARG(uint, id), Q_ARG(uint, 2));
    } else if (operationName() == QLatin1String("expireNotification")) {
        // expireNotification deletes the job, so we have to invoke it queued, in this case emitResult() can be called
        m_engine->metaObject()->invokeMethod(m_engine, "removeNotification", Qt::QueuedConnection, Q_ARG(uint, id), Q_ARG(uint, 1));
    } else if (operationName() == QLatin1String("createNotification")) {
        int expireTimeout = parameters().value(QStringLiteral("expireTimeout")).toInt();
        bool isPersistent = parameters().value(QStringLiteral("isPersistent")).toBool();

        QVariantMap hints;
        if (parameters().value(QStringLiteral("skipGrouping")).toBool()) {
            hints.insert(QStringLiteral("x-kde-skipGrouping"), true);
        }

        int rv = m_engine->createNotification(parameters().value(QStringLiteral("appName")).toString(),
                                              parameters().value(QStringLiteral("appIcon")).toString(),
                                              parameters().value(QStringLiteral("summary")).toString(),
                                              parameters().value(QStringLiteral("body")).toString(),
                                              isPersistent ? 0 : expireTimeout,
                                              parameters().value(QStringLiteral("actions")).toStringList(),
                                              hints);
        setResult(rv);
        return;
    } else if (operationName() == QLatin1String("configureNotification")) {
        m_engine->configureNotification(parameters()[QStringLiteral("appRealName")].toString(), parameters()[QStringLiteral("eventId")].toString());
    } else if (operationName() == QLatin1String("inhibit")) {
        const QString hint = parameters()[QStringLiteral("hint")].toString();
        const QString value = parameters()[QStringLiteral("value")].toString();
        auto t = m_engine->createInhibition(hint, value);
        setResult(QVariant::fromValue(t));
        return;
    }

    emitResult();
}
