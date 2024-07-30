/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef FD_AC_PLUGGED_JOB_H
#define FD_AC_PLUGGED_JOB_H

#include "backends/abstractacpluggedjob.h"

class QDBusError;
class QDBusMessage;
class QDBusPendingCallWatcher;
namespace Solid
{
class FDAcPluggedJob : public AbstractAcPluggedJob
{
    Q_OBJECT
public:
    explicit FDAcPluggedJob(QObject *parent = nullptr);
    bool isPlugged() const override;

private Q_SLOTS:
    void doStart() override;

    void slotDBusReply(const QDBusMessage &msg);
    void slotDBusError(const QDBusError &error);

private:
    bool m_isPlugged;
};
}

#endif // FD_AC_PLUGGED_JOB_H
