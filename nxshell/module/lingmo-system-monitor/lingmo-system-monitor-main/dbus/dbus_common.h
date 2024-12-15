// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_COMMON_H
#define DBUS_COMMON_H

#include <QMap>
#include <QVariant>
#include <QtDBus>
#include <QtGlobal>

#include <string.h>

/**
* @brief namespace dbus::common
*/
namespace dbus {
namespace common {

#define DBUS_SYSTEMD1_SERVICE "org.freedesktop.systemd1"
const QDBusObjectPath kSystemDObjectPath {"/org/freedesktop/systemd1"};

constexpr const char *UnitTypeServiceSuffix = ".service";

enum DBusSignatureType {
    kDBusByteType,
    kDBusBooleanType,
    kDBusInt16Type,
    kDBusUInt16Type,
    kDBusInt32Type,
    kDBusUInt32Type,
    kDBusInt64Type,
    kDBusUInt64Type,
    kDBusDoubleType,
    kDBusStringType,
    kDBusObjectType,
    kDBusSignatureType,
    kDBusArrayType,
    kDBusStructStartType,
    kDBusStructEndType,
    kDBusVariantType,
    kDBusMapStartType,
    kDBusMapEndType,
    kDBusFDType,

    kDBusInvalidType
};

// Not a valid type code, used to terminate signatures
const char kDBusInvalidTypeSignature = '\0';
// 8-bit unsigned integer
const char kDBusByteTypeSignature = 'y';
// Boolean value, 0 is FALSE and 1 is TRUE. Everything else is invalid.
const char kDBusBooleanTypeSignature = 'b';
// 16-bit signed integer
const char kDBusInt16TypeSignature = 'n';
// 16-bit unsigned integer
const char kDBusUInt16TypeSignature = 'q';
// 32-bit signed integer
const char kDBusInt32TypeSignature = 'i';
// 32-bit unsigned integer
const char kDBusUInt32TypeSignature = 'u';
// 64-bit signed integer
const char kDBusInt64TypeSignature = 'x';
// 64-bit unsigned integer
const char kDBusUInt64TypeSignature = 't';
// IEEE 754 double
const char kDBusDoubleTypeSignature = 'd';
// UTF-8 string (must be valid UTF-8). Must be nul terminated and contain no other nul bytes.
const char kDBusStringTypeSignature = 's';
// Name of an object instance
const char kDBusObjectTypeSignature = 'o';
// A type signature
const char kDBusSignatureTypeSignature = 'g';
// Array
const char kDBusArrayTypeSignature = 'a';
// Struct
const char kDBusStructStartTypeSignature = '(';
const char kDBusStructEndTypeSignature = ')';
// Variant type (the type of the value is part of the value itself)
const char kDBusVariantTypeSignature = 'v';
// Entry in a dict or map (array of key-value pairs)
const char kDBusMapStartTypeSignature = '{';
const char kDBusMapEndTypeSignature = '}';
// Unix file descriptor
const char kDBusFDTypeSignature = 'h';

const QMap<char, DBusSignatureType> DBusSignatureToTypeMap = {
    {kDBusInvalidTypeSignature, kDBusInvalidType},
    {kDBusByteTypeSignature, kDBusByteType},
    {kDBusBooleanTypeSignature, kDBusBooleanType},
    {kDBusInt16TypeSignature, kDBusInt16Type},
    {kDBusUInt16TypeSignature, kDBusUInt16Type},
    {kDBusInt32TypeSignature, kDBusInt32Type},
    {kDBusUInt32TypeSignature, kDBusUInt32Type},
    {kDBusInt64TypeSignature, kDBusInt64Type},
    {kDBusUInt64TypeSignature, kDBusUInt64Type},
    {kDBusDoubleTypeSignature, kDBusDoubleType},
    {kDBusStringTypeSignature, kDBusStringType},
    {kDBusObjectTypeSignature, kDBusObjectType},
    {kDBusSignatureTypeSignature, kDBusSignatureType},
    {kDBusArrayTypeSignature, kDBusArrayType},
    {kDBusStructStartTypeSignature, kDBusStructStartType},
    {kDBusStructEndTypeSignature, kDBusStructEndType},
    {kDBusVariantTypeSignature, kDBusVariantType},
    {kDBusMapStartTypeSignature, kDBusMapStartType},
    {kDBusMapEndTypeSignature, kDBusMapEndType},
    {kDBusFDTypeSignature, kDBusFDType}
};

const QMap<DBusSignatureType, char> DBusTypeToSignatureMap = {
    {kDBusInvalidType, kDBusInvalidTypeSignature},
    {kDBusByteType, kDBusByteTypeSignature},
    {kDBusBooleanType, kDBusBooleanTypeSignature},
    {kDBusInt16Type, kDBusInt16TypeSignature},
    {kDBusUInt16Type, kDBusUInt16TypeSignature},
    {kDBusInt32Type, kDBusInt32TypeSignature},
    {kDBusUInt32Type, kDBusUInt32TypeSignature},
    {kDBusInt64Type, kDBusInt64TypeSignature},
    {kDBusUInt64Type, kDBusUInt64TypeSignature},
    {kDBusDoubleType, kDBusDoubleTypeSignature},
    {kDBusStringType, kDBusStringTypeSignature},
    {kDBusObjectType, kDBusObjectTypeSignature},
    {kDBusSignatureType, kDBusSignatureTypeSignature},
    {kDBusArrayType, kDBusArrayTypeSignature},
    {kDBusStructStartType, kDBusStructStartTypeSignature},
    {kDBusStructEndType, kDBusStructEndTypeSignature},
    {kDBusVariantType, kDBusVariantTypeSignature},
    {kDBusMapStartType, kDBusMapStartTypeSignature},
    {kDBusMapEndType, kDBusMapEndTypeSignature},
    {kDBusFDType, kDBusFDTypeSignature}
};

// unit load states
enum UnitLoadState {
    kUnitStubState,
    kUnitLoadedState,
    kUnitNotFoundState,
    kUnitBadSettingState,
    kUnitErrorState,
    kUnitMergedState,
    kUnitMaskedState
};
constexpr const char *kUnitStubStateText = QT_TRANSLATE_NOOP("DBus.Unit.Load.State", "stub");
constexpr const char *kUnitLoadedStateText = QT_TRANSLATE_NOOP("DBus.Unit.Load.State", "loaded");
constexpr const char *kUnitNotFoundStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Load.State", "not-found");
constexpr const char *kUnitBadSettingStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Load.State", "bad-setting");
constexpr const char *kUnitErrorStateText = QT_TRANSLATE_NOOP("DBus.Unit.Load.State", "error");
constexpr const char *kUnitMergedStateText = QT_TRANSLATE_NOOP("DBus.Unit.Load.State", "merged");
constexpr const char *kUnitMaskedStateText = QT_TRANSLATE_NOOP("DBus.Unit.Load.State", "masked");

// unit active states
enum UnitActiveState {
    kUnitActiveState,
    kUnitReloadingState,
    kUnitInactiveState,
    kUnitFailedState,
    kUnitActivatingState,
    kUnitDeactivatingState
};
constexpr const char *kUnitActiveStateText = QT_TRANSLATE_NOOP("DBus.Unit.Active.State", "active");
constexpr const char *kUnitReloadingStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Active.State", "reloading");
constexpr const char *kUnitInactiveStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Active.State", "inactive");
constexpr const char *kUnitFailedStateText = QT_TRANSLATE_NOOP("DBus.Unit.Active.State", "failed");
constexpr const char *kUnitActivatingStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Active.State", "activating");
constexpr const char *kUnitDeactivatingStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Active.State", "deactivating");

/**
* @brief isFinalState Check if the state string is in final state
* @param state State text string
* @return true: state in final state; false: state in non-final state
*/
inline bool isFinalState(const char *state)
{
    bool b = false;
    b |= !strcmp(state, kUnitActiveStateText);
    b |= !strcmp(state, kUnitInactiveStateText);
    b |= !strcmp(state, kUnitFailedStateText);

    return b;
}

inline bool isActiveState(const char *state)
{
    bool b = !strcmp(state, "active");
    return b;
}

// service unit substates
enum ServiceUnitSubState {
    kServiceUnitDeadSubState,
    kServiceUnitStartPreSubState,
    kServiceUnitStartSubState,
    kServiceUnitStartPostSubState,
    kServiceUnitRunningSubState,
    kServiceUnitExitedSubState,
    kServiceUnitReloadSubState,
    kServiceUnitStopSubState,
    kServiceUnitStopWatchdogSubState,
    kServiceUnitStopSigTermSubState,
    kServiceUnitStopSigKillSubState,
    kServiceUnitStopPostSubState,
    kServiceUnitFinalSigTermSubState,
    kServiceUnitFinalSigKillSubState,
    kServiceUnitFailedSubState,
    kServiceUnitAutoRestartSubState
};
constexpr const char *kServiceUnitDeadSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "dead");
constexpr const char *kServiceUnitStartPreSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "start-pre");
constexpr const char *kServiceUnitStartSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "start");
constexpr const char *kServiceUnitStartPostSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "start-post");
constexpr const char *kServiceUnitRunningSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "running");
constexpr const char *kServiceUnitExitedSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "exited");
constexpr const char *kServiceUnitReloadSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "reload");
constexpr const char *kServiceUnitStopSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "stop");
constexpr const char *kServiceUnitStopWatchdogSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "stop-watchdog");
constexpr const char *kServiceUnitStopSigTermSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "stop-sigterm");
constexpr const char *kServiceUnitStopSigKillSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "stop-sigkill");
constexpr const char *kServiceUnitStopPostSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "stop-post");
constexpr const char *kServiceUnitFinalSigTermSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "final-sigterm");
constexpr const char *kServiceUnitFinalSigKillSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "final-sigkill");
constexpr const char *kServiceUnitFailedSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "failed");
constexpr const char *kServiceUnitAutoRestartSubStateText =
    QT_TRANSLATE_NOOP("DBus.Unit.Sub.State", "auto-restart");

