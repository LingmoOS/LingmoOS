/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NOTIFYBYANDROID_H
#define NOTIFYBYANDROID_H

#include "knotificationplugin.h"

#include <QJniObject>
#include <QPointer>

/** Android notification backend. */
class NotifyByAndroid : public KNotificationPlugin
{
    Q_OBJECT
public:
    explicit NotifyByAndroid(QObject *parent = nullptr);
    ~NotifyByAndroid() override;

    // interface of KNotificationPlugin
    QString optionName() override;
    void notify(KNotification *notification, const KNotifyConfig &notifyConfig) override;
    void update(KNotification *notification, const KNotifyConfig &notifyConfig) override;
    void close(KNotification *notification) override;

    // interface from Java
    void notificationFinished(int id);
    void notificationActionInvoked(int id, const QString &action);
    void notificationInlineReply(int id, const QString &text);

private:
    void notifyDeferred(KNotification *notification);
    QJniObject createAndroidNotification(KNotification *notification, const KNotifyConfig &notifyConfig) const;

    QJniObject m_backend;
    QHash<int, QPointer<KNotification>> m_notifications;
};

#endif // NOTIFYBYANDROID_H
