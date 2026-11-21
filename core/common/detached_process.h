// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <sys/types.h>
#include <vector>

#include "common/environment.h"

namespace Lingmo::Core {

// Takes the oom_score_adj value in the file at `path` back to 0 when it is
// below, and never lowers it. Async-signal-safe, for between fork and exec.
void dropOomProtection(const char* path);

// Runs `command` (the program, looked up on PATH, and its arguments) in a
// session of its own with exactly `environment`; its end is collected on the
// calling thread's loop. It is an application, so it leaves the session's
// protection from the out-of-memory killer behind. The program's id, or -1
// when it cannot be started.
pid_t startDetached(const std::vector<std::string>& command, const std::string& workingDirectory,
                    const Environment& environment);

} // namespace Lingmo::Core
