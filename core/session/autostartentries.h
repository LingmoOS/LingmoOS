// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QStringList>

namespace Lingmo {

// Returns true for programs the session starts itself, so autostart
// entries cannot start them a second time.
bool isStartedBySession(const QString &executable);

// Collect Exec= values from valid autostart .desktop files.
// Directories are ordered from highest to lowest precedence;
// a name seen in an earlier directory masks the same name in later ones.
QStringList autostartEntries(const QStringList &directories,
                              const QStringList &desktops);

} // namespace Lingmo
