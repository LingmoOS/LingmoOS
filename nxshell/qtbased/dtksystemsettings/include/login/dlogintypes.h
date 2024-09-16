// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtklogin_global.h"

#include <qdatetime.h>
#include <qobject.h>
#include <qvariant.h>

DLOGIN_BEGIN_NAMESPACE
// Do not use any unknown enum
enum class ShutdownType { PowerOff, DryPowerOff, Reboot, DryReboot, Halt, DryHalt, Unknown };

struct ScheduledShutdownValue
{
    ShutdownType type;
    QDateTime time;
};

enum InhibitBehavior {
    Shutdown = 1 << 0,
    Sleep = 1 << 1,
    Idle = 1 << 2,
    HandlePowerKey = 1 << 3,
    HandleSuspendKey = 1 << 4,
    HandleHibernateKey = 1 << 5,
    HandleLidSwitch = 1 << 6
};

enum class PowerAction {
    PowerOff,
    Reboot,
    Halt,
    KExec,
    Suspend,
    Hibernate,
    HybridSleep,
    SuspendThenHibernate,
    Lock,
    FactoryReset,
    Ignore,
    Unknown
};

enum class ExecuteStatus { Yes, No, Challenge, Na, Unknown };
enum class SessionRole { Leader, All, Unknown };
enum class InhibitMode { Block, Delay, Unknown };
enum class SessionState { Online, Active, Closing, Unknown };
enum class SessionType { TTY, X11, Mir, Wayland, Unspecified };
enum class SessionClass { User, Greeter, LockScreen, Unknown };
enum class UserState { Offline, Lingering, Online, Active, Closing, Unknown };

struct Inhibitor
{
    int what;
    QString who;
    QString why;
    InhibitMode mode;
    quint32 UID;
    quint32 PID;
    friend QDebug operator<<(QDebug debug, const Inhibitor &inhibitor);
    friend bool operator==(const Inhibitor &lhs, const Inhibitor &rhs){
        return lhs.what == rhs.what && lhs.who == rhs.who && lhs.why == rhs.why && lhs.mode == rhs.mode && lhs.UID == rhs.UID && lhs.PID == rhs.PID;
    }
};

enum DLoginErrorCode {
    NoError = 0,
    Other = 1,
    Failed,
    NoMemory,
    ServiceUnknown,
    NoReply,
    BadAddress,
    NotSupported,
    LimitsExceeded,
    AccessDenied,
    NoServer,
    Timeout,
    NoNetwork,
    AddressInUse,
    Disconnected,
    InvalidArgs,
    UnknownMethod,
    TimedOut,
    InvalidSignature,
    UnknownInterface,
    UnknownObject,
    UnknownProperty,
    PropertyReadOnly,
    InternalError,
    InvalidService,
    InvalidObjectPath,
    InvalidInterface,
    InvalidMember
};

QDebug operator<<(QDebug debug, const ShutdownType &type);
QDebug operator<<(QDebug debug, const ScheduledShutdownValue &scheduledShutdown);
QDebug operator<<(QDebug debug, const PowerAction &action);
QDebug operator<<(QDebug debug, const ExecuteStatus &status);
QDebug operator<<(QDebug debug, const SessionRole &role);
QDebug operator<<(QDebug debug, const InhibitMode &mode);
QDebug operator<<(QDebug debug, const SessionState &state);
QDebug operator<<(QDebug debug, const SessionType &type);
QDebug operator<<(QDebug debug, const SessionClass &sessionClass);
QDebug operator<<(QDebug debug, const UserState &userState);

DLOGIN_END_NAMESPACE

Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::ShutdownType)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::ScheduledShutdownValue)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::PowerAction)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::ExecuteStatus)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::SessionRole)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::InhibitMode)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::SessionState)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::SessionType)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::SessionClass)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::UserState)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::Inhibitor)
