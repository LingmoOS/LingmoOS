// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "base/event_loop.h"
#include "base/paths.h"
#include "base/timer.h"
#include "lingmo/audio/backend.h"
#include "sound_teststream.h"
#include "ui/application.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/segmented_control.h"
#include "ui/window.h"
#include "widgets/listrow.h"
#include "widgets/sliderrow.h"
#include "window/page.h"
#include "window/settingswindow.h"

extern char** environ;

using namespace Lingmo;
using namespace Lingmo::Audio;
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
    if (!view)
        return false;
    for (; view; view = view->parent()) {
        if (!view->isVisible())
            return false;
    }
    return true;
}

// Whether a shown label under the view reads the text.
bool showsText(View* view, const std::string& text)
{
    if (auto* label = dynamic_cast<Label*>(view); label && isShown(label) && label->text() == text)
        return true;
    for (const auto& child : view->children()) {
        if (showsText(child.get(), text))
            return true;
    }
    return false;
}

// A program run beside the test, its output into a log file.
class Process
{
public:
    ~Process() { stop(); }

    bool start(const std::string& program, const std::vector<std::string>& arguments, const std::string& log)
    {
        posix_spawn_file_actions_t actions;
        posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, log.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO);
        std::vector<char*> argv{const_cast<char*>(program.c_str())};
        for (const std::string& argument : arguments)
            argv.push_back(const_cast<char*>(argument.c_str()));
        argv.push_back(nullptr);
        const int result = posix_spawn(&m_pid, program.c_str(), &actions, nullptr, argv.data(), environ);
        posix_spawn_file_actions_destroy(&actions);
        if (result != 0)
            m_pid = 0;
        return m_pid != 0;
    }

    void stop()
    {
        if (!m_pid)
            return;
        ::kill(m_pid, SIGTERM);
        for (int i = 0; i < 300; ++i) {
            if (::waitpid(m_pid, nullptr, WNOHANG) == m_pid) {
                m_pid = 0;
                return;
            }
            ::usleep(10000);
        }
        ::kill(m_pid, SIGKILL);
        ::waitpid(m_pid, nullptr, 0);
        m_pid = 0;
    }

private:
    pid_t m_pid = 0;
};

// The variables the private PipeWire is found by; put back afterwards.
const char* const kVariables[] = {"XDG_RUNTIME_DIR", "PIPEWIRE_RUNTIME_DIR", "PIPEWIRE_REMOTE", "XDG_CONFIG_HOME",
                                  "XDG_STATE_HOME", "XDG_CACHE_HOME"};

} // namespace

// The Sound page against a private PipeWire and WirePlumber with two sinks and a source.
class SoundPageTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        for (const char* name : kVariables) {
            if (const char* value = std::getenv(name))
                saved[name] = value;
        }
        char directory[] = "/tmp/lingmo-sound-test-XXXXXX";
        runtime = ::mkdtemp(directory);
        ::setenv("XDG_RUNTIME_DIR", runtime.c_str(), 1);
        ::setenv("PIPEWIRE_RUNTIME_DIR", runtime.c_str(), 1);
        ::setenv("PIPEWIRE_REMOTE", "lingmo-audio-test", 1);
        ::setenv("XDG_CONFIG_HOME", (runtime + "/config").c_str(), 1);
        ::setenv("XDG_STATE_HOME", (runtime + "/state").c_str(), 1);
        ::setenv("XDG_CACHE_HOME", (runtime + "/cache").c_str(), 1);
        backend = Backend::shared();
        startServers();
        ASSERT_TRUE(waitFor([] { return backend->isConnected(); }, 10s));
        activateDevices();
    }

    static void TearDownTestSuite()
    {
        drivers.clear();
        backend.reset();
        stopServers();
        for (const char* name : kVariables) {
            const auto value = saved.find(name);
            if (value == saved.end())
                ::unsetenv(name);
            else
                ::setenv(name, value->second.c_str(), 1);
        }
        std::error_code error;
        std::filesystem::remove_all(runtime, error);
    }

    static void startServers()
    {
        const std::string log = runtime + "/servers.log";
        ASSERT_TRUE(server.start(base::paths::findExecutable("pipewire"),
                                 {"-c", SETTINGS_SOURCE_DIR "/../tests/ui/sound_pipewire.conf"}, log));
        ASSERT_TRUE(waitFor([] { return std::filesystem::exists(runtime + "/lingmo-audio-test"); }));
        ASSERT_TRUE(policy.start(base::paths::findExecutable("wireplumber"), {"--profile=policy"}, log));
    }

    static void stopServers()
    {
        drivers.clear();
        policy.stop();
        server.stop();
    }

    static void activateDevices()
    {
        for (const char* name : {"test-output-a", "test-output-b", "test-input"}) {
            ASSERT_TRUE(waitFor([name] { return find(name) != nullptr; }, 10s));
            const auto node = find(name);
            auto stream = std::make_unique<TestStream>(backend->core(), std::string("lingmo.internal.test-") + name,
                                                       node->property(PW_KEY_OBJECT_SERIAL),
                                                       node->kind() == AudioObject::Source);
            ASSERT_TRUE(stream->connected);
            TestStream* added = stream.get();
            ASSERT_TRUE(waitFor([added] { return added->streaming; }));
            drivers.push_back(std::move(stream));
        }
        ASSERT_TRUE(waitFor([] { return backend->defaultNode(AudioObject::Sink) != nullptr; }, 10s));
        // WirePlumber leaves the virtual test source alone as the default.
        find("test-input")->setDefault(true);
        ASSERT_TRUE(waitFor([] { return backend->defaultNode(AudioObject::Source) == find("test-input"); }));
    }

    static std::shared_ptr<AudioObject> find(const std::string& name)
    {
        for (const auto& object : backend->objects()) {
            if (object->name() == name)
                return object;
        }
        return nullptr;
    }

    void openPage()
    {
        settingsWindow = std::make_unique<SettingsWindow>();
        settingsWindow->window().setScale(1.5);
        settingsWindow->window().resize({900, 820});
        settingsWindow->reveal();
        settingsWindow->switchToPage("sound");
        settingsWindow->window().grab();
    }

    View* view(const std::string& name) { return findView(settingsWindow->currentPage(), name); }
    View* child(const std::string& prefix, const std::shared_ptr<AudioObject>& device)
    {
        View* found = view(prefix + std::to_string(device->index()));
        return isShown(found) ? found : nullptr;
    }

    void TearDown() override { settingsWindow.reset(); }

    static inline std::string runtime;
    static inline std::map<std::string, std::string> saved;
    static inline std::shared_ptr<Backend> backend;
    static inline Process server;
    static inline Process policy;
    static inline std::vector<std::unique_ptr<TestStream>> drivers;
    std::unique_ptr<SettingsWindow> settingsWindow;
};

