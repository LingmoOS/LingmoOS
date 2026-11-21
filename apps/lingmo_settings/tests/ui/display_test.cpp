// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/timer.h"
#include "lingmo/display/display_types.h"
#include "ui/application.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/test_input.h"
#include "ui/window.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "window/page.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::base;
using namespace Lingmo::Display;
using namespace Lingmo::ui;
using namespace settings;
using namespace std::chrono_literals;

namespace {

bool waitFor(const std::function<bool()>& condition, std::chrono::milliseconds timeout = 5s)
{
    EventLoop& loop = Application::instance()->loop();
    if (condition())
        return true;
    Timer poll(&loop);
    poll.onTimeout = [&] {
        if (condition())
            loop.quit();
    };
    Timer limit(&loop);
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
    if (!view)
        return false;
    for (; view; view = view->parent()) {
        if (!view->isVisible())
            return false;
    }
    return true;
}

std::string texts(View* view)
{
    std::string result;
    if (auto* label = dynamic_cast<Label*>(view); label && isShown(label))
        result += label->text() + "|";
    for (const auto& child : view->children())
        result += texts(child.get());
    return result;
}

// A DRM card node that exists, for the brightness row's connector name.
std::string cardDevice()
{
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator("/dev/dri", error)) {
        if (entry.path().filename().string().starts_with("card"))
            return entry.path().string();
    }
    return {};
}

using Request = std::tuple<std::string, bool, RequestedMode, uint32_t, uint32_t>;

// The compositor's display service, the night light and brightness of
// lingmo-services, on a thread of their own: the page reads brightness
// with blocking calls.
class FakeServices
{
public:
    FakeServices()
    {
        std::promise<void> ready;
        m_thread = std::thread([this, &ready] { run(ready); });
        ready.get_future().get();
    }

    ~FakeServices()
    {
        m_handle.post([this] { m_loop->quit(); });
        m_thread.join();
    }

    // False when another connection owns a name: the page would talk to that
    // service instead.
    bool owned() const { return m_owned; }
    std::vector<std::vector<Request>> requests() const { return locked(m_requests); }
    std::vector<bool> previews() const { return locked(m_previews); }
    std::vector<std::string> confirmed() const { return locked(m_confirmed); }
    int32_t schedule() const { return locked(m_schedule); }
    std::vector<std::string> brightnessDisplays() const { return locked(m_brightnessDisplays); }

private:
    template<typename T>
    T locked(const T& value) const
    {
        std::lock_guard lock(m_mutex);
        return value;
    }

    void run(std::promise<void>& ready)
    {
        EventLoop loop;
        m_loop = &loop;
        m_handle = loop.handle();
        std::unique_ptr<dbus::Connection> bus = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        std::unique_ptr<dbus::Connection> services = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);

        PhysicalDisplay panel;
        panel.sessionId = "eDP-1";
        panel.connectorName = "eDP-1";
        panel.devicePath = cardDevice();
        panel.connectionType = 1;
        panel.capabilities = ColorTransformCapability;
        panel.recommendedScale120 = 180;
        for (const auto& [width, height] : {std::pair{2160, 1440}, std::pair{1920, 1280}, std::pair{1080, 720}}) {
            Mode mode;
            mode.width = width;
            mode.height = height;
            mode.refreshMilliHz = 60000;
            mode.preferred = width == 2160;
            mode.current = width == 2160;
            panel.modes.push_back(mode);
        }
        std::vector<PhysicalDisplay> physical = {panel};
        LogicalDisplay logical;
        logical.id = "eDP-1";
        logical.destinations = {"eDP-1"};
        logical.width = 1440;
        logical.height = 960;
        logical.scale120 = 180;
        logical.primary = true;
        std::vector<LogicalDisplay> logicals = {logical};
        uint64_t revision = 1;
        std::string previewToken;

