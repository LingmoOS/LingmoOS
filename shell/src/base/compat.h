// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Force-included before every compositor translation unit via -include.
// Extends the toolkit's base/compat.h (base::, ui::, dbus::, i18n::) with
// the compositor's own short-form namespace aliases so that sources using
// compositor::, core::, display::, render::, and motion:: still compile.
//
// This file is found first because -I src/ precedes -isystem lingmo-sdk/
// in the compiler invocation. It uses #include_next to pull in the SDK copy.

#pragma once

#ifdef __cplusplus

// Pull in the toolkit's aliases (base::, ui::, dbus::, i18n::).
// #include_next skips the current directory and picks up the next
// base/compat.h on the include path, which is the installed SDK copy.
#include_next "base/compat.h"

// ── Compositor-internal namespace aliases ──────────────────────────────────
//
// The namespaces are only forward-declared here so that the file compiles
// without requiring every header that defines them. The actual definitions
// are provided by the headers each translation unit already includes.

namespace Lingmo::Compositor {}
namespace Lingmo::Core {}
namespace Lingmo::Display {}
namespace Lingmo::Render {}
namespace Lingmo::Scene {}
namespace Lingmo::Scene::Motion {}

// Short aliases matching the pre-alias source spelling.
namespace compositor = Lingmo::Compositor;
namespace core       = Lingmo::Core;
namespace display    = Lingmo::Display;
namespace render     = Lingmo::Render;
namespace motion     = Lingmo::Scene::Motion;

#endif // __cplusplus
