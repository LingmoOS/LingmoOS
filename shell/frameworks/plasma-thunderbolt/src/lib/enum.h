/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef BOLT_ENUM_H_
#define BOLT_ENUM_H_

#include "kbolt_export.h"

#include <QMetaObject>
#include <QMetaType>
#include <QString>

namespace Bolt
{
// NOTE: Keep this split over two lines otherwise MOC may fail to see
// the Q_NAMESPACE macro if KBOLT_EXPORT is not expanded correctly.
KBOLT_EXPORT
Q_NAMESPACE

enum class Status {
    Unknown = -1,
    Disconnected,
    Connecting,
    Connected,
    Authorizing,
    AuthError,
    Authorized,
};

Q_ENUM_NS(Status)

Status statusFromString(const QString &str);
QString statusToString(Status status);

enum class Auth {
    None = 0,
    NoPCIE = 1 << 0,
    Secure = 1 << 1,
    NoKey = 1 << 2,
    Boot = 1 << 3,
};
Q_ENUM_NS(Auth)
Q_DECLARE_FLAGS(AuthFlags, Auth)

AuthFlags authFlagsFromString(const QString &str);
QString authFlagsToString(AuthFlags flags);

enum class KeyState {
    Unknown = -1,
    Missing,
    Have,
    New,
};
Q_ENUM_NS(KeyState)

KeyState keyStateFromString(const QString &str);

enum class Policy {
    Unknown = -1,
    Default,
    Manual,
    Auto,
};
Q_ENUM_NS(Policy)

Policy policyFromString(const QString &str);
QString policyToString(Policy policy);

enum class Type {
    Unknown = -1,
    Host,
    Peripheral,
};
Q_ENUM_NS(Type)

Type typeFromString(const QString &str);

enum class AuthMode {
    Disabled = 0,
    Enabled,
};
Q_ENUM_NS(AuthMode)

AuthMode authModeFromString(const QString &str);
QString authModeToString(AuthMode authMode);

enum class Security {
    Unknown = -1,
    None,
    DPOnly,
    User = '1', /* sic! */
    Secure = '2', /* sic! */
    USBOnly = 4,
};
Q_ENUM_NS(Security)

Security securityFromString(const QString &str);

} // namespace

Q_DECLARE_METATYPE(Bolt::Status)
Q_DECLARE_METATYPE(Bolt::AuthFlags)
Q_DECLARE_METATYPE(Bolt::KeyState)
Q_DECLARE_METATYPE(Bolt::Policy)
Q_DECLARE_METATYPE(Bolt::Type)
Q_DECLARE_METATYPE(Bolt::AuthMode)
Q_DECLARE_METATYPE(Bolt::Security)
Q_DECLARE_OPERATORS_FOR_FLAGS(Bolt::AuthFlags)

#endif