        dbus::Object display(bus.get(), "/org/lingmo/DisplayConfig1", "org.lingmo.DisplayConfig1");
        display.addMethod("GetState", [&] {
            return std::make_tuple(std::string("test"), std::string("seat0"), uint64_t(1), revision, physical,
                                   logicals, std::vector<Placement>(), previewToken,
                                   uint64_t(previewToken.empty() ? 0 : 1));
        });
        const auto configure = [&](bool preview) {
            return [&, preview](const std::string&, uint64_t, uint64_t, const std::vector<Request>& displays,
                                const std::string&, const std::vector<Placement>&, const MirrorGroups&) {
                std::string token;
                {
                    std::lock_guard lock(m_mutex);
                    m_requests.push_back(displays);
                    m_previews.push_back(preview);
                    token = "token-" + std::to_string(m_requests.size());
                }
                if (preview)
                    previewToken = token;
                ++revision;
                for (const auto& [id, enabled, mode, scale120, rotation] : displays) {
                    for (Mode& candidate : physical.front().modes)
                        candidate.current = candidate.width == mode.width && candidate.height == mode.height;
                    logicals.front().scale120 = scale120;
                    logicals.front().rotation = rotation;
                }
                display.emitSignal("OperationFinished", token, true, false, uint32_t(0), std::string(),
                                   std::vector<std::string>(), uint64_t(1), revision);
                display.emitSignal("StateChanged", uint64_t(1), revision);
                return token;
            };
        };
        display.addMethod("SetConfiguration", configure(false));
        display.addMethod("ApplyConfiguration", configure(true));
        display.addMethod("ConfirmConfiguration", [&](const std::string& token) {
            {
                std::lock_guard lock(m_mutex);
                m_confirmed.push_back(token);
            }
            previewToken.clear();
            display.emitSignal("StateChanged", uint64_t(1), revision);
        });
        display.addMethod("RevertConfiguration", [&](const std::string&) {
            previewToken.clear();
            display.emitSignal("StateChanged", uint64_t(1), revision);
        });
        display.addSignal("StateChanged", "tt");
        display.addSignal("OperationFinished", "sbbusastt");
        display.addSignal("PreviewChanged", "sttt");
        display.publish();
        m_owned = bus->requestName("org.lingmo.DisplayConfig");

        bool enabled = false;
        dbus::Object nightLight(services.get(), "/org/lingmo/Services/NightLight", "org.lingmo.Services.NightLight");
        nightLight.addProperty("enabled", [&] { return enabled; });
        nightLight.addProperty("temperature", [] { return 0.5; });
        nightLight.addProperty("schedule", [this] { return locked(m_schedule); });
        nightLight.addProperty("customStart", [] { return int32_t(18 * 60); });
        nightLight.addProperty("customEnd", [] { return int32_t(6 * 60); });
        nightLight.addProperty("available", [] { return true; });
        nightLight.addProperty("sunset", [] { return int32_t(18 * 60 + 30); });
        nightLight.addProperty("sunrise", [] { return int32_t(6 * 60 + 15); });
        nightLight.addMethod("setEnabled", [&](bool value) {
            enabled = value;
            nightLight.emitSignal("enabledChanged", value);
        });
        nightLight.addMethod("setSchedule", [&](int32_t value) {
            {
                std::lock_guard lock(m_mutex);
                m_schedule = value;
            }
            nightLight.emitSignal("scheduleChanged", value);
        });
        for (const char* name : {"enabledChanged", "availableChanged"})
            nightLight.addSignal(name, "b");
        for (const char* name :
             {"scheduleChanged", "customStartChanged", "customEndChanged", "sunsetChanged", "sunriseChanged"})
            nightLight.addSignal(name, "i");
        nightLight.addSignal("temperatureChanged", "d");
        nightLight.publish();

        if (!panel.devicePath.empty())
            m_brightnessDisplays = {std::filesystem::path(panel.devicePath).filename().string() + "-eDP-1"};
        dbus::Object brightness(services.get(), "/org/lingmo/Services/Brightness", "org.lingmo.Services.Brightness");
        brightness.addMethod("displays", [this] { return locked(m_brightnessDisplays); });
        brightness.addMethod("displayBrightness", [](const std::string&) { return int32_t(70); });
        brightness.addMethod("setDisplayBrightness", [](const std::string&, int32_t) {});
        brightness.addSignal("displaysChanged", "");
        brightness.addSignal("displayBrightnessChanged", "si");
        brightness.publish();
        m_owned = services->requestName("org.lingmo.Services") && m_owned;

        ready.set_value();
        loop.run();
    }

    std::thread m_thread;
    EventLoop* m_loop = nullptr;
    LoopHandle m_handle;
    bool m_owned = false;
    mutable std::mutex m_mutex;
    std::vector<std::vector<Request>> m_requests;
    std::vector<bool> m_previews;
    std::vector<std::string> m_confirmed;
    int32_t m_schedule = 0;
    std::vector<std::string> m_brightnessDisplays;
};

