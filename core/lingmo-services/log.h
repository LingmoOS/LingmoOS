// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/logging.h"

namespace Lingmo::Services {

const Lingmo::Base::LogCategory &log();

} // namespace Lingmo::Services

// The service sources log through the shorter name.
namespace services = Lingmo::Services;
