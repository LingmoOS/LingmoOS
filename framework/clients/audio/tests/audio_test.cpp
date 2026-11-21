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
#include <limits>
#include <memory>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include <spa/param/audio/raw.h>
#include <spa/param/profile.h>
#include <spa/param/props.h>
#include <spa/param/route.h>
#include <spa/pod/builder.h>

#include "base/event_loop.h"
#include "base/timer.h"
#include "lingmo/audio/audio_model.h"
#include "lingmo/audio/backend.h"
#include "lingmo/audio/volume_monitor.h"
#include "fake_audio_device.h"
#include "test_stream.h"

extern char** environ;

using namespace Lingmo::Audio;
using namespace Lingmo::Base;
using namespace std::chrono_literals;

// Runs the loop until the condition holds or the time is up.
static bool waitFor(EventLoop& loop, const std::function<bool()>& condition, std::chrono::milliseconds timeout = 5s)
{
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

static void wait(EventLoop& loop, std::chrono::milliseconds time)
{
    waitFor(loop, [] { return false; }, time);
}

static bool near(double a, double b)
{
    return std::abs(a - b) < 0.0001;
}

namespace {

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

    static int run(const std::string& program, const std::vector<std::string>& arguments)
    {
        Process process;
        if (!process.start(program, arguments, "/dev/null"))
            return -1;
        int status = 0;
        ::waitpid(process.m_pid, &status, 0);
        process.m_pid = 0;
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }

private:
    pid_t m_pid = 0;
};

} // namespace

// A private PipeWire and WirePlumber with two sinks and a source, shared by
// the tests in their order, as the daemon's state carries from one to the next.
class AudioTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        char directory[] = "/tmp/lingmo-audio-test-XXXXXX";
        runtime = ::mkdtemp(directory);
        // The PipeWire libraries and daemons find each other through these.
        ::setenv("XDG_RUNTIME_DIR", runtime.c_str(), 1);
        ::setenv("PIPEWIRE_RUNTIME_DIR", runtime.c_str(), 1);
        ::setenv("PIPEWIRE_REMOTE", "lingmo-audio-test", 1);
        ::setenv("XDG_CONFIG_HOME", (runtime + "/config").c_str(), 1);
        ::setenv("XDG_STATE_HOME", (runtime + "/state").c_str(), 1);
        ::setenv("XDG_CACHE_HOME", (runtime + "/cache").c_str(), 1);
        ::unsetenv("PIPEWIRE_CONFIG_NAME");
        ::unsetenv("PIPEWIRE_CONFIG_DIR");
        ::unsetenv("WIREPLUMBER_CONFIG_DIR");
        const std::filesystem::path fragments = runtime + "/config/wireplumber/wireplumber.conf.d";
        std::filesystem::create_directories(fragments);
        std::filesystem::copy_file(WIREPLUMBER_TEST_CONFIG, fragments / "50-lingmo-test.conf");
        loop = new EventLoop;
        backend = Backend::shared();
        wait(*loop, 50ms);
        ASSERT_FALSE(backend->isConnected());
        startServers();
        ASSERT_TRUE(waitFor(*loop, [] { return backend->isConnected(); }, 10s));
        ASSERT_TRUE(waitFor(*loop, [] { return find("test-output-a") && find("test-output-b") && find("test-input"); },
                            10s));
        ASSERT_TRUE(waitFor(*loop, [] { return backend->defaultNode(AudioObject::Sink) != nullptr; }, 10s));
        activateDevices();
    }

    static void TearDownTestSuite()
    {
        drivers.clear();
        backend.reset();
        stopServers();
        delete loop;
        loop = nullptr;
        std::error_code error;
        std::filesystem::remove_all(runtime, error);
    }

    static void startServers()
    {
        const std::string log = runtime + "/servers.log";
        ASSERT_TRUE(server.start(PIPEWIRE_EXECUTABLE, {"-c", AUDIO_TEST_CONFIG}, log));
        ASSERT_TRUE(waitFor(*loop, [] { return std::filesystem::exists(runtime + "/lingmo-audio-test"); }));
        ASSERT_TRUE(policy.start(WIREPLUMBER_EXECUTABLE, {"--profile=policy"}, log));
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
            ASSERT_TRUE(waitFor(*loop, [name] { return find(name) != nullptr; }, 10s));
            const auto node = find(name);
            auto stream = std::make_unique<TestStream>(backend->core(), std::string("lingmo.internal.test-") + name,
                                                       node->property(PW_KEY_OBJECT_SERIAL),
                                                       node->kind() == AudioObject::Source);
            ASSERT_TRUE(stream->connected);
            TestStream* added = stream.get();
            ASSERT_TRUE(waitFor(*loop, [added] { return added->streaming; }));
            drivers.push_back(std::move(stream));
        }
    }

    static std::shared_ptr<AudioObject> find(const std::string& name)
    {
        for (const auto& object : backend->objects()) {
            if (object->name() == name)
                return object;
        }
        return nullptr;
    }

    static bool until(const std::function<bool()>& condition) { return waitFor(*loop, condition); }

    static inline std::string runtime;
    static inline EventLoop* loop = nullptr;
    static inline std::shared_ptr<Backend> backend;
    static inline Process server;
    static inline Process policy;
    static inline std::vector<std::unique_ptr<TestStream>> drivers;
};

