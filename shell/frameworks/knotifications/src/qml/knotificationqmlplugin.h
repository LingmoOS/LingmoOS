/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONQMLPLUGIN_H
#define KNOTIFICATIONQMLPLUGIN_H

#include <QQmlEngine>

#include <KNotification>
#include <KNotificationPermission>
#include <KNotificationReplyAction>

struct NotificationActionForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(NotificationAction)
    QML_FOREIGN(KNotificationAction);
};

struct NotificationReplyActionForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(NotificationReplyAction)
    QML_UNCREATABLE("")
    QML_FOREIGN(KNotificationReplyAction);
};

class NotificationWrapper : public KNotification
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Notification)
    Q_PROPERTY(KNotificationReplyAction *replyAction READ replyActionFactory CONSTANT)
    Q_PROPERTY(QQmlListProperty<KNotificationAction> actions READ actionsProperty NOTIFY actionsChanged)
    Q_PROPERTY(KNotificationAction *defaultAction READ defaultAction WRITE setDefaultActionQml NOTIFY defaultActionChanged)
public:
    explicit NotificationWrapper(QObject *parent = nullptr);

    KNotificationReplyAction *replyActionFactory();

    int actionCount() const;

    KNotificationAction *actionAt(qsizetype index);

    QQmlListProperty<KNotificationAction> actionsProperty() const;

    static qsizetype actionsCount(QQmlListProperty<KNotificationAction> *list);

    static void appendAction(QQmlListProperty<KNotificationAction> *list, KNotificationAction *value);

    static KNotificationAction *actionAt(QQmlListProperty<KNotificationAction> *list, qsizetype index);

    static void clearActions(QQmlListProperty<KNotificationAction> *list);

private:
    QQmlListProperty<KNotificationAction> m_actionsProperty;
};

class NotificationPermissionWrapper : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(NotificationPermission)
    QML_SINGLETON
public:
    Q_INVOKABLE bool checkPermission()
    {
        return KNotificationPermission::checkPermission() == Qt::PermissionStatus::Granted;
    }

    Q_INVOKABLE void requestPermission(const QJSValue &callback)
    {
        KNotificationPermission::requestPermission(this, [&callback](Qt::PermissionStatus status) {
            callback.call({status == Qt::PermissionStatus::Granted});
        });
    }
};

#endif // KNOTIFICATIONQMLPLUGIN_H
