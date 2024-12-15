// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOTHTYPES_H
#define DBLUETOTHTYPES_H

#include "dtkbluetooth_global.h"
#include <QObject>
#include <QVariantMap>
#include <QDBusObjectPath>
#include <QPair>

DBLUETOOTH_BEGIN_NAMESPACE

enum class AgentError : quint8 { Canceled, Rejected };

enum class RequestDest : quint8 { OrgBluezAgent, OrgBluezObexAgent };

enum class ObexSessionType : quint8 { Client, Server };

struct ObexSessionInfo
{
    ObexSessionType sessionInfo;
    quint64 sessionId;
    bool operator==(const ObexSessionInfo &other) const
    {
        return (sessionInfo == other.sessionInfo) and (sessionId == other.sessionId);
    }
};

DBLUETOOTH_END_NAMESPACE

#endif