TEST_F(AudioTest, VolumeScale)
{
    EXPECT_EQ(AudioObject::fromLinear(0.125f), 0.5);
    EXPECT_EQ(AudioObject::toLinear(0.5), 0.125f);
    EXPECT_EQ(AudioObject::toLinear(-1.0), 0.0f);
    EXPECT_EQ(AudioObject::toLinear(100.0), 3.375f);
    EXPECT_EQ(AudioObject::toLinear(std::numeric_limits<double>::infinity()), 0.0f);
    EXPECT_EQ(AudioObject::fromLinear(std::numeric_limits<float>::quiet_NaN()), 0.0);
    for (double value : {0.0, 0.01, 0.25, 0.75, 1.0, 1.5})
        EXPECT_LT(std::abs(AudioObject::fromLinear(AudioObject::toLinear(value)) - value), 0.000001);
}

TEST_F(AudioTest, ParseProperties)
{
    AudioObject object(nullptr, 42, AudioObject::Sink);
    EXPECT_FALSE(object.hasVolume());
    EXPECT_EQ(object.activePortIndex(), -1);
    uint8_t buffer[1024];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    const float volumes[] = {0.125f, 1.0f};
    const uint32_t channels[] = {SPA_AUDIO_CHANNEL_FL, SPA_AUDIO_CHANNEL_FR};
    const auto* param = static_cast<spa_pod*>(spa_pod_builder_add_object(
        &builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props, SPA_PROP_channelVolumes,
        SPA_POD_Array(sizeof(float), SPA_TYPE_Float, 2, volumes), SPA_PROP_channelMap,
        SPA_POD_Array(sizeof(uint32_t), SPA_TYPE_Id, 2, channels), SPA_PROP_mute, SPA_POD_Bool(true)));
    object.updateVolume(param);
    EXPECT_TRUE(object.hasVolume());
    EXPECT_TRUE(object.muted());
    EXPECT_EQ(object.volume(), 1.0);
    EXPECT_EQ(object.channelVolumes(), (std::vector<double>{0.5, 1.0}));
    EXPECT_EQ(object.channels(), (std::vector<std::string>{"FL", "FR"}));
    object.setChannelVolume(-1, 0.5);
    object.setChannelVolume(2, 0.5);
    object.updateVolume(nullptr);
    EXPECT_EQ(object.volume(), 1.0);

    const spa_dict_item items[] = {{"node.name", "test"}, {"node.description", "Test speaker"}};
    const spa_dict properties = SPA_DICT_INIT(items, 2);
    object.updateProperties(&properties);
    EXPECT_EQ(object.description(), "Test speaker");
    EXPECT_EQ(object.displayName(), "Test speaker");
    EXPECT_EQ(object.cardIndex(), AudioObject::InvalidId);
}

