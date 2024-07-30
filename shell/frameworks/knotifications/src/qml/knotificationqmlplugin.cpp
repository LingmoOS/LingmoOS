/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knotificationqmlplugin.h"

NotificationWrapper::NotificationWrapper(QObject *parent)
    : KNotification(QString(), KNotification::CloseOnTimeout, parent)
{
    setAutoDelete(false);

    m_actionsProperty = QQmlListProperty<KNotificationAction>(this,
                                                              nullptr,
                                                              &NotificationWrapper::appendAction,
                                                              &NotificationWrapper::actionsCount,
                                                              &NotificationWrapper::actionAt,
                                                              &NotificationWrapper::clearActions);
}

KNotificationReplyAction *NotificationWrapper::replyActionFactory()
{
    if (!replyAction()) {
        setReplyAction(std::make_unique<KNotificationReplyAction>(QString()));
    }
    return replyAction();
}

int NotificationWrapper::actionCount() const
{
    return actions().count();
}

KNotificationAction *NotificationWrapper::actionAt(qsizetype index)
{
    return actions().at(index);
}

QQmlListProperty<KNotificationAction> NotificationWrapper::actionsProperty() const
{
    return m_actionsProperty;
}

qsizetype NotificationWrapper::actionsCount(QQmlListProperty<KNotificationAction> *list)
{
    return static_cast<NotificationWrapper *>(list->object)->actionCount();
}

void NotificationWrapper::appendAction(QQmlListProperty<KNotificationAction> *list, KNotificationAction *value)
{
    auto notification = static_cast<NotificationWrapper *>(list->object);
    auto actions = notification->actions();
    actions << value;
    notification->setActionsQml(actions);
}

KNotificationAction *NotificationWrapper::actionAt(QQmlListProperty<KNotificationAction> *list, qsizetype index)
{
    return static_cast<NotificationWrapper *>(list->object)->actionAt(index);
}

void NotificationWrapper::clearActions(QQmlListProperty<KNotificationAction> *list)
{
    auto notification = static_cast<KNotification *>(list->object);
    notification->clearActions();
}

#include "moc_knotificationqmlplugin.cpp"
