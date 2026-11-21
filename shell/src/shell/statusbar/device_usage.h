// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"

struct pw_context;
struct pw_core;
struct pw_loop;
struct pw_registry;

namespace Lingmo::Shell {

class TaskManager;

// One process using one capture device (Lingmo.Privacy's DeviceUse).
struct DeviceUse {
    enum Kind {
        Camera,
        Microphone,
        SystemAudio,
    };

    Kind kind = Camera;
    uint32_t pid = 0;
    // The sandboxed application's id, as the portal granted it access.
    std::string appId;
    // What the process calls itself; a fallback for one no application claims.
    std::string name;

    bool operator==(const DeviceUse &) const = default;
};

using DeviceUses = std::vector<DeviceUse>;

// The streams that capture from a camera, a microphone or a sink's monitor,
// read from the PipeWire graph on the main loop (Lingmo.Privacy's
// PipeWireCapture).
class PipeWireCapture
{
public:
    PipeWireCapture();
    ~PipeWireCapture();

    PipeWireCapture(const PipeWireCapture &) = delete;
    PipeWireCapture &operator=(const PipeWireCapture &) = delete;

    const DeviceUses &uses() const { return m_uses; }

    base::Signal<> usesChanged;

    // Called from PipeWire's callbacks; not part of the public API.
    struct Proxy;
    void addGlobal(uint32_t id, const char *type, uint32_t version, const void *properties);
    void removeGlobal(uint32_t id);
    void scheduleUpdate();
    void failSoon();

private:
    void connectToDaemon();
    void disconnectFromDaemon();
    void iterate();
    void fail();
    void update();
    DeviceUse useOf(const Proxy *stream, DeviceUse::Kind kind) const;

    pw_loop *m_loop = nullptr;
    pw_context *m_context = nullptr;
    pw_core *m_core = nullptr;
    pw_registry *m_registry = nullptr;
    struct Hooks;
    std::unique_ptr<Hooks> m_hooks;
    int m_watch = 0;
    base::Timer m_retry;
    std::map<uint32_t, std::unique_ptr<Proxy>> m_proxies;
    // Link id to its output and input node.
    std::map<uint32_t, std::pair<uint32_t, uint32_t>> m_links;
    DeviceUses m_uses;
    bool m_iterating = false;
    bool m_updatePending = false;
    bool m_failurePending = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

// The processes that hold a video device open themselves, outside PipeWire:
// opens and closes of /dev/video* arrive through inotify, the processes'
// descriptors are read only then, and an open must last a second to count.
class VideoDeviceWatch
{
public:
    static constexpr std::chrono::milliseconds kSettleDelay {1000};

    explicit VideoDeviceWatch(std::string deviceDirectory = "/dev", std::string processDirectory = "/proc");
    ~VideoDeviceWatch();

    VideoDeviceWatch(const VideoDeviceWatch &) = delete;
    VideoDeviceWatch &operator=(const VideoDeviceWatch &) = delete;

    const DeviceUses &uses() const { return m_uses; }
    // Reads the processes now.
    void scan();

    base::Signal<> usesChanged;

private:
    void readEvents();
    void watchDevices();

    std::string m_deviceDirectory;
    std::string m_processDirectory;
    int m_fd = -1;
    int m_directoryWatch = -1;
    int m_watch = 0;
    // Watch descriptor to device path.
    std::map<int, std::string> m_devices;
    base::Timer m_settle;
    DeviceUses m_uses;
};

// Which processes use a camera, a microphone, or record the system's audio
// (Lingmo.Privacy's DeviceUsage), cameras first; and the applications
// behind them (the status bar's DeviceUsers), one entry per application.
class DeviceUsage
{
public:
    struct Application {
        std::string name;
        // The entry's icon, empty for a process no application claims.
        std::string icon;
        bool camera = false;
        bool microphone = false;
        bool systemAudio = false;

        bool operator==(const Application &) const = default;
    };

    // `capture` and `videoDevices` may be null (tests); `tasks` too.
    DeviceUsage(std::unique_ptr<PipeWireCapture> capture, std::unique_ptr<VideoDeviceWatch> videoDevices,
                TaskManager *tasks);
    ~DeviceUsage();

    // The session's: PipeWire's graph and /dev.
    static std::unique_ptr<DeviceUsage> create(TaskManager *tasks);

    const DeviceUses &uses() const { return m_uses; }
    bool camera() const { return isUsed(DeviceUse::Camera); }
    bool microphone() const { return isUsed(DeviceUse::Microphone); }
    bool systemAudio() const { return isUsed(DeviceUse::SystemAudio); }
    const std::vector<Application> &applications() const { return m_applications; }

    // Tests set the uses themselves.
    void setUsesForTesting(DeviceUses uses);

    base::Signal<> usesChanged;
    base::Signal<> applicationsChanged;

private:
    bool isUsed(DeviceUse::Kind kind) const;
    void update();
    void setUses(DeviceUses uses);
    void updateApplications();

    std::unique_ptr<PipeWireCapture> m_capture;
    std::unique_ptr<VideoDeviceWatch> m_videoDevices;
    TaskManager *m_tasks;
    DeviceUses m_uses;
    std::vector<Application> m_applications;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