TEST_F(AudioTest, ParseProfilesAndRoutes)
{
    AudioObject card(nullptr, 99, AudioObject::Card);
    uint8_t buffer[1024];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    const auto* profile = static_cast<spa_pod*>(spa_pod_builder_add_object(
        &builder, SPA_TYPE_OBJECT_ParamProfile, SPA_PARAM_EnumProfile, SPA_PARAM_PROFILE_index, SPA_POD_Int(7),
        SPA_PARAM_PROFILE_name, SPA_POD_String("stereo"), SPA_PARAM_PROFILE_description, SPA_POD_String("Stereo"),
        SPA_PARAM_PROFILE_available, SPA_POD_Id(SPA_PARAM_AVAILABILITY_yes)));
    card.updateProfile(profile, false);
    card.updateProfile(profile, false);
    card.updateProfile(profile, true);
    ASSERT_EQ(card.profiles().size(), 1u);
    EXPECT_EQ(card.activeProfileIndex(), 0);
    EXPECT_EQ(card.profiles().front().availability, AudioObject::Available);
    builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    const int devices[] = {3};
    const int profiles[] = {7};
    const auto* route = static_cast<spa_pod*>(spa_pod_builder_add_object(
        &builder, SPA_TYPE_OBJECT_ParamRoute, SPA_PARAM_EnumRoute, SPA_PARAM_ROUTE_index, SPA_POD_Int(9),
        SPA_PARAM_ROUTE_device, SPA_POD_Int(3), SPA_PARAM_ROUTE_direction, SPA_POD_Id(SPA_DIRECTION_OUTPUT),
        SPA_PARAM_ROUTE_name, SPA_POD_String("headphones"), SPA_PARAM_ROUTE_devices,
        SPA_POD_Array(sizeof(int), SPA_TYPE_Int, 1, devices), SPA_PARAM_ROUTE_profiles,
        SPA_POD_Array(sizeof(int), SPA_TYPE_Int, 1, profiles), SPA_PARAM_ROUTE_available,
        SPA_POD_Id(SPA_PARAM_AVAILABILITY_no)));
    card.updateRoute(route, false);
    card.updateRoute(route, true);
    ASSERT_EQ(card.routes().size(), 1u);
    EXPECT_EQ(card.activeRoutes().at(3), 9);
    EXPECT_EQ(card.routes().front().availability, AudioObject::Unavailable);
}

TEST_F(AudioTest, DiscoveryAndModels)
{
    AudioModel sinks(AudioObject::Sink, backend);
    AudioModel sources(AudioObject::Source, backend);
    EXPECT_EQ(sinks.count(), 2);
    EXPECT_EQ(sources.count(), 1);
    EXPECT_EQ(sources.objects().front()->name(), "test-input");
    EXPECT_EQ(sinks.defaultNode(), backend->defaultNode(AudioObject::Sink));
}

TEST_F(AudioTest, VolumeAndMute)
{
    const auto sink = find("test-output-a");
    ASSERT_TRUE(until([&] { return sink->volumeWritable(); }));
    sink->setVolume(0.42);
    ASSERT_TRUE(until([&] { return near(sink->volume(), 0.42); }));
    sink->setMuted(true);
    ASSERT_TRUE(until([&] { return sink->muted(); }));
    sink->setMuted(false);
    ASSERT_TRUE(until([&] { return !sink->muted(); }));
    sink->setChannelVolumes({0.25, 0.5});
    ASSERT_TRUE(until([&] { return near(sink->channelVolumes().at(0), 0.25); }));
    sink->setVolume(1.0);
    ASSERT_TRUE(until([&] { return near(sink->volume(), 1.0); }));
    EXPECT_TRUE(near(sink->channelVolumes().at(0), 0.5));
    sink->setVolume(std::numeric_limits<double>::quiet_NaN());
    sink->setChannelVolumes({0.1});
    sink->setChannelVolume(-1, 0.0);
    wait(*loop, 30ms);
    EXPECT_TRUE(near(sink->volume(), 1.0));
    sink->setVolume(-1.0);
    ASSERT_TRUE(until([&] { return sink->volume() == 0.0; }));
    sink->setVolume(5.0);
    ASSERT_TRUE(until([&] { return near(sink->volume(), 1.5); }));
    sink->setVolume(0.5);
    ASSERT_TRUE(until([&] { return near(sink->volume(), 0.5); }));
    const auto source = find("test-input");
    ASSERT_TRUE(until([&] { return source->volumeWritable(); }));
    source->setMuted(true);
    ASSERT_TRUE(until([&] { return source->muted(); }));
    source->setMuted(false);
    ASSERT_TRUE(until([&] { return !source->muted(); }));
}

