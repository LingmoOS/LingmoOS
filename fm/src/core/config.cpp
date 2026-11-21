// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/config.h"

#include "base/paths.h"

namespace fm {

static std::string& overridden()
{
    static std::string path;
    return path;
}

std::string settingsFile()
{
    return overridden().empty() ? Lingmo::base::paths::lingmoConfig("filemanager") : overridden();
}

void setSettingsFile(std::string path)
{
    overridden() = std::move(path);
}

} // namespace fm
