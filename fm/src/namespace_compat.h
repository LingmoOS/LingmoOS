// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// The sources name the framework namespace files::; it is Lingmo::Files. The
// project's compile options force-include this header so the alias is visible
// in every translation unit.
#pragma once

#ifdef __cplusplus

namespace Lingmo {
namespace Files {}
} // namespace Lingmo

namespace files = Lingmo::Files;

#endif // __cplusplus
