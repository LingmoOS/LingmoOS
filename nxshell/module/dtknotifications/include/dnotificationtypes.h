// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QSharedPointer>
#include <QDebugStateSaver>
#include <QVariant>
#include "dtknotification_global.h"

DNOTIFICATIONS_BEGIN_NAMESPACE
struct DNotificationData
{
    QString appName;
    QString id;
    QString appIcon;
    QString summary;
    QString body;
    QStringList actions;
    QVariantMap hints;
    QString ctime;
    uint replacesId;
    QString timeout;
    friend bool operator==(const DNotificationData &lhs, const DNotificationData &rhs)
    {
        return lhs.appName == rhs.appName
                && lhs.id == rhs.id
                && lhs.appIcon == rhs.appIcon
                && lhs.summary == rhs.summary
                && lhs.body == rhs.body
                && lhs.actions == rhs.actions
                && lhs.hints == rhs.hints
                && lhs.ctime == rhs.ctime
                && lhs.replacesId == rhs.replacesId
                && lhs.timeout == rhs.timeout;
    }

    friend bool operator==(const QVariantMap& lhs, const QVariantMap& rhs)
    {
        if (lhs.size() != rhs.size()) {
            return false;
        }

        for (auto it = lhs.constBegin(); it != lhs.constEnd(); ++it) {
            auto key = it.key();
            if (!rhs.contains(key) || lhs[key] != rhs[key]) {
                return false;
            }
        }

        return true;
    }

};
typedef QSharedPointer<DNotificationData> DNotificationDataPtr;

enum class ClosedReason {
    Expired = 1,
    Dismissed,
    Closed,
    Unknown
};

enum AppNotificationConfigItem {
    AppName,
    AppIcon,
    EnableNotification,
    EnablePreview,
    EnableSound,
    ShowInNotificationCenter,
    LockScreenShowNofitication
};

enum DNDModeNotificaitonConfigItem {
    DNDMode,
    LockScreenOpenDNDMode,
    OpenByTimeInterval,
    StartTime,
    EndTime,
    ShowIcon
};

struct ServerInformation {
    QString name;
    QString vendor;
    QString version;
    QString spec_version;
    friend bool operator==(const ServerInformation &lhs, const ServerInformation &rhs)
    {
        return lhs.name == rhs.name
                && lhs.vendor == rhs.vendor
                && lhs.version == rhs.version
                && lhs.spec_version == rhs.spec_version;
    }
};

QDebug operator<<(QDebug debug, const DNotificationData &nofitication);
QDebug operator<<(QDebug debug, const ServerInformation &serverInformation);

DNOTIFICATIONS_END_NAMESPACE

Q_DECLARE_METATYPE(DNOTIFICATIONS_NAMESPACE::ClosedReason)
Q_DECLARE_METATYPE(DNOTIFICATIONS_NAMESPACE::ServerInformation)
Q_DECLARE_METATYPE(DNOTIFICATIONS_NAMESPACE::AppNotificationConfigItem)
Q_DECLARE_METATYPE(DNOTIFICATIONS_NAMESPACE::DNDModeNotificaitonConfigItem)
