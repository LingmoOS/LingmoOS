// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>

#include "shell/controlcenter/control_center_services.h"

namespace Lingmo::Shell {

class NotificationSettings;
class Shell;
class StatusBar;

// The session's: NetworkManager and BlueZ on the system bus, PipeWire,
// org.lingmo.Privacy's switches, the Appearance service, and the bar's
// backend for MPRIS, brightness, battery and power profiles. `notifications`
// may be null (no Do Not Disturb).
std::unique_ptr<ControlCenterServices> createSessionServices(Shell *shell, StatusBar &statusBar,
                                                             NotificationSettings *notifications);

} // namespace Lingmo::Shell