struct UnitFileChange {
    QString type;           // Type of the change (one of symlink or unlink)
    QString fileName;       // File name of the symlink
    QString destination;    // Destination of the symlink
};

struct EnableUnitFilesResult {
    bool hasInstallInfo;
    QList<QSharedPointer<UnitFileChange>> changes;
};

struct DisableUnitFilesResult {
    QList<QSharedPointer<UnitFileChange>> changes;
};

enum UnitState {
    kUnitStateEnabled,
    kUnitStateDisabled,
    kUnitStateStatic,
    kUnitStateTransient,
    kUnitStateIndirect,
    kUnitStateEnabledRuntime,
    kUnitStateMasked,
    kUnitStateGenerated
};
constexpr const char *kUnitStateEnabledText = QT_TRANSLATE_NOOP("DBus.Unit.State", "enabled");
constexpr const char *kUnitStateDisabledText = QT_TRANSLATE_NOOP("DBus.Unit.State", "disabled");
constexpr const char *kUnitStateStaticText = QT_TRANSLATE_NOOP("DBus.Unit.State", "static");
constexpr const char *kUnitStateTransientText = QT_TRANSLATE_NOOP("DBus.Unit.State", "transient");
constexpr const char *kUnitStateIndirectText = QT_TRANSLATE_NOOP("DBus.Unit.State", "indirect");
constexpr const char *kUnitStateEnabledRuntimeText =
    QT_TRANSLATE_NOOP("DBus.Unit.State", "enabled-runtime");
