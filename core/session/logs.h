// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/logging.h"

inline const Lingmo::Base::LogCategory &sessionLog()
{
    static const Lingmo::Base::LogCategory category("lingmo.session");
    return category;
}

// LM_* macros are defined in base/logging.h.
// Use them directly: LM_INFO(sessionLog()) << "message";
