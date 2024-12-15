// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class NotificationService;

class NotificationPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Notification")

public:
    explicit NotificationPortal(QObject *parent);
    ~NotificationPortal() = default;

public slots:
    void AddNotification(const QString &app_id, const QString &id, const QVariantMap &notification);
    void RemoveNotification(const QString &app_id, const QString &id);

private slots:
    void actionInvoked(uint nId, const QString &action);
    void clearTimeoutData();

private:
    struct NotificationInfo {
        QString appId;
        QString id; // from "AddNotification" param2
        qint64 timestamp;
    };

private:
    NotificationService *m_service;

    QMap<uint, NotificationInfo> m_idInfoMap;
    QMap<QString, uint> m_idMaps;
};
