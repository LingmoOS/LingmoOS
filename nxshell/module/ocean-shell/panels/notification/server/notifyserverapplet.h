// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"

namespace notification {

class NotificationManager;
class NotifyServerApplet : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
public:
    explicit NotifyServerApplet(QObject *parent = nullptr);
    ~NotifyServerApplet() override;

    bool load() override;
    bool init() override;

Q_SIGNALS:
    void notificationStateChanged(qint64 id, int processedType);

public Q_SLOTS:
    void actionInvoked(qint64 id, uint bubbleId, const QString &actionKey);
    void notificationClosed(qint64 id, uint bubbleId, uint reason);
    QVariant appValue(const QString &appId, int configItem);
    void removeNotification(qint64 id);
    void removeNotifications(const QString &appName);
    void removeNotifications();

private:
    NotificationManager *m_manager = nullptr;
    QThread *m_worker = nullptr;
};

}
