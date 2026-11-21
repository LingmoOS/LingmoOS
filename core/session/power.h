// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// System power actions via systemd-logind.
// Each call blocks until logind answers and returns whether it accepted.
namespace Lingmo::Power {

bool reboot();
bool powerOff();
bool suspend();

} // namespace Lingmo::Power
