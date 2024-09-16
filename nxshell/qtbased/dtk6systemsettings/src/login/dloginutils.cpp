// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dloginutils.h"

#include <qdebug.h>
#include <qmap.h>

std::ostream &operator<<(std::ostream &os, const QString &str)
{
    os << str.toStdString();
    return os;
}

std::ostream &operator<<(std::ostream &os, const QStringList &strList)
{
    const QString str = strList.join(",");
    os << "{ " << str << " }";
    return os;
}

DLOGIN_BEGIN_NAMESPACE

namespace Utils {
QString decodeBehavior(int behavior)
{
    static const QMap<InhibitBehavior, QString> behaviorMap = {
        { InhibitBehavior::Shutdown, "shutdown" },
        { InhibitBehavior::Sleep, "sleep" },
        { InhibitBehavior::Idle, "idle" },
        { InhibitBehavior::HandlePowerKey, "handle-power-key" },
        { InhibitBehavior::HandleSuspendKey, "handle-suspend-key" },
        { InhibitBehavior::HandleHibernateKey, "handle-hibernate-key" },
        { InhibitBehavior::HandleLidSwitch, "handle-lid-switch" }
    };
    QString decoded;
    for (auto it = behaviorMap.begin(); it != behaviorMap.end(); it++) {
        if (behavior & it.key()) {
            decoded += it.value();
            decoded += ":";
        }
    }
    if (!decoded.isEmpty()) {
        decoded.chop(1);
    }
    return decoded;
}

int encodeBehavior(const QString &behavior)
{
    static const QMap<QString, InhibitBehavior> behaviorMap = {
        { "shutdown", InhibitBehavior::Shutdown },
        { "sleep", InhibitBehavior::Sleep },
        { "idle", InhibitBehavior::Idle },
        { "handle-power-key", InhibitBehavior::HandlePowerKey },
        { "handle-suspend-key", InhibitBehavior::HandleSuspendKey },
        { "handle-hibernate-key", InhibitBehavior::HandleHibernateKey },
        { "handle-lid-switch", InhibitBehavior::HandleLidSwitch }
    };
    int result = 0;
    QStringList behaviors = behavior.split(":");
    foreach (const QString &strBehavior, behaviors) {
        if (behaviorMap.contains(strBehavior)) {
            result |= behaviorMap[strBehavior];
        }
    }
    return result;
}

QString modeToString(const InhibitMode &mode)
{
    static const QMap<InhibitMode, QString> modeMap = { { InhibitMode::Block, "block" },
                                                        { InhibitMode::Delay, "delay" } };
    return modeMap[mode];
}

InhibitMode stringToMode(const QString &strMode)
{
    if (strMode == "block") {
        return InhibitMode::Block;
    } else {
        return InhibitMode::Delay;
    }
}

DBusInhibitor inhibitorToDBus(const Inhibitor &inhibitor)
{
    DBusInhibitor dBusInhibitor;
    dBusInhibitor.what = decodeBehavior(inhibitor.what);
    dBusInhibitor.who = inhibitor.who;
    dBusInhibitor.why = inhibitor.why;
    dBusInhibitor.mode = modeToString(inhibitor.mode);
    dBusInhibitor.PID = inhibitor.PID;
    dBusInhibitor.UID = inhibitor.UID;
    return dBusInhibitor;
}

Inhibitor inhibitorFromDBus(const DBusInhibitor &inhibitorDBus)
{
    Inhibitor inhibitor;
    inhibitor.what = encodeBehavior(inhibitorDBus.what);
    inhibitor.who = inhibitorDBus.who;
    inhibitor.why = inhibitorDBus.why;
    inhibitor.mode = stringToMode(inhibitorDBus.mode);
    inhibitor.PID = inhibitorDBus.PID;
    inhibitor.UID = inhibitorDBus.UID;
    return inhibitor;
}

QString statusToString(const ExecuteStatus &status)
{
    static const QMap<ExecuteStatus, QString> statusMap = { { ExecuteStatus::Yes, "yes" },
                                                            { ExecuteStatus::No, "no" },
                                                            { ExecuteStatus::Challenge,
                                                              "challenge" },
                                                            { ExecuteStatus::Na, "na" },
                                                            { ExecuteStatus::Unknown, "" } };
    return statusMap[status];
}

ExecuteStatus stringToStatus(const QString &strStatus)
{
    static const QMap<QString, ExecuteStatus> statusMap = { { "yes", ExecuteStatus::Yes },
                                                            { "no", ExecuteStatus::No },
                                                            { "challenge",
                                                              ExecuteStatus::Challenge },
                                                            { "na", ExecuteStatus::Na } };
    if (statusMap.contains(strStatus)) {
        return statusMap[strStatus];
    } else {
        return ExecuteStatus::Unknown;
    }
}

QString sessionRoleToString(const SessionRole &sessionRole)
{
    static const QMap<SessionRole, QString> roleMap = { { SessionRole::All, "all" },
                                                        { SessionRole::Leader, "leader" },
                                                        { SessionRole::Unknown, "" } };
    return roleMap[sessionRole];
}

SessionRole stringToSessionRole(const QString &strSessionRole)
{
    if (strSessionRole == "leader") {
        return SessionRole::Leader;
    } else if (strSessionRole == "all") {
        return SessionRole::All;
    } else {
        return SessionRole::Unknown;
    }
}

PowerAction stringToAction(const QString &strAction)
{
    static const QMap<QString, PowerAction> actionMap = {
        { "poweroff", PowerAction::PowerOff },
        { "reboot", PowerAction::Reboot },
        { "halt", PowerAction::Halt },
        { "kexec", PowerAction::KExec },
        { "suspend", PowerAction::Suspend },
        { "hibernate", PowerAction::Hibernate },
        { "hybrid-sleep", PowerAction::HybridSleep },
        { "suspend-then-hibernate", PowerAction::SuspendThenHibernate },
        { "lock", PowerAction::Lock },
        { "ignore", PowerAction::Ignore }
    };
    if (actionMap.contains(strAction)) {
        return actionMap[strAction];
    } else {
        return PowerAction::Unknown;
    }
}

QString actionToString(const PowerAction &action)
{
    static const QMap<PowerAction, QString> actionMap = {
        { PowerAction::PowerOff, "poweroff" },
        { PowerAction::Reboot, "reboot" },
        { PowerAction::Halt, "halt" },
        { PowerAction::KExec, "kexec" },
        { PowerAction::Suspend, "suspend" },
        { PowerAction::Hibernate, "hibernate" },
        { PowerAction::HybridSleep, "hybrid-sleep" },
        { PowerAction::SuspendThenHibernate, "suspend-then-hibernate" },
        { PowerAction::FactoryReset, "factory-reset" },
        { PowerAction::Lock, "lock" },
        { PowerAction::Ignore, "ignore" },
        { PowerAction::Unknown, "" }
    };
    return actionMap[action];
}

QString shutdownTypeToString(const ShutdownType &type)
{
    static const QMap<ShutdownType, QString> typeMap = {
        { ShutdownType::PowerOff, "poweroff" }, { ShutdownType::DryPowerOff, "dry-poweroff" },
        { ShutdownType::Reboot, "reboot" },     { ShutdownType::DryReboot, "dry-reboot" },
        { ShutdownType::Halt, "halt" },         { ShutdownType::DryHalt, "dry-halt" },
        { ShutdownType::Unknown, "" }
    };
    return typeMap[type];
}

ShutdownType stringToShutdownType(const QString &strType)
{
    static const QMap<QString, ShutdownType> typeMap = {
        { "poweroff", ShutdownType::PowerOff }, { "dry-poweroff", ShutdownType::DryPowerOff },
        { "reboot", ShutdownType::Reboot },     { "dry-reboot", ShutdownType::DryReboot },
        { "halt", ShutdownType::Halt },         { "dry-halt", ShutdownType::DryHalt }
    };
    if (typeMap.contains(strType)) {
        return typeMap[strType];
    } else {
        return ShutdownType::Unknown;
    }
}

QString sessionClassToString(const SessionClass &sessionClass)
{
    static const QMap<SessionClass, QString> classMap = { { SessionClass::User, "user" },
                                                          { SessionClass::Greeter, "greeter" },
                                                          { SessionClass::LockScreen,
                                                            "lock-screen" },
                                                          { SessionClass::Unknown, "" } };
    return classMap[sessionClass];
}

SessionClass stringToSessionClass(const QString &strClass)
{
    if (strClass == "user") {
        return SessionClass::User;
    } else if (strClass == "greeter") {
        return SessionClass::Greeter;
    } else if (strClass == "lock-screen") {
        return SessionClass::LockScreen;
    } else {
        return SessionClass::Unknown;
    }
}

QString sessionTypeToString(const SessionType &sessionType)
{
    static const QMap<SessionType, QString> sessionMap = { { SessionType::TTY, "tty" },
                                                           { SessionType::X11, "x11" },
                                                           { SessionType::Mir, "mir" },
                                                           { SessionType::Wayland, "wayland" },
                                                           { SessionType::Unspecified,
                                                             "unspecified" } };
    return sessionMap[sessionType];
}

SessionType stringToSessionType(const QString &strType)
{
    static const QMap<QString, SessionType> sessionMap = { { "tty", SessionType::TTY },
                                                           { "x11", SessionType::X11 },
                                                           { "mir", SessionType::Mir },
                                                           { "wayland", SessionType::Wayland },
                                                           { "unspecified",
                                                             SessionType::Unspecified } };
    if (sessionMap.contains(strType)) {
        return sessionMap[strType];
    } else {
        return SessionType::Unspecified;
    }
}

QString sessionStateToString(const SessionState &sessionState)
{
    static const QMap<SessionState, QString> stateMap = { { SessionState::Online, "online" },
                                                          { SessionState::Active, "active" },
                                                          { SessionState::Closing, "closing" },
                                                          { SessionState::Unknown, "" } };
    return stateMap[sessionState];
}

SessionState stringToSessionState(const QString &strState)
{
    if (strState == "online") {
        return SessionState::Online;
    } else if (strState == "active") {
        return SessionState::Active;
    } else if (strState == "closing") {
        return SessionState::Closing;
    } else {
        return SessionState::Unknown;
    }
}

QString userStateToString(const UserState &userState)
{
    static const QMap<UserState, QString> stateMap = {
        { UserState::Offline, "offline" }, { UserState::Lingering, "lingering" },
        { UserState::Online, "online" },   { UserState::Active, "active" },
        { UserState::Closing, "closing" }, { UserState::Unknown, "" }
    };
    return stateMap[userState];
}

UserState stringToUserState(const QString &strState)
{
    static const QMap<QString, UserState> stateMap = { { "offline", UserState::Offline },
                                                       { "lingering", UserState::Lingering },
                                                       { "online", UserState::Online },
                                                       { "active", UserState::Active },
                                                       { "closing", UserState::Closing } };
    if (stateMap.contains(strState)) {
        return stateMap[strState];
    } else {
        return UserState::Unknown;
    }
}

QString scheduledShutdownValueToString(const ScheduledShutdownValue &value)
{
    return "{type: " + shutdownTypeToString(value.type) + ", time: " + value.time.toString() + "}";
}

QString inhibitorToString(const Inhibitor &inhibitor)
{
    QString str = "{";
    str += "what: " + Utils::decodeBehavior(inhibitor.what) + ", ";
    str += "who: " + inhibitor.who + ", ";
    str += "why: " + inhibitor.why + ", ";
    str += "mode: " + Utils::modeToString(inhibitor.mode) + ", ";
    str += "UID: " + QString::number(inhibitor.UID) + ", ";
    str += "PID: " + QString::number(inhibitor.PID) + "}";
    return str;
}

bool registerAllStringConverter()
{
    static bool registered = false;
    if (!registered) {
        QMetaType::registerConverter<ShutdownType, QString, QString (*)(const ShutdownType &)>(
                shutdownTypeToString);
        QMetaType::registerConverter<PowerAction, QString, QString (*)(const PowerAction &)>(
                actionToString);
        QMetaType::registerConverter<ExecuteStatus, QString, QString (*)(const ExecuteStatus &)>(
                statusToString);
        QMetaType::registerConverter<SessionRole, QString, QString (*)(const SessionRole &)>(
                sessionRoleToString);
        QMetaType::registerConverter<InhibitMode, QString, QString (*)(const InhibitMode &)>(
                modeToString);
        QMetaType::registerConverter<SessionState, QString, QString (*)(const SessionState &)>(
                sessionStateToString);
        QMetaType::registerConverter<SessionType, QString, QString (*)(const SessionType &)>(
                sessionTypeToString);
        QMetaType::registerConverter<SessionClass, QString, QString (*)(const SessionClass &)>(
                sessionClassToString);
        QMetaType::registerConverter<UserState, QString, QString (*)(const UserState &)>(
                userStateToString);
        QMetaType::registerConverter<ScheduledShutdownValue,
                                     QString,
                                     QString (*)(const ScheduledShutdownValue &)>(
                scheduledShutdownValueToString);
        QMetaType::registerConverter<Inhibitor, QString, QString (*)(const Inhibitor &)>(
                inhibitorToString);
        registered = true;
        return true;
    } else {
        return false;
    }
}

} // namespace Utils
DLOGIN_END_NAMESPACE
