/*
    SPDX-FileCopyrightText: 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <Lingmo5Support/Service>

class NotificationsEngine;

class NotificationService : public Lingmo5Support::Service
{
    Q_OBJECT

public:
    NotificationService(NotificationsEngine *parent, const QString &source);

protected:
    Lingmo5Support::ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters) override;

private:
    NotificationsEngine *m_notificationEngine;
};
