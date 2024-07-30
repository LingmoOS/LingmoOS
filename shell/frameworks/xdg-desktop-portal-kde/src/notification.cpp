/*
 * SPDX-FileCopyrightText: 2016 Red Hat Inc
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 */

#include "notification.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>

#include "fdo_application_interface.h"
#include "notification_debug.h"
#include "portalicon.h"

using Action = QPair<QString, QVariant>;
Q_DECLARE_METATYPE(QList<Action>);

NotificationPortal::NotificationPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    PortalIcon::registerDBusType();
}

static QString appPathFromId(const QString &app_id)
{
    QString ret = QLatin1Char('/') + app_id;
    ret.replace('.', '/');
    ret.replace('-', '_');
    return ret;
}

void NotificationPortal::AddNotification(const QString &app_id, const QString &id, const QVariantMap &notification)
{
    qCDebug(XdgDesktopPortalKdeNotification) << "AddNotification called with parameters:";
    qCDebug(XdgDesktopPortalKdeNotification) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeNotification) << "    id: " << id;
    qCDebug(XdgDesktopPortalKdeNotification) << "    notification: " << notification;

    // We have to use "notification" as an ID because any other ID will not be configured
    KNotification *notify = new KNotification(QStringLiteral("notification"), KNotification::CloseOnTimeout | KNotification::DefaultEvent, this);
    if (notification.contains(QStringLiteral("title"))) {
        notify->setTitle(notification.value(QStringLiteral("title")).toString());
    }
    if (notification.contains(QStringLiteral("body"))) {
        notify->setText(notification.value(QStringLiteral("body")).toString());
    }
    if (notification.contains(QStringLiteral("icon"))) {
        QVariant iconVariant = notification.value(QStringLiteral("icon"));
        if (iconVariant.type() == QVariant::String) {
            notify->setIconName(iconVariant.toString());
        } else {
            QDBusArgument argument = iconVariant.value<QDBusArgument>();
            PortalIcon icon = qdbus_cast<PortalIcon>(argument);
            QVariant iconData = icon.data.variant();
            if (icon.str == QStringLiteral("themed") && iconData.type() == QVariant::StringList) {
                notify->setIconName(iconData.toStringList().first());
            } else if (icon.str == QStringLiteral("bytes") && iconData.type() == QVariant::ByteArray) {
                QPixmap pixmap;
                if (pixmap.loadFromData(iconData.toByteArray(), "PNG")) {
                    notify->setPixmap(pixmap);
                }
            }
        }
    }

    const QString priority = notification.value(QStringLiteral("priority")).toString();
    if (priority == QLatin1String("low")) {
        notify->setUrgency(KNotification::LowUrgency);
    } else if (priority == QLatin1String("normal")) {
        notify->setUrgency(KNotification::NormalUrgency);
    } else if (priority == QLatin1String("high")) {
        notify->setUrgency(KNotification::HighUrgency);
    } else if (priority == QLatin1String("urgent")) {
        notify->setUrgency(KNotification::CriticalUrgency);
    }

    auto actionInvoked = [notify, app_id, id](const QString &actionId, const QVariant &actionTarget) {
        QVariantMap platformData;
        if (!notify->xdgActivationToken().isEmpty()) {
            platformData = {
                {QStringLiteral("activation-token"), notify->xdgActivationToken()},

                // apparently gtk uses "desktop-startup-id"
                {QStringLiteral("desktop-startup-id"), notify->xdgActivationToken()},
            };
        }

        qCDebug(XdgDesktopPortalKdeNotification) << "Notification activated:";
        qCDebug(XdgDesktopPortalKdeNotification) << "    app_id: " << app_id;
        qCDebug(XdgDesktopPortalKdeNotification) << "    id: " << id;
        qCDebug(XdgDesktopPortalKdeNotification) << "    action: " << actionId << actionTarget;

        QVariantList params;
        if (actionTarget.isValid()) {
            params += actionTarget;
        }

        OrgFreedesktopApplicationInterface iface(app_id, appPathFromId(app_id), QDBusConnection::sessionBus());
        if (actionId.startsWith("app.") && iface.isValid()) {
            iface.ActivateAction(actionId.mid(4), params, platformData);
        } else {
            if (iface.isValid()) {
                iface.Activate(platformData);
            }

            QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/org/freedesktop/portal/desktop"),
                                                              QStringLiteral("org.freedesktop.impl.portal.Notification"),
                                                              QStringLiteral("ActionInvoked"));
            message << app_id << id << actionId << params;
            QDBusConnection::sessionBus().send(message);
        }
    };

    if (notification.contains(QStringLiteral("default-action")) && notification.contains(QStringLiteral("default-action-target"))) {
        KNotificationAction *action = notify->addDefaultAction(notification.value(QStringLiteral("default-action")).toString());

        connect(action, &KNotificationAction::activated, this, [actionInvoked, notification] {
            actionInvoked(notification.value(QStringLiteral("default-action")).toString(), notification.value(QStringLiteral("default-action-target")));
        });
    }

    if (notification.contains(QStringLiteral("buttons"))) {
        const QDBusArgument dbusArgument = notification.value(QStringLiteral("buttons")).value<QDBusArgument>();
        const auto buttons = qdbus_cast<QList<QVariantMap>>(dbusArgument);

        for (const QVariantMap &button : std::as_const(buttons)) {
            auto action = notify->addAction(button.value(QStringLiteral("label")).toString());
            connect(action, &KNotificationAction::activated, this, [button, actionInvoked] {
                actionInvoked(button.value(QStringLiteral("action")).toString(), button.value(QStringLiteral("target")));
            });
        }
    }
    notify->setHint(QStringLiteral("desktop-entry"), app_id);
    notify->setHint(QStringLiteral("x-kde-xdgTokenAppId"), app_id);

    connect(notify, &KNotification::closed, this, &NotificationPortal::notificationClosed);

    m_notifications.insert(QStringLiteral("%1:%2").arg(app_id, id), notify);
    notify->sendEvent();
}

void NotificationPortal::RemoveNotification(const QString &app_id, const QString &id)
{
    qCDebug(XdgDesktopPortalKdeNotification) << "RemoveNotification called with parameters:";
    qCDebug(XdgDesktopPortalKdeNotification) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeNotification) << "    id: " << id;

    KNotification *notify = m_notifications.take(QStringLiteral("%1:%2").arg(app_id, id));
    if (notify) {
        disconnect(notify, &KNotification::closed, this, &NotificationPortal::notificationClosed);
        notify->close();
    }
}

void NotificationPortal::notificationClosed()
{
    KNotification *notify = qobject_cast<KNotification *>(sender());
    Q_ASSERT(notify);
    const QString appId = notify->property("app_id").toString();
    const QString id = notify->property("id").toString();

    auto n = m_notifications.take(QStringLiteral("%1:%2").arg(appId, id));
    if (n) {
        Q_ASSERT(n == notify);
        n->close();
    }
}