TEST_F(AudioTest, ExternalChanges)
{
    const auto sink = find("test-output-a");
    ASSERT_EQ(Process::run(PW_CLI_EXECUTABLE, {"set-param", std::to_string(sink->index()), "Props",
                                               "{ channelVolumes: [ 0.064 0.064 ], mute: true }"}),
              0);
    ASSERT_TRUE(until([&] { return near(sink->volume(), 0.4); }));
    ASSERT_TRUE(until([&] { return sink->muted(); }));
    sink->setMuted(false);
    ASSERT_TRUE(until([&] { return !sink->muted(); }));
}

TEST_F(AudioTest, DefaultDevices)
{
    const auto first = find("test-output-a");
    const auto second = find("test-output-b");
    second->setDefault(true);
    ASSERT_TRUE(until([&] { return backend->defaultNode(AudioObject::Sink) == second; }));
    EXPECT_TRUE(second->isDefault());
    EXPECT_FALSE(first->isDefault());
    first->setDefault(true);
    ASSERT_TRUE(until([&] { return backend->defaultNode(AudioObject::Sink) == first; }));
    find("test-input")->setDefault(true);
    ASSERT_TRUE(until([&] { return backend->defaultNode(AudioObject::Source) == find("test-input"); }));
}

TEST_F(AudioTest, Hotplug)
{
    AudioModel sinks(AudioObject::Sink, backend);
    auto* properties = pw_properties_new("factory.name", "support.null-audio-sink", PW_KEY_NODE_NAME, "hotplug-output",
                                         PW_KEY_MEDIA_CLASS, "Audio/Sink", "audio.position", "[ FL FR ]", nullptr);
    auto* proxy = static_cast<pw_proxy*>(
        pw_core_create_object(backend->core(), "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &properties->dict, 0));
    pw_properties_free(properties);
    ASSERT_TRUE(proxy);
    ASSERT_TRUE(until([&] { return sinks.count() == 3; }));
    std::weak_ptr<AudioObject> device = find("hotplug-output");
    ASSERT_FALSE(device.expired());
    device.lock()->setDefault(true);
    ASSERT_TRUE(until([&] { return backend->defaultNode(AudioObject::Sink) == device.lock(); }));
    pw_proxy_destroy(proxy);
    ASSERT_TRUE(until([&] { return sinks.count() == 2; }));
    ASSERT_TRUE(until([&] { return device.expired(); }));
    ASSERT_TRUE(until([&] { return backend->defaultNode(AudioObject::Sink) != nullptr; }));
}

TEST_F(AudioTest, HardwareRoutes)
{
    FakeAudioDevice device;
    ASSERT_TRUE(device.exportTo(backend->core()));
    ASSERT_TRUE(until([] { return find("test-card") != nullptr; }));
    const auto card = find("test-card");
    ASSERT_TRUE(until([&] { return card->profiles().size() == 2; }));
    ASSERT_TRUE(until([&] { return card->activeProfileIndex() >= 0; }));
    card->setActiveProfileIndex(1);
    ASSERT_TRUE(until([&] { return device.profile == 8; }));
    ASSERT_TRUE(until([&] { return card->activeProfileIndex() == 1; }));
    EXPECT_TRUE(device.saved);

    const std::string cardId = std::to_string(card->index());
    auto* properties = pw_properties_new("factory.name", "support.null-audio-sink", PW_KEY_NODE_NAME, "test-card-output",
                                         PW_KEY_MEDIA_CLASS, "Audio/Sink", PW_KEY_DEVICE_ID, cardId.c_str(),
                                         "card.profile.device", "3", "audio.position", "[ FL FR ]", nullptr);
    auto* proxy = static_cast<pw_proxy*>(
        pw_core_create_object(backend->core(), "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &properties->dict, 0));
    pw_properties_free(properties);
    ASSERT_TRUE(proxy);
    ASSERT_TRUE(until([] { return find("test-card-output") != nullptr; }));
    const auto sink = find("test-card-output");
    ASSERT_TRUE(until([&] { return sink->volumeWritable(); }));
    ASSERT_TRUE(until([&] { return sink->ports().size() == 2; }));
    sink->setActivePortIndex(1);
    ASSERT_TRUE(until([&] { return device.route == 10; }));
    ASSERT_TRUE(until([&] { return sink->activePortIndex() == 1; }));
    EXPECT_EQ(sink->displayName(), "Headphones");
    sink->setChannelVolumes({0.5, 0.5});
    ASSERT_TRUE(until([&] { return device.volumes[0] == 0.125f && device.volumes[1] == 0.125f; }));
    ASSERT_TRUE(until([&] { return near(sink->volume(), 0.5); }));
    EXPECT_TRUE(device.saved);
    sink->setVolume(0.25);
    ASSERT_TRUE(until([&] { return device.volumes[0] == 0.015625f; }));
    ASSERT_TRUE(until([&] { return near(sink->volume(), 0.25); }));
    sink->setMuted(true);
    ASSERT_TRUE(until([&] { return device.muted; }));
    ASSERT_TRUE(until([&] { return sink->muted(); }));
    const int writes = device.writes;
    sink->setActivePortIndex(-1);
    card->setActiveProfileIndex(999);
    wait(*loop, 30ms);
    EXPECT_EQ(device.writes, writes);

    device.volumes[0] = 0.064f;
    device.volumes[1] = 0.064f;
    device.muted = false;
    device.notify();
    ASSERT_TRUE(until([&] { return near(sink->volume(), 0.4); }));
    ASSERT_TRUE(until([&] { return !sink->muted(); }));
    pw_proxy_destroy(proxy);
    ASSERT_TRUE(until([] { return find("test-card-output") == nullptr; }));
}

TEST_F(AudioTest, StreamRouting)
{
    const auto first = find("test-output-a");
    const auto second = find("test-output-b");
    TestStream playback(backend->core(), "test-playback", first->property(PW_KEY_OBJECT_SERIAL));
    ASSERT_TRUE(playback.connected);
    ASSERT_TRUE(until([&] { return playback.streaming; }));
    ASSERT_TRUE(until([] { return find("test-playback") != nullptr; }));
    const auto stream = find("test-playback");
    EXPECT_EQ(stream->kind(), AudioObject::Playback);
    ASSERT_TRUE(until([&] { return stream->deviceIndex() == first->index(); }));
    ASSERT_TRUE(until([&] { return stream->volumeWritable(); }));
    stream->setVolume(0.25);
    ASSERT_TRUE(until([&] { return near(stream->volume(), 0.25); }));
    stream->setMuted(true);
    ASSERT_TRUE(until([&] { return stream->muted(); }));
    stream->setDeviceIndex(second->index());
    ASSERT_TRUE(until([&] { return stream->deviceIndex() == second->index(); }));
    stream->setDeviceIndex(find("test-input")->index());
    wait(*loop, 30ms);
    EXPECT_EQ(stream->deviceIndex(), second->index());
    first->switchStreams();
    ASSERT_TRUE(until([&] { return stream->deviceIndex() == first->index(); }));

    TestStream recording(backend->core(), "test-recording", find("test-input")->property(PW_KEY_OBJECT_SERIAL), true);
    ASSERT_TRUE(recording.connected);
    ASSERT_TRUE(until([&] { return recording.streaming; }));
    ASSERT_TRUE(until([] { return find("test-recording") != nullptr; }));
    EXPECT_EQ(find("test-recording")->kind(), AudioObject::Capture);
    ASSERT_TRUE(until([] { return find("test-recording")->deviceIndex() == find("test-input")->index(); }));

    auto* properties = pw_properties_new("factory.name", "audiotestsrc", PW_KEY_NODE_NAME, "second-input",
                                         PW_KEY_MEDIA_CLASS, "Audio/Source", "audio.position", "[ FL FR ]", nullptr);
    auto* proxy = static_cast<pw_proxy*>(
        pw_core_create_object(backend->core(), "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &properties->dict, 0));
    pw_properties_free(properties);
    ASSERT_TRUE(proxy);
    ASSERT_TRUE(until([] { return find("second-input") != nullptr; }));
    find("second-input")->setDefault(true);
    ASSERT_TRUE(until([] { return backend->defaultNode(AudioObject::Source) == find("second-input"); }));
    find("test-recording")->setDeviceIndex(find("second-input")->index());
    ASSERT_TRUE(until([] { return find("test-recording")->deviceIndex() == find("second-input")->index(); }));
    find("test-recording")->setDeviceIndex(find("test-input")->index());
    ASSERT_TRUE(until([] { return find("test-recording")->deviceIndex() == find("test-input")->index(); }));
    pw_proxy_destroy(proxy);
    ASSERT_TRUE(until([] { return find("second-input") == nullptr; }));
    ASSERT_TRUE(until([] { return backend->defaultNode(AudioObject::Source) == find("test-input"); }));
}

TEST_F(AudioTest, Monitor)
{
    const auto sink = find("test-output-a");
    sink->setVolume(1.0);
    ASSERT_TRUE(until([&] { return near(sink->volume(), 1.0); }));
    VolumeMonitor monitor(backend);
    monitor.setTarget(sink);
    double peak = 0.0;
    const auto subscription = monitor.volumeChanged.connect([&] { peak = std::max(peak, monitor.volume()); });
    TestStream tone(backend->core(), "test-tone", sink->property(PW_KEY_OBJECT_SERIAL), false, 0.5f);
    ASSERT_TRUE(tone.connected);
    ASSERT_TRUE(waitFor(*loop, [&] { return monitor.isAvailable(); }, 2s));
    ASSERT_TRUE(waitFor(*loop, [&] { return peak > 0.01; }, 2s));
    monitor.setTarget(nullptr);
    EXPECT_EQ(monitor.volume(), 0.0);
    EXPECT_FALSE(monitor.isAvailable());
}

TEST_F(AudioTest, Reconnect)
{
    AudioModel sinks(AudioObject::Sink, backend);
    std::weak_ptr<AudioObject> previous = find("test-output-a");
    VolumeMonitor monitor(backend);
    monitor.setTarget(previous.lock());
    stopServers();
    ASSERT_TRUE(until([] { return !backend->isConnected(); }));
    EXPECT_EQ(sinks.count(), 0);
    EXPECT_FALSE(backend->defaultNode(AudioObject::Sink));
    ASSERT_TRUE(until([&] { return previous.expired(); }));
    EXPECT_FALSE(monitor.target());
    startServers();
    ASSERT_TRUE(waitFor(*loop, [] { return backend->isConnected(); }, 10s));
    ASSERT_TRUE(waitFor(*loop, [&] { return sinks.count() == 2; }, 10s));
    ASSERT_TRUE(waitFor(*loop, [] { return backend->defaultNode(AudioObject::Sink) != nullptr; }, 10s));
    activateDevices();
    const auto sink = find("test-output-a");
    ASSERT_TRUE(until([&] { return sink->volumeWritable(); }));
    sink->setVolume(0.3);
    ASSERT_TRUE(until([&] { return near(sink->volume(), 0.3); }));
}

// As a page that shows sound opens and closes: each backend reads the graph
// and is the last one to go.
TEST_F(AudioTest, LastBackendComesAndGoes)
{
    drivers.clear();
    backend.reset();
    for (int i = 0; i < 5; ++i) {
        backend = Backend::shared();
        ASSERT_TRUE(waitFor(*loop, [] { return backend->isConnected() && find("test-output-a"); }, 10s));
        backend.reset();
    }
    backend = Backend::shared();
    ASSERT_TRUE(waitFor(*loop, [] { return backend->isConnected() && find("test-output-a"); }, 10s));
}
