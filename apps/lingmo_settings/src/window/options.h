// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace settings {

// The command line's switches that pages read.
struct Options
{
    // Sample network data, for screenshots and tests.
    bool mockNetwork = false;
    // Sample battery data.
    bool mockBattery = false;
};

const Options& options();
void setOptions(const Options& options);

} // namespace settings
