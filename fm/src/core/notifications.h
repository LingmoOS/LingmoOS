// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>

namespace fm::notifications {

// A message in the notification centre, titled with the desktop entry's
// application; failed runs when no notification service answers.
void show(const std::string& desktopEntry, const std::string& icon, const std::string& body, int timeoutMs,
          std::function<void()> failed = {});

} // namespace fm::notifications
