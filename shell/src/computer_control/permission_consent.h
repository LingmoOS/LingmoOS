// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "base/dbus.h"
#include "computer_control/controller.h"
#include "computer_control/remembered_apps.h"

namespace Lingmo::ComputerControl {

// Asks the user through lingmo-services' permission prompt, kind
// "computer-control". Any session client can own a bus name first, so the
// owner's process has to be one that shows system dialogs, and the prompt goes
// to that owner's unique name. An application in `remembered` is allowed
// without asking, and the prompt offers to add the one asking now.
class PermissionConsent
{
public:
    // The session bus when no connection is given.
    explicit PermissionConsent(std::function<bool(int64_t pid)> showsSystemDialogs,
                               RememberedApps *remembered = nullptr,
                               base::dbus::Connection *bus = nullptr);
    PermissionConsent(const PermissionConsent &) = delete;
    PermissionConsent &operator=(const PermissionConsent &) = delete;
    ~PermissionConsent();

    // Environment::askConsent. Must outlive the questions it asks.
    // `appId` is the desktop entry of the agent's application, for the
    // prompt's icon; empty when it has none.
    std::function<void()> ask(const std::string &agentName, int64_t pid, const std::string &appId,
                              std::function<void(ConsentResult)> answer);

private:
    struct Request;
    void prompt(const std::shared_ptr<Request> &request, const std::string &owner);

    std::function<bool(int64_t)> m_showsSystemDialogs;
    RememberedApps *m_remembered;
    base::dbus::Connection *m_bus;
};

} // namespace Lingmo::ComputerControl

