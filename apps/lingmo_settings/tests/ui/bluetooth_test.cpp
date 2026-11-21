// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "base/event_loop.h"
#include "base/timer.h"
#include "bluetooth_fake.h"
#include "lingmo/bluez/manager.h"
#include "ui/application.h"
#include "ui/label.h"
#include "ui/window.h"
#include "widgets/listrow.h"
#include "window/page.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using namespace settings;
using namespace std::chrono_literals;

namespace {

bool waitFor(const std::function<bool()>& condition, std::chrono::milliseconds timeout = 5s)
{
    base::EventLoop& loop = Application::instance()->loop();
    if (condition())
        return true;
    base::Timer poll(&loop);
    poll.onTimeout = [&] {
        if (condition())
            loop.quit();
    };
    base::Timer limit(&loop);
    limit.setSingleShot(true);
    limit.onTimeout = [&loop] { loop.quit(); };
    poll.start(5ms);
    limit.start(timeout);
    loop.run();
    return condition();
}

View* findView(View* view, const std::string& name)
{
    if (!view)
        return nullptr;
    if (view->name() == name)
        return view;
    for (const auto& child : view->children()) {
        if (View* found = findView(child.get(), name))
            return found;
    }
    return nullptr;
}

bool isShown(View* view)
{
    for (; view; view = view->parent()) {
        if (!view->isVisible())
            return false;
    }
    return true;
}

// The texts of the labels under a view, in order.
std::string texts(View* view)
{
    std::string result;
    if (auto* label = dynamic_cast<Label*>(view); label && isShown(label))
        result += label->text() + "|";
    for (const auto& child : view->children())
        result += texts(child.get());
    return result;
}

// BlueZ on the test's session bus, shared with the page and the sidebar.
struct BluetoothPageTest : testing::Test
{
    void SetUp() override
    {
        server = std::make_unique<fake::FakeBluez>(Application::instance()->loop());
        manager = std::make_shared<bluez::Manager>(base::dbus::Connection::session());
        bluez::Manager::share(manager);
    }

    void TearDown() override
    {
        settingsWindow.reset();
        manager.reset();
        server.reset();
    }

    void openWindow()
    {
        settingsWindow = std::make_unique<SettingsWindow>();
        window().setScale(1.5);
        window().resize({900, 820});
        settingsWindow->reveal();
        window().grab();
    }

    Window& window() { return settingsWindow->window(); }
    View* view(const std::string& name) { return findView(settingsWindow->currentPage(), name); }
    bool inSidebar(const std::string& id)
    {
        window().grab();
        return window().layoutDump().find("\"sidebar:" + id + "\"") != std::string::npos;
    }

    std::unique_ptr<fake::FakeBluez> server;
    std::shared_ptr<bluez::Manager> manager;
    std::unique_ptr<SettingsWindow> settingsWindow;
};

} // namespace

TEST_F(BluetoothPageTest, SidebarFollowsTheAdapters)
{
    openWindow();
    waitFor([] { return false; }, 50ms);
    EXPECT_FALSE(manager->available());
    EXPECT_FALSE(inSidebar("bluetooth"));
    ASSERT_TRUE(server->registerService());
    ASSERT_TRUE(waitFor([this] { return manager->available(); }));
    ASSERT_TRUE(waitFor([this] { return inSidebar("bluetooth"); }));
    // Turned off, Bluetooth keeps its entry.
    manager->setEnabled(false);
    ASSERT_TRUE(waitFor([this] { return !manager->enabled(); }));
    EXPECT_TRUE(inSidebar("bluetooth"));
    settingsWindow->switchToPage("bluetooth");
    const fake::Interfaces adapterProperties = server->objects[{fake::adapter}];
    server->remove(fake::adapter, {fake::adapterInterface});
    ASSERT_TRUE(waitFor([this] { return !inSidebar("bluetooth"); }));
    window().grab();
    EXPECT_EQ(settingsWindow->currentPage()->title(), "Wi-Fi");
    server->add(fake::adapter, adapterProperties);
    ASSERT_TRUE(waitFor([this] { return inSidebar("bluetooth"); }));
    server->unregisterService();
    ASSERT_TRUE(waitFor([this] { return !inSidebar("bluetooth"); }));
}

TEST_F(BluetoothPageTest, DiscoversWhileThePageShows)
{
    ASSERT_TRUE(server->registerService());
    ASSERT_TRUE(waitFor([this] { return manager->available(); }));
    openWindow();
    settingsWindow->switchToPage("bluetooth");
    ASSERT_TRUE(waitFor([this] { return manager->scanning(); }));
    // Pairing prompts belong to the shell's agent, not to the page.
    EXPECT_FALSE(manager->agentEnabled());
    settingsWindow->switchToPage("about");
    ASSERT_TRUE(waitFor([this] { return !manager->scanning(); }));
    settingsWindow->switchToPage("bluetooth");
    ASSERT_TRUE(waitFor([this] { return manager->scanning(); }));
}

TEST_F(BluetoothPageTest, ListsAdaptersAndDevices)
{
    ASSERT_TRUE(server->registerService());
    ASSERT_TRUE(waitFor([this] { return manager->available(); }));
    openWindow();
    settingsWindow->switchToPage("bluetooth");
    ASSERT_TRUE(waitFor([this] { return manager->scanning(); }));
    window().grab();
    View* adapter = view("adapter:" + fake::adapter);
    ASSERT_TRUE(adapter);
    EXPECT_EQ(texts(adapter), "Test adapter|Not discoverable|");
    View* paired = view("bluetooth:paired");
    View* other = view("bluetooth:other");
    ASSERT_TRUE(paired && other);
    EXPECT_FALSE(isShown(paired));
    EXPECT_TRUE(isShown(other));
    ASSERT_TRUE(isShown(findView(other, fake::device)));
    EXPECT_EQ(texts(findView(other, fake::device)), "Headphones|");
    // While scanning, the title's indicator stands in for the placeholder.
    EXPECT_FALSE(isShown(findView(other, "placeholder")));

    server->set(fake::device, fake::deviceInterface, "Paired", true);
    ASSERT_TRUE(waitFor([&] { return isShown(paired) && isShown(findView(paired, fake::device)); }));
    EXPECT_EQ(texts(findView(paired, fake::device)), "Headphones|Not connected|");
    server->set(fake::device, fake::deviceInterface, "Connected", true);
    ASSERT_TRUE(waitFor([&] { return texts(findView(paired, fake::device)) == "Headphones|Connected|"; }));

    // The other devices' card goes, and the placeholder shows once scanning stops.
    server->set(fake::adapter, fake::adapterInterface, "Discovering", false);
    ASSERT_TRUE(waitFor([&] { return isShown(findView(other, "placeholder")); }));
    EXPECT_EQ(texts(findView(other, "placeholder")), "No devices found. Make sure your device is discoverable.|");

    // Turned off, only the switch and the reason stay.
    manager->setEnabled(false);
    ASSERT_TRUE(waitFor([&] { return !isShown(paired) && !isShown(adapter); }));
}

TEST_F(BluetoothPageTest, ExplainsFailures)
{
    server->failMethod = "StartDiscovery";
    ASSERT_TRUE(server->registerService());
    ASSERT_TRUE(waitFor([this] { return manager->available(); }));
    openWindow();
    settingsWindow->switchToPage("bluetooth");
    View* message = view("bluetooth:message");
    ASSERT_TRUE(message);
    ASSERT_TRUE(waitFor([&] { return isShown(message); }));
    EXPECT_EQ(texts(message), manager->errorText() + "|");
    EXPECT_EQ(manager->errorText().find("Injected failure"), std::string::npos);
}
