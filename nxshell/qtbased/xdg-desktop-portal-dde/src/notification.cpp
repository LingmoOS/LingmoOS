// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notification.h"

#include "notification_interface.h"

#include <QDateTime>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgDesktopDDENotification, "xdg-dde-notification")

static const uint defaultTimeout = 5;

NotificationPortal::NotificationPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_service(new NotificationService("org.freedesktop.Notifications",
                                            "/org/freedesktop/Notifications",
                                            QDBusConnection::sessionBus(), this))
{
    qCDebug(XdgDesktopDDENotification) << "init NotificationPortal";

    connect(m_service, &NotificationService::ActionInvoked, this, &NotificationPortal::actionInvoked);
}

void NotificationPortal::AddNotification(const QString &app_id, const QString &id, const QVariantMap &notification)
{
    if (!m_service->isValid()) {
        qCWarning(XdgDesktopDDENotification) << "notification service has error!";
    }

    qCDebug(XdgDesktopDDENotification) << "notification add start";
    QString title = notification.value("title", QString()).toString();
    QString body = notification.value("body", QString()).toString();
    QString icon = notification.value("icon", QString()).toString();

    QStringList notificationActions;
    if (notification.contains(QStringLiteral("default-action")) && notification.contains(QStringLiteral("default-action-target"))) {
        // default action is conveniently mapped to action number 0 so it uses the same action invocation method as the others
        notificationActions.append("default");
    }

    if (notification.contains(QStringLiteral("buttons"))) {
        QList<QVariantMap> buttons;
        QDBusArgument dbusArgument = notification.value(QStringLiteral("buttons")).value<QDBusArgument>();
        while (!dbusArgument.atEnd()) {
            dbusArgument >> buttons;
        }

        QStringList actions;
        for (const QVariantMap &button : std::as_const(buttons)) {
            actions << button.value(QStringLiteral("label")).toString();
        }

        if (!actions.isEmpty()) {
            notificationActions.append(actions);
        }
    }

    // default 5s
    int expireTimeout = defaultTimeout;
    uint defaultId = 0;
    uint nId = m_service->Notify(app_id, defaultId, icon, QString(), body, notificationActions, QVariantMap(), expireTimeout);

    NotificationInfo info;
    info.appId = app_id;
    info.id = id;
    info.timestamp = QDateTime::currentSecsSinceEpoch();

    m_idInfoMap.insert(nId, info);
    m_idMaps.insert(id, nId);

    clearTimeoutData();
}

void NotificationPortal::RemoveNotification(const QString &app_id, const QString &id)
{
    Q_UNUSED(app_id)
    qCDebug(XdgDesktopDDENotification) << "notification remove start";

    if (!m_service->isValid() || m_idMaps.contains(id)) {
        return;
    }

    m_service->CloseNotification(m_idMaps[id]);
}

void NotificationPortal::actionInvoked(uint nId, const QString &action)
{
    if (!m_idInfoMap.contains(nId)) {
        return;
    }

    const NotificationInfo &info = m_idInfoMap[nId];

    // signals:void ActionInvoked(const QString &app_id, const QString &id, const QString &action, const QList<QVariant> &) const;
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/org/freedesktop/portal/desktop"),
                                                      QStringLiteral("org.freedesktop.impl.portal.Notification"),
                                                      QStringLiteral("ActionInvoked"));
    message << info.appId << info.id << action << QVariantList();
    QDBusConnection::sessionBus().send(message);
}

void NotificationPortal::clearTimeoutData()
{
    qint64 timestamp = QDateTime::currentSecsSinceEpoch();
    QMapIterator<uint, NotificationInfo> it(m_idInfoMap);
    while (it.hasNext()) {
        it.next();

        if (timestamp - it.value().timestamp > defaultTimeout) {
            m_idInfoMap.remove(it.key());
        }
    }
}
