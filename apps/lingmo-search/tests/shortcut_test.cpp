// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "accelerators/accelerator_table.h"
#include "accelerators/accelerator_delegate.h"
#include "base/dbus.h"
#include "base/event_loop.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <poll.h>
using namespace Lingmo::Accelerators;
void check(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
int main() {
    for (const auto& table : {std::vector<AcceleratorData>(kAcceleratorData.begin(), kAcceleratorData.end()),
                              std::vector<AcceleratorData>(kCommandAcceleratorData.begin(), kCommandAcceleratorData.end())}) {
        size_t found = 0;
        for (const auto& item : table) if (item.keysym == XKB_KEY_space && item.modifiers == MetaDown && item.triggerOnPress) {
            check(item.action == AcceleratorAction::ToggleSearch, "Meta+Space collision"); ++found;
        }
        check(found == 1, "search default binding");
    }
    check(std::find(kActionsAllowedAtLockScreen.begin(), kActionsAllowedAtLockScreen.end(), AcceleratorAction::ToggleSearch) == kActionsAllowedAtLockScreen.end(), "search forbidden while locked");
    check(acceleratorActionFromName("toggle-search") == AcceleratorAction::ToggleSearch, "stable action name");
    Lingmo::base::EventLoop loop;
    auto* bus = Lingmo::base::dbus::Connection::session(); check(bus, "test bus");
    std::string token;
    Lingmo::base::dbus::Object object(bus, "/org/lingmo/Search", "org.lingmo.Search");
    object.addMethod("toggle", [&](const std::string& value) { token = value; });
    check(object.publish() && bus->requestName("org.lingmo.Search"), "search endpoint");
    DBusAcceleratorDelegate delegate; delegate.searchActivationToken = [] { return std::string("test-shell-token"); };
    delegate.toggleSearch();
    auto end = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (token.empty() && std::chrono::steady_clock::now() < end) { pollfd fd{loop.fd(), POLLIN, 0}; ::poll(&fd, 1, 5); loop.dispatch(); }
    check(token == "test-shell-token", "shell activation token forwarded");
    std::cout << "Search global shortcut contract passed\n";
}
