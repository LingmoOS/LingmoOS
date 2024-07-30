/*
 * SPDX-FileCopyrightText: 2018-2019 Red Hat Inc
 * SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018-2019 Jan Grulich <jgrulich@redhat.com>
 */

#pragma once

#include <QDBusArgument>
#include <QMap>
#include <QString>

/// a{sa{sv}}
using VariantMapMap = QMap<QString, QMap<QString, QVariant>>;

/// sa{sv}
using Shortcut = QPair<QString, QVariantMap>;

/// a(sa{sv})
using Shortcuts = QList<Shortcut>;

Q_DECLARE_METATYPE(VariantMapMap)
Q_DECLARE_METATYPE(Shortcuts)
