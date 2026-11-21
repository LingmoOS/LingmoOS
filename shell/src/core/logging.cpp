// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/logging.h"

namespace Lingmo::Core {

void setLoggingRules(std::string_view rules)
{
    base::logging::setRules(rules);
}

} // namespace Lingmo::Core

const Lingmo::Core::LogCategory lingmoWayland("lingmo.wayland");
const Lingmo::Core::LogCategory lingmoSession("lingmo.session");
const Lingmo::Core::LogCategory lingmoDrm("lingmo.drm");
const Lingmo::Core::LogCategory lingmoRender("lingmo.render");
const Lingmo::Core::LogCategory lingmoScene("lingmo.scene");
const Lingmo::Core::LogCategory lingmoAurora("lingmo.aurora");
const Lingmo::Core::LogCategory lingmoInput("lingmo.input");
const Lingmo::Core::LogCategory lingmoInputTransitions("lingmo.input.transitions");
const Lingmo::Core::LogCategory lingmoCursorTransitions("lingmo.cursor.transitions");
