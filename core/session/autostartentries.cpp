// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "autostartentries.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>
#include <unistd.h>

#include "base/paths.h"

namespace base = Lingmo::Base;

namespace Lingmo {

static QStringList desktopFiles(const QString &directory)
{
    QStringList names;
    QDir dir(directory);
    const QStringList entries = dir.entryList({QStringLiteral("*.desktop")},
                                                QDir::Files);
    for (const QString &name : entries) {
        if (QFileInfo(dir.filePath(name)).isFile())
            names << name;
    }
    std::sort(names.begin(), names.end());
    return names;
}

static bool isExecutableFile(const QString &path)
{
    return QFileInfo(path).isFile() && ::access(path.toLocal8Bit().constData(), X_OK) == 0;
}

bool isStartedBySession(const QString &executable)
{
    static const QStringList programs = {
        QStringLiteral("lingmo-services"),
        QStringLiteral("lingmo-appruntime"),
        QStringLiteral("lingmo-filemanager"),
        QStringLiteral("lingmo-aurora"),
        QStringLiteral("lingmo-polkit-agent"),
        QStringLiteral("xdg-user-dirs-update"),
    };
    return programs.contains(executable);
}

QStringList autostartEntries(const QStringList &directories,
                             const QStringList &desktops)
{
    QStringList result;
    QSet<QString> seen;

    for (const QString &directory : directories) {
        for (const QString &name : desktopFiles(directory)) {
            // A disabled or invalid override still masks lower-priority entries.
            if (seen.contains(name))
                continue;
            seen.insert(name);

            // QSettings parses INI-style files natively, with the [Desktop Entry]
            // group handled automatically as "Desktop Entry/".
            QSettings desktop(QDir(directory).filePath(name), QSettings::IniFormat);
            desktop.beginGroup(QStringLiteral("Desktop Entry"));

            const QString type = desktop.value(QStringLiteral("Type")).toString();
            if (type != QLatin1String("Application"))
                continue;

            const bool hidden = desktop.value(QStringLiteral("Hidden")).toBool();
            const bool gnomeDisabled =
                desktop.value(QStringLiteral("X-GNOME-Autostart-enabled"), true).toBool();
            if (hidden || !gnomeDisabled)
                continue;

            const auto matchesDesktops = [&](const QString &key) {
                const QString raw = desktop.value(key).toString();
                if (raw.isEmpty())
                    return false;
                const QStringList names = raw.split(QLatin1Char(';'),
                                                     Qt::SkipEmptyParts);
                for (const QString &d : desktops) {
                    if (names.contains(d))
                        return true;
                }
                return false;
            };

            // OnlyShowIn: if present, must contain one of our desktops.
            const bool hasOnlyShowIn = desktop.contains(QStringLiteral("OnlyShowIn"));
            if (hasOnlyShowIn && !matchesDesktops(QStringLiteral("OnlyShowIn")))
                continue;
            // NotShowIn: if one of our desktops is listed, skip the entry.
            if (matchesDesktops(QStringLiteral("NotShowIn")))
                continue;

            const QString tryExec = desktop.value(QStringLiteral("TryExec")).toString();
            if (!tryExec.isEmpty()) {
                if (tryExec.startsWith(QLatin1Char('/'))) {
                    if (!isExecutableFile(tryExec))
                        continue;
                } else if (tryExec.contains(QLatin1Char('/'))
                           || QString::fromStdString(
                                  base::paths::findExecutable(tryExec.toStdString())).isEmpty()) {
                    continue;
                }
            }

            const QString command = desktop.value(QStringLiteral("Exec")).toString().trimmed();
            if (!command.isEmpty())
                result << command;
        }
    }
    return result;
}

} // namespace Lingmo
