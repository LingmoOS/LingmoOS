// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace fm {

// ~/.config/lingmo/filemanager.conf, the file manager's settings, which
// this process alone writes.
std::string settingsFile();
// Another file for every setting, for tests and tools; before first use.
void setSettingsFile(std::string path);

} // namespace fm
