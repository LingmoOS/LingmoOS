// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_CORE_LOGGING_H_
#define LINGMO_CORE_LOGGING_H_

#include <string_view>

#include "base/logging.h"

namespace Lingmo::Core {

// Written to with LM_DEBUG(lingmoInput) and friends.
using LogCategory = base::LogCategory;

// Applies `rules` (base::logging::setRules syntax) to every category.
void setLoggingRules(std::string_view rules);

} // namespace Lingmo::Core

// Startup and operational messages are on; routine window and input
// diagnostics need a debug rule (--log-rules).
extern const Lingmo::Core::LogCategory lingmoWayland;
extern const Lingmo::Core::LogCategory lingmoSession;
extern const Lingmo::Core::LogCategory lingmoDrm;
extern const Lingmo::Core::LogCategory lingmoRender;
extern const Lingmo::Core::LogCategory lingmoScene;
extern const Lingmo::Core::LogCategory lingmoAurora;
extern const Lingmo::Core::LogCategory lingmoInput;
// Opt-in button/capture transitions; no pointer-motion or keyboard text logging.
extern const Lingmo::Core::LogCategory lingmoInputTransitions;
extern const Lingmo::Core::LogCategory lingmoCursorTransitions;

#endif // LINGMO_CORE_LOGGING_H_