TEST_F(SoundPageTest, MultipleDevices)
{
    auto playback = std::make_unique<TestStream>(backend->core(), "settings-playback",
                                                 find("test-output-a")->property(PW_KEY_OBJECT_SERIAL));
    ASSERT_TRUE(waitFor([&] { return playback->streaming; }));
    ASSERT_TRUE(waitFor([] { return find("settings-playback") != nullptr; }));
    openPage();
    const auto first = find("test-output-a");
    const auto second = find("test-output-b");
    const auto source = find("test-input");
    ASSERT_TRUE(waitFor([&] { return child("device-", first) && child("device-", second); }));
    EXPECT_FALSE(child("device-", source));
    settingsWindow->window().grab();
    EXPECT_GT(child("device-", first)->geometry().width, 600);

    auto* secondRow = dynamic_cast<ListRow*>(child("device-", second));
    ASSERT_TRUE(secondRow && secondRow->onClicked);
    secondRow->onClicked();
    ASSERT_TRUE(waitFor([&] { return backend->defaultNode(AudioObject::Sink) == second; }));
    ASSERT_TRUE(waitFor([&] { return child("volume-", second) != nullptr; }));
    EXPECT_FALSE(child("volume-", first));

    const double originalVolume = first->volume();
    auto* slider = dynamic_cast<SliderRow*>(child("volume-", second));
    ASSERT_TRUE(slider && slider->onMoved);
    slider->onMoved(0.57);
    ASSERT_TRUE(waitFor([&] { return std::abs(second->volume() - 0.57) < 0.0001; }));
    EXPECT_LT(std::abs(first->volume() - originalVolume), 0.0001);
    EXPECT_FALSE(view("mute"));
    // With no mute switch, muting shows as zero.
    second->setMuted(true);
    ASSERT_TRUE(waitFor([&] { return slider->value() == 0.0; }));
    slider->onMoved(0.4);
    ASSERT_TRUE(waitFor([&] { return !second->muted() && std::abs(second->volume() - 0.4) < 0.0001; }));
    slider->onMoved(0.0);
    ASSERT_TRUE(waitFor([&] { return second->muted(); }));
    EXPECT_FALSE(first->muted());
    EXPECT_LT(std::abs(first->volume() - originalVolume), 0.0001);

    auto* tabs = dynamic_cast<SegmentedControl*>(view("tabs"));
    ASSERT_TRUE(tabs && tabs->onCurrentChanged);
    tabs->setCurrentIndex(1);
    tabs->onCurrentChanged(1);
    ASSERT_TRUE(waitFor([&] { return child("device-", source) != nullptr; }));
    EXPECT_FALSE(child("device-", first));
    ASSERT_TRUE(waitFor([&] { return child("volume-", source) != nullptr; }));
    const auto stream = find("settings-playback");
    ASSERT_TRUE(stream);
    EXPECT_FALSE(view("stream-" + std::to_string(stream->index())));
    EXPECT_FALSE(showsText(settingsWindow->currentPage(), "Applications"));
    EXPECT_FALSE(showsText(settingsWindow->currentPage(), "Test sound"));

    // The daemon going and coming back takes the rows with it.
    tabs->setCurrentIndex(0);
    tabs->onCurrentChanged(0);
    const std::string firstRow = "device-" + std::to_string(first->index());
    playback.reset();
    ASSERT_TRUE(waitFor([] { return find("settings-playback") == nullptr; }));
    stopServers();
    ASSERT_TRUE(waitFor([] { return !backend->isConnected(); }));
    ASSERT_TRUE(waitFor([&] { return !isShown(view(firstRow)); }));
    EXPECT_TRUE(showsText(settingsWindow->currentPage(), "Connecting to the audio service…"));
    startServers();
    ASSERT_TRUE(waitFor([] { return find("test-output-a") && find("test-output-b"); }, 10s));
    activateDevices();
    ASSERT_TRUE(waitFor([&] { return child("device-", find("test-output-a")) != nullptr; }, 10s));
}
