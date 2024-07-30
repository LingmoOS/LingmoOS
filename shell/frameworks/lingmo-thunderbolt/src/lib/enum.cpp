/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "enum.h"
#include "libkbolt_debug.h"

#include <QList>
#include <QStringList>
#include <QStringView>

Bolt::Status Bolt::statusFromString(const QString &str)
{
    if (str == QLatin1String("unknown")) {
        return Bolt::Status::Unknown;
    } else if (str == QLatin1String("disconnected")) {
        return Bolt::Status::Disconnected;
    } else if (str == QLatin1String("connecting")) {
        return Bolt::Status::Connecting;
    } else if (str == QLatin1String("connected")) {
        return Bolt::Status::Connected;
    } else if (str == QLatin1String("authorizing")) {
        return Bolt::Status::Authorizing;
    } else if (str == QLatin1String("authorized")) {
        return Bolt::Status::Authorized;
    } else if (str == QLatin1String("auth-error")) {
        return Bolt::Status::AuthError;
    } else {
        qCCritical(log_libkbolt, "Unknown Status enum value '%s'", qUtf8Printable(str));
        Q_ASSERT(false);
        return Bolt::Status::Unknown;
    }
}

QString Bolt::statusToString(Bolt::Status status)
{
    switch (status) {
    case Bolt::Status::Unknown:
        return QStringLiteral("unknown");
    case Bolt::Status::Disconnected:
        return QStringLiteral("disconnected");
    case Bolt::Status::Connecting:
        return QStringLiteral("connecting");
    case Bolt::Status::Connected:
        return QStringLiteral("connected");
    case Bolt::Status::Authorizing:
        return QStringLiteral("authorizing");
    case Bolt::Status::Authorized:
        return QStringLiteral("authorized");
    case Bolt::Status::AuthError:
        return QStringLiteral("auth-error");
    }
    Q_UNREACHABLE();
    return {};
}

Bolt::AuthFlags Bolt::authFlagsFromString(const QString &str)
{
    const auto splitRef = str.split(QStringLiteral("|"));
    Bolt::AuthFlags outFlags = Bolt::Auth::None;
    for (const auto &flag : splitRef) {
        const auto f = flag.trimmed();
        if (f == QLatin1String("none")) {
            outFlags |= Bolt::Auth::None;
        } else if (f == QLatin1String("nopcie")) {
            outFlags |= Bolt::Auth::NoPCIE;
        } else if (f == QLatin1String("secure")) {
            outFlags |= Bolt::Auth::Secure;
        } else if (f == QLatin1String("nokey")) {
            outFlags |= Bolt::Auth::NoKey;
        } else if (f == QLatin1String("boot")) {
            outFlags |= Bolt::Auth::Boot;
        } else {
            qCCritical(log_libkbolt, "Unknown AuthFlags enum value '%s'", qUtf8Printable(str));
            Q_ASSERT(false);
            return Bolt::Auth::None;
        }
    }
    return outFlags;
}

QString Bolt::authFlagsToString(AuthFlags flags)
{
    QStringList str;
    if (flags == AuthFlags(Bolt::Auth::None)) {
        return QStringLiteral("none");
    }
    if (flags & Bolt::Auth::NoPCIE) {
        str.push_back(QStringLiteral("nopcie"));
    }
    if (flags & Bolt::Auth::Secure) {
        str.push_back(QStringLiteral("secure"));
    }
    if (flags & Bolt::Auth::NoKey) {
        str.push_back(QStringLiteral("nokey"));
    }
    if (flags & Bolt::Auth::Boot) {
        str.push_back(QStringLiteral("boot"));
    }

    return str.join(QStringLiteral(" | "));
}

Bolt::KeyState Bolt::keyStateFromString(const QString &str)
{
    if (str == QLatin1String("unknown")) {
        return Bolt::KeyState::Unknown;
    } else if (str == QLatin1String("missing")) {
        return Bolt::KeyState::Missing;
    } else if (str == QLatin1String("have")) {
        return Bolt::KeyState::Have;
    } else if (str == QLatin1String("new")) {
        return Bolt::KeyState::New;
    } else {
        qCCritical(log_libkbolt, "Unknown KeyState enum value '%s'", qUtf8Printable(str));
        Q_ASSERT(false);
        return Bolt::KeyState::Unknown;
    }
}

Bolt::Policy Bolt::policyFromString(const QString &str)
{
    if (str == QLatin1String("unknown")) {
        return Bolt::Policy::Unknown;
    } else if (str == QLatin1String("default")) {
        return Bolt::Policy::Default;
    } else if (str == QLatin1String("manual")) {
        return Bolt::Policy::Manual;
    } else if (str == QLatin1String("auto")) {
        return Bolt::Policy::Auto;
    } else {
        qCCritical(log_libkbolt, "Unknown Policy enum value '%s'", qUtf8Printable(str));
        Q_ASSERT(false);
        return Bolt::Policy::Unknown;
    }
}

QString Bolt::policyToString(Bolt::Policy policy)
{
    switch (policy) {
    case Policy::Unknown:
        return QStringLiteral("unknown");
    case Policy::Auto:
        return QStringLiteral("auto");
    case Policy::Default:
        return QStringLiteral("default");
    case Policy::Manual:
        return QStringLiteral("manual");
    }

    Q_UNREACHABLE();
    return {};
}

Bolt::Type Bolt::typeFromString(const QString &str)
{
    if (str == QLatin1String("unknown")) {
        return Bolt::Type::Unknown;
    } else if (str == QLatin1String("host")) {
        return Bolt::Type::Host;
    } else if (str == QLatin1String("peripheral")) {
        return Bolt::Type::Peripheral;
    } else {
        qCCritical(log_libkbolt, "Unknown Type enum value '%s'", qUtf8Printable(str));
        Q_ASSERT(false);
        return Bolt::Type::Unknown;
    }
}

Bolt::AuthMode Bolt::authModeFromString(const QString &str)
{
    if (str == QLatin1String("disabled")) {
        return Bolt::AuthMode::Disabled;
    } else if (str == QLatin1String("enabled")) {
        return Bolt::AuthMode::Enabled;
    } else {
        qCCritical(log_libkbolt, "Unknown AuthMode enum value '%s'", qUtf8Printable(str));
        Q_ASSERT(false);
        return Bolt::AuthMode::Disabled;
    }
}

QString Bolt::authModeToString(Bolt::AuthMode authMode)
{
    switch (authMode) {
    case Bolt::AuthMode::Enabled:
        return QStringLiteral("enabled");
    case Bolt::AuthMode::Disabled:
        return QStringLiteral("disabled");
    }

    Q_UNREACHABLE();
    return {};
}

Bolt::Security Bolt::securityFromString(const QString &str)
{
    if (str == QLatin1String("unknown")) {
        return Bolt::Security::Unknown;
    } else if (str == QLatin1String("none")) {
        return Bolt::Security::None;
    } else if (str == QLatin1String("dponly")) {
        return Bolt::Security::DPOnly;
    } else if (str == QLatin1String("user")) {
        return Bolt::Security::User;
    } else if (str == QLatin1String("secure")) {
        return Bolt::Security::Secure;
    } else if (str == QLatin1String("usbonly")) {
        return Bolt::Security::USBOnly;
    } else {
        qCCritical(log_libkbolt, "Unknown Security enum value '%s'", qUtf8Printable(str));
        Q_ASSERT(false);
        return Bolt::Security::Unknown;
    }
}

#include "moc_enum.cpp"