struct DisplayPageTest : testing::Test
{
    void SetUp() override
    {
        services = std::make_unique<FakeServices>();
        ASSERT_TRUE(services->owned());
        settingsWindow = std::make_unique<SettingsWindow>();
        settingsWindow->window().setScale(1.5);
        settingsWindow->window().resize({900, 820});
        settingsWindow->reveal();
        settingsWindow->switchToPage("display");
        settingsWindow->window().grab();
        ASSERT_TRUE(waitFor([this] { return isShown(view("display:name")); }));
    }

    void TearDown() override
    {
        settingsWindow.reset();
        services.reset();
    }

    View* view(const std::string& name) { return findView(settingsWindow->currentPage(), name); }
    RowComboBox* combo(const std::string& name) { return dynamic_cast<RowComboBox*>(view(name)); }

    std::unique_ptr<FakeServices> services;
    std::unique_ptr<SettingsWindow> settingsWindow;
};

} // namespace

TEST_F(DisplayPageTest, NamesTheOnlyScreen)
{
    EXPECT_EQ(texts(view("display:name")), "Built-in Display|");
    EXPECT_FALSE(isShown(view("display:tabs")));
    EXPECT_FALSE(isShown(view("display:arrangement")));
    EXPECT_FALSE(isShown(view("display:error")));
    EXPECT_EQ(combo("display:resolution")->currentText(), "2160 × 1440 (Recommended)");
    EXPECT_EQ(combo("display:resolution")->count(), 3);
    EXPECT_EQ(combo("display:refresh")->currentText(), "60.00 Hz");
    EXPECT_EQ(combo("display:scale")->currentText(), "150% (Recommended)");
    EXPECT_EQ(combo("display:rotation")->currentText(), "Normal");
    EXPECT_EQ(isShown(view("display:brightness")), !services->brightnessDisplays().empty());
}

TEST_F(DisplayPageTest, ScaleAppliesAtOnce)
{
    RowComboBox* scale = combo("display:scale");
    ASSERT_TRUE(scale && scale->onActivated);
    // 125%.
    scale->onActivated(1);
    ASSERT_TRUE(waitFor([this] { return services->requests().size() == 1; }));
    EXPECT_FALSE(services->previews().front());
    EXPECT_EQ(std::get<3>(services->requests().front().front()), 150u);
    ASSERT_TRUE(waitFor([this] { return combo("display:scale")->currentText() == "125%"; }));
}

TEST_F(DisplayPageTest, ResolutionWaitsForConfirmation)
{
    const size_t windows = Application::instance()->windows().size();
    RowComboBox* resolution = combo("display:resolution");
    ASSERT_TRUE(resolution && resolution->onActivated);
    resolution->onActivated(1);
    ASSERT_TRUE(waitFor([this] { return services->requests().size() == 1; }));
    EXPECT_TRUE(services->previews().front());
    EXPECT_EQ(std::get<2>(services->requests().front().front()).width, 1920);
    // The confirmation opens over the window; Return keeps the mode.
    ASSERT_TRUE(waitFor([windows] { return Application::instance()->windows().size() == windows + 1; }));
    Window* dialog = Application::instance()->windows().back();
    TestInput::setKeyboardFocus(*dialog, true);
    TestInput::typeKey(*dialog, key::Return);
    ASSERT_TRUE(waitFor([this] { return services->confirmed().size() == 1; }));
    EXPECT_EQ(services->confirmed().front(), "token-1");
    ASSERT_TRUE(waitFor([windows] { return Application::instance()->windows().size() == windows; }));
    EXPECT_EQ(combo("display:resolution")->currentText(), "1920 × 1280");
}

TEST_F(DisplayPageTest, NightLightScheduleRows)
{
    ASSERT_TRUE(waitFor([this] { return isShown(view("display:nightlight")); }));
    EXPECT_FALSE(isShown(view("display:sunset")));
    EXPECT_FALSE(isShown(view("display:from")));
    RowComboBox* schedule = combo("display:schedule");
    ASSERT_TRUE(schedule && schedule->onActivated);
    schedule->onActivated(1);
    ASSERT_TRUE(waitFor([this] { return services->schedule() == 1 && isShown(view("display:sunset")); }));
    // Sunset 18:30 in the session's short time format.
    EXPECT_NE(texts(view("display:sunset")).find("30"), std::string::npos);
    schedule->onActivated(2);
    ASSERT_TRUE(waitFor([this] { return isShown(view("display:from")) && !isShown(view("display:sunset")); }));
}