constexpr const char *kUnitStateMaskedRuntimeText = QT_TRANSLATE_NOOP("DBus.Unit.State", "masked");
constexpr const char *kUnitStateGeneratedRuntimeText =
    QT_TRANSLATE_NOOP("DBus.Unit.State", "generated");
const QMap<QString, UnitState> unitStateMap = {
    {"enabled", kUnitStateEnabled},
    {"disabled", kUnitStateDisabled},
    {"static", kUnitStateStatic},
    {"transient", kUnitStateTransient},
    {"indirect", kUnitStateIndirect},
    {"enabled-runtime", kUnitStateEnabledRuntime},
    {"masked", kUnitStateMasked},
    {"generated", kUnitStateGenerated}
};

constexpr const char *kServiceAutoStartup = QT_TRANSLATE_NOOP("DBus.Unit.Startup.Mode", "Auto");
constexpr const char *kServiceManualStartup = QT_TRANSLATE_NOOP("DBus.Unit.Startup.Mode", "Manual");
constexpr const char *kServiceNAStartup = QT_TRANSLATE_NOOP("DBus.Unit.Startup.Mode", "N/A");

/**
 * @brief isUnitStateEnabled Check if unit is in enabled/disabled state, is-enabled (from
 * systemctl.c#unit_is_enabled)
 * @param state Unit state
 * @return true: enabled state; false: disabled state
 */
