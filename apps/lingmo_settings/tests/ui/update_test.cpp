// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include <gtest/gtest.h>
#include "base/dbus.h"
#include "base/event_loop.h"
#include "ui/application.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/window.h"
#include "widgets/listrow.h"
#include "window/page.h"
#include "window/settingswindow.h"
#include <chrono>
#include <poll.h>
#include <string>
using namespace Lingmo;
namespace {
ui::View* find(ui::View* root, const std::string& name) {
    if (root->name() == name) return root;
    for (const auto& child : root->children()) if (auto* view = find(child.get(), name)) return view;
    return nullptr;
}
bool wait(const std::function<bool()>& ready) {
    auto& loop = ui::Application::instance()->loop();
    auto end = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!ready() && std::chrono::steady_clock::now() < end) { pollfd fd{loop.fd(), POLLIN, 0}; ::poll(&fd, 1, 5); loop.dispatch(); }
    return ready();
}
std::string buttonText(ui::Button* button) {
    for (const auto& child : button->children())
        if (auto* label = dynamic_cast<ui::Label*>(child.get()))
            return label->text();
    return {};
}
}
TEST(UpdatePage, FailedRequestCanRetry) {
    settings::SettingsWindow window;
    window.switchToPage("update"); window.reveal(); window.window().grab();
    auto* action = dynamic_cast<ui::Button*>(find(window.currentPage(), "update:action"));
    auto* status = dynamic_cast<ui::Label*>(find(window.currentPage(), "update:status"));
    ASSERT_TRUE(action); ASSERT_TRUE(status);
    EXPECT_FALSE(action->isEnabled());
    auto* bus = base::dbus::Connection::session(); ASSERT_TRUE(bus);
    int32_t state = 6; int calls = 0;
    base::dbus::Object object(bus, "/org/lingmo/Services/Update", "org.lingmo.Services.Update");
    object.addProperty("state", [&] { return state; });
    object.addProperty("channel", [] { return int32_t(0); });
    object.addProperty("errorLog", [] { return std::string("Previous failure"); });
    object.addSignal("stateChanged", "i");
    object.addMethod("checkForUpdate", "", "", [&](base::dbus::MethodCall& call) {
        ++calls;
        if (calls == 1) call.replyError("org.lingmo.TestFailure", "Simulated service failure");
        else { state = 1; object.emitSignal("stateChanged", state); call.reply(); }
    });
    ASSERT_TRUE(object.publish()); ASSERT_TRUE(bus->requestName("org.lingmo.Services"));
    ASSERT_TRUE(wait([&] { return action->isEnabled(); }));
    action->onClicked(); EXPECT_FALSE(action->isEnabled());
    ASSERT_TRUE(wait([&] { return calls == 1 && action->isEnabled(); }));
    EXPECT_EQ(status->text(), "Update failed!");
    action->onClicked();
    ASSERT_TRUE(wait([&] { return calls == 2 && status->text() == "Checking for updates"; }));
    EXPECT_FALSE(action->isEnabled());
    bus->releaseName("org.lingmo.Services");
    ASSERT_TRUE(wait([&] { return status->text() == "Update service unavailable"; }));
    EXPECT_FALSE(action->isEnabled());
}

TEST(UpdatePage, AdvancedSettingsOpens) {
    settings::SettingsWindow window;
    window.switchToPage("update"); window.reveal(); window.window().grab();
    auto* advanced = dynamic_cast<settings::NavigationItem*>(find(window.currentPage(), "update:advanced"));
    ASSERT_TRUE(advanced);
    advanced->onClicked();
    ASSERT_NE(nullptr, window.currentPage());
    EXPECT_EQ(window.currentPage()->title(), "Advanced Settings");
}

TEST(UpdatePage, InstalledUpdatePromptsForRestart) {
    settings::SettingsWindow window;
    window.switchToPage("update"); window.reveal(); window.window().grab();
    auto* action = dynamic_cast<ui::Button*>(find(window.currentPage(), "update:action"));
    auto* status = dynamic_cast<ui::Label*>(find(window.currentPage(), "update:status"));
    ASSERT_TRUE(action); ASSERT_TRUE(status);

    auto* bus = base::dbus::Connection::session(); ASSERT_TRUE(bus);
    base::dbus::Object update(bus, "/org/lingmo/Services/Update", "org.lingmo.Services.Update");
    update.addProperty("state", [] { return int32_t(7); });
    update.addProperty("channel", [] { return int32_t(0); });
    update.addProperty("newVersion", [] { return std::string("5.0"); });
    update.addProperty("upgradableCount", [] { return int32_t(0); });
    update.addProperty("changelogText", [] { return std::string(); });
    update.addProperty("errorLog", [] { return std::string(); });
    update.addSignal("stateChanged", "i");
    ASSERT_TRUE(update.publish());
    ASSERT_TRUE(bus->requestName("org.lingmo.Services"));

    int reboots = 0;
    base::dbus::Object session(bus, "/org/lingmo/Session", "org.lingmo.Session");
    session.addMethod("reboot", [&] { ++reboots; });
    ASSERT_TRUE(session.publish());
    ASSERT_TRUE(bus->requestName("org.lingmo.Session"));

    ASSERT_TRUE(wait([&] { return status->text() == "Restart required"; }));
    EXPECT_EQ(buttonText(action), "Restart now");
    ASSERT_TRUE(action->isEnabled());
    action->onClicked();
    ASSERT_TRUE(wait([&] { return reboots == 1; }));

    bus->releaseName("org.lingmo.Session");
    bus->releaseName("org.lingmo.Services");
}
