// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "log.h"

namespace Lingmo::Services {

const Lingmo::Base::LogCategory &log()
{
    static const Lingmo::Base::LogCategory category("lingmo.services");
    return category;
}

} // namespace Lingmo::Services
