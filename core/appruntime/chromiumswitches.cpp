// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chromiumswitches.h"

#include "base/paths.h"

#include <filesystem>
#include <QString>

namespace fs = std::filesystem;

static bool pathExists(const fs::path &path)
{
    std::error_code error;
    return fs::exists(path, error);
}

static bool hasChromiumResources(const fs::path &directory)
{
    return pathExists(directory / "icudtl.dat")
        && pathExists(directory / "chrome_100_percent.pak");
}

bool isChromiumProgram(const QString &program)
{
    const std::string path = program.contains(QLatin1Char('/'))
        ? program.toStdString()
        : Lingmo::Base::paths::findExecutable(program.toStdString());
    if (path.empty())
        return false;

    std::error_code error;
    const fs::path binary = fs::canonical(path, error);
    if (error)
        return false;

    return hasChromiumResources(binary.parent_path())
        || hasChromiumResources(fs::path("/usr/lib") / binary.filename());
}

QStringList withChromiumSwitches(const QStringList &command)
{
    if (command.isEmpty() || !isChromiumProgram(command.front()))
        return command;

    QStringList result = command;
    result.insert(1, QStringLiteral("--disable-font-subpixel-positioning"));
    return result;
}
