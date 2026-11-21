// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "userdirectories.h"

#include <filesystem>
#include <utility>

#include "base/file.h"
#include "logs.h"

namespace fs = std::filesystem;

namespace Lingmo {

void UserDirectories::ensure(const QString &home, const QString &configHome)
{
    const QString file = configHome + QStringLiteral("/user-dirs.dirs");
    std::error_code error;
    if (home.isEmpty() || configHome.isEmpty()
        || fs::exists(file.toStdString(), error))
        return;

    static const std::pair<const char *, const char *> directories[] = {
        {"DESKTOP",     "Desktop"},
        {"DOWNLOAD",    "Downloads"},
        {"TEMPLATES",   "Templates"},
        {"PUBLICSHARE", "Public"},
        {"DOCUMENTS",   "Documents"},
        {"MUSIC",       "Music"},
        {"PICTURES",    "Pictures"},
        {"VIDEOS",      "Videos"},
    };

    QString contents;
    for (const auto &[key, name] : directories) {
        const QString path = home + QLatin1Char('/') + QLatin1String(name);
        fs::create_directories(path.toStdString(), error);
        if (!fs::is_directory(path.toStdString(), error)) {
            LM_WARNING(sessionLog()) << "Could not create" << path;
            continue;
        }
        contents += QStringLiteral("XDG_") + QLatin1String(key)
                  + QStringLiteral("_DIR=\"$HOME/") + QLatin1String(name)
                  + QStringLiteral("\"\n");
    }

    if (!Lingmo::Base::file::writeAtomically(file.toStdString(), contents.toStdString()))
        LM_WARNING(sessionLog()) << "Could not write" << file;
}

} // namespace Lingmo
