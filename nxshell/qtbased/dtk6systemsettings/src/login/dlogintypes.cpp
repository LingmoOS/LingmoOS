// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dlogintypes.h"

#include "dloginutils.h"

#include <QDataStream>
#include <QDebug>

DLOGIN_BEGIN_NAMESPACE

QDebug operator<<(QDebug debug, const ShutdownType &type)
{
    debug << Utils::shutdownTypeToString(type);
    return debug;
}

QDebug operator<<(QDebug debug, const ScheduledShutdownValue &scheduledShutdown)
{
    debug << Utils::scheduledShutdownValueToString(scheduledShutdown);
    return debug;
}

QDebug operator<<(QDebug debug, const Inhibitor &inhibitor)
{
    debug << Utils::inhibitorToString(inhibitor);
    return debug;
}

QDebug operator<<(QDebug debug, const PowerAction &action)
{
    debug << Utils::actionToString(action);
    return debug;
}

QDebug operator<<(QDebug debug, const ExecuteStatus &status)
{
    debug << Utils::statusToString(status);
    return debug;
}

QDebug operator<<(QDebug debug, const SessionRole &role)
{
    debug << Utils::sessionRoleToString(role);
    return debug;
}

QDebug operator<<(QDebug debug, const InhibitMode &mode)
{
    debug << Utils::modeToString(mode);
    return debug;
}

QDebug operator<<(QDebug debug, const SessionState &state)
{
    debug << Utils::sessionStateToString(state);
    return debug;
}

QDebug operator<<(QDebug debug, const SessionType &type)
{
    debug << Utils::sessionTypeToString(type);
    return debug;
}

QDebug operator<<(QDebug debug, const SessionClass &sessionClass)
{
    debug << Utils::sessionClassToString(sessionClass);
    return debug;
}

QDebug operator<<(QDebug debug, const UserState &userState)
{
    debug << Utils::userStateToString(userState);
    return debug;
}

DLOGIN_END_NAMESPACE