inline bool isUnitStateEnabled(const QString &state)
{
    if (state.isEmpty())
        return false;

    int s = unitStateMap[state];
    return (s == kUnitStateEnabled)
           || (s == kUnitStateEnabledRuntime)
           || (s == kUnitStateStatic);
}
/**
 * @brief isUnitNoOp Check if unit is in NoOp state
 * @param state Unit state
 * @return true: unit in NoOp state; false: otherwise
 */
inline bool isUnitNoOp(const QString &state)
{
    int s = unitStateMap[state];
    return (s == kUnitStateEnabledRuntime)
           || (s == kUnitStateStatic)
           || (s == kUnitStateIndirect)
           || (s == kUnitStateTransient)
           || (s == kUnitStateMasked)
           || state.isEmpty();
}
#define EXEC_SYSV_CHECK "/lib/systemd/systemd-sysv-install"
/**
 * @brief isSysVInitEnabled Check if sysv init script is auto/manual start
 * @param sname Service name
 * @return true: auto startup; false: manual startup
 */
inline bool isSysVInitEnabled(const QString &sname)
{
    int rc = QProcess::execute(EXEC_SYSV_CHECK, {"is-enabled", sname});
    // from /lib/systemd/systemd-sysv-install#is-enabled
    if (rc == 0) {
        return true; // enabled
    } else {
        return false;
    }
}
/**
 * @brief isServiceAutoStartup Check if service is auto/manual start
 * @param sname Service name
 * @param state Service state
 * @return true: auto startup; false: manual startup
 */
inline bool isServiceAutoStartup(const QString &sname, const QString &state)
{
    if (state == kUnitStateGeneratedRuntimeText || state == kUnitStateIndirectText) {
        return isSysVInitEnabled(sname);
    } else {
        return isUnitStateEnabled(state);
    }
}

enum UnitControlJobMode {
    kJobModeFail,
    kJobModeReplace,
    kJobModeReplaceIrreversibly,
    kJobModeIsolate,
    kJobModeIgnoreDependencies,
    kJobModeIgnoreRequirements,
    kJobModeFlush
};
constexpr const char *kJobModeFailCMD = "fail";
constexpr const char *kJobModeReplaceCMD = "replace";
constexpr const char *kJobModeReplaceIrreversiblyCMD = "replace-irreversibly";
constexpr const char *kJobModeIsolateCMD = "isolate";
constexpr const char *kJobModeIgnoreDependenciesCMD = "ignore-dependencies";
constexpr const char *kJobModeIgnoreRequirementsCMD = "ignore-requirements";
constexpr const char *kJobModeFlushCMD = "flush";

const QMap<int, const char *> UnitControlJobModeMap = {
    {kJobModeFail, kJobModeFailCMD},
    {kJobModeReplace, kJobModeReplaceCMD},
    {kJobModeReplaceIrreversibly, kJobModeReplaceIrreversiblyCMD},
    {kJobModeIsolate, kJobModeIsolateCMD},
    {kJobModeIgnoreDependencies, kJobModeIgnoreDependenciesCMD},
    {kJobModeIgnoreRequirements, kJobModeIgnoreRequirementsCMD},
    {kJobModeFlush, kJobModeFlushCMD}
};

} // namespace common
} // namespace dbus

#endif  // DBUS_COMMON_H
