/*
 *   SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QDBusConnection>
#include <QDBusInterface>

// clang-format off

#define KAMD_DBUS_SERVICE                                                      \
    QStringLiteral("org.kde.ActivityManager")

#define KAMD_DBUS_OBJECT_PATH(A)                                               \
    (sizeof(A) > 2 ? QLatin1String("/ActivityManager/" A)                      \
                   : QLatin1String("/ActivityManager"))

#define KAMD_DBUS_OBJECT(A)                                                    \
    QLatin1String("org.kde.ActivityManager." A)


