// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/device_usage.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstring>
#include <fstream>
#include <optional>
#include <tuple>

#include <dirent.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <pipewire/pipewire.h>

#include "base/event_loop.h"
#include "core/main_loop.h"
#include "shell/common/task_manager.h"

namespace Lingmo::Shell {

using Properties = std::map<std::string, std::string>;

static Properties toMap(const spa_dict *dict)
{
    Properties map;
    if (!dict)
        return map;
    const spa_dict_item *item;
    spa_dict_for_each(item, dict)
    {
        if (item->key && item->value)
            map[item->key] = item->value;
    }
    return map;
}

static std::string valueOf(const Properties &properties, const char *key)
{
    const auto it = properties.find(key);
    return it != properties.end() ? it->second : std::string();
}

static uint32_t toId(const std::string &text)
{
    if (text.empty() || text.size() > 10)
        return 0;
    uint64_t value = 0;
    for (char c : text) {
        if (c < '0' || c > '9')
            return 0;
        value = value * 10 + uint64_t(c - '0');
    }
    return value <= UINT32_MAX ? uint32_t(value) : 0;
}

static bool isWatchedClass(const char *mediaClass)
{
    static const char *const classes[] = {"Audio/Source", "Audio/Source/Virtual", "Audio/Sink",
                                          "Video/Source", "Stream/Input/Audio", "Stream/Input/Video"};
    for (const char *watched : classes) {
        if (std::strcmp(mediaClass, watched) == 0)
            return true;
    }
    return false;
}

static bool isCamera(const Properties &properties)
{
    const std::string api = valueOf(properties, PW_KEY_DEVICE_API);
    return valueOf(properties, PW_KEY_MEDIA_ROLE) == "Camera" || api == "v4l2" || api == "libcamera";
}

struct PipeWireCapture::Proxy {
    PipeWireCapture *owner = nullptr;
    uint32_t id = 0;
    bool client = false;
    pw_proxy *proxy = nullptr;
    spa_hook listener {};
    Properties properties;
    bool running = false;
};

struct PipeWireCapture::Hooks {
    spa_hook core {};
    spa_hook registry {};
};

static const pw_core_events kCoreEvents = [] {
    pw_core_events events {};
    events.version = PW_VERSION_CORE_EVENTS;
    events.error = [](void *data, uint32_t id, int, int result, const char *) {
        if (id == PW_ID_CORE && (result == -EPIPE || result == -ECONNRESET))
            static_cast<PipeWireCapture *>(data)->failSoon();
    };
    return events;
}();

static const pw_registry_events kRegistryEvents = [] {
    pw_registry_events events {};
    events.version = PW_VERSION_REGISTRY_EVENTS;
    events.global = [](void *data, uint32_t id, uint32_t, const char *type, uint32_t version,
                       const spa_dict *properties) {
        static_cast<PipeWireCapture *>(data)->addGlobal(id, type, version, properties);
    };
    events.global_remove = [](void *data, uint32_t id) { static_cast<PipeWireCapture *>(data)->removeGlobal(id); };
    return events;
}();

static const pw_node_events kNodeEvents = [] {
    pw_node_events events {};
    events.version = PW_VERSION_NODE_EVENTS;
    events.info = [](void *data, const pw_node_info *info) {
        auto *proxy = static_cast<PipeWireCapture::Proxy *>(data);
        if (info->change_mask & PW_NODE_CHANGE_MASK_PROPS)
            proxy->properties = toMap(info->props);
        if (info->change_mask & PW_NODE_CHANGE_MASK_STATE)
            proxy->running = info->state == PW_NODE_STATE_RUNNING;
        proxy->owner->scheduleUpdate();
    };
    return events;
}();

static const pw_client_events kClientEvents = [] {
    pw_client_events events {};
    events.version = PW_VERSION_CLIENT_EVENTS;
    events.info = [](void *data, const pw_client_info *info) {
        auto *proxy = static_cast<PipeWireCapture::Proxy *>(data);
        if (info->change_mask & PW_CLIENT_CHANGE_MASK_PROPS) {
            proxy->properties = toMap(info->props);
            proxy->owner->scheduleUpdate();
        }
    };
    return events;
}();

PipeWireCapture::PipeWireCapture()
    : m_hooks(std::make_unique<Hooks>())
    , m_retry(core::mainLoop())
{
    pw_init(nullptr, nullptr);
    m_retry.setSingleShot(true);
    m_retry.onTimeout = [this] { connectToDaemon(); };
    connectToDaemon();
}

PipeWireCapture::~PipeWireCapture()
{
    *m_alive = false;
    m_retry.stop();
    disconnectFromDaemon();
    if (m_context)
        pw_context_destroy(m_context);
    if (m_loop) {
        pw_loop_leave(m_loop);
        pw_loop_destroy(m_loop);
    }
    pw_deinit();
}

void PipeWireCapture::failSoon()
{
    if (m_failurePending)
        return;
    m_failurePending = true;
    core::mainLoop()->post([this, alive = m_alive] {
        if (*alive)
            fail();
    });
}

void PipeWireCapture::connectToDaemon()
{
    if (m_core)
        return;
    m_failurePending = false;
    if (!m_loop) {
        m_loop = pw_loop_new(nullptr);
        if (m_loop)
            pw_loop_enter(m_loop);
    }
    if (m_loop && !m_context)
        m_context = pw_context_new(m_loop, nullptr, 0);
    if (m_context) {
        m_core = pw_context_connect(m_context,
                                    pw_properties_new(PW_KEY_APP_NAME, "Lingmo Privacy", PW_KEY_APP_ID,
                                                      "org.lingmo.Privacy", nullptr),
                                    0);
    }
    if (!m_core) {
        fail();
        return;
    }
    pw_core_add_listener(m_core, &m_hooks->core, &kCoreEvents, this);
    m_registry = pw_core_get_registry(m_core, PW_VERSION_REGISTRY, 0);
    if (!m_registry) {
        fail();
        return;
    }
    pw_registry_add_listener(m_registry, &m_hooks->registry, &kRegistryEvents, this);
    m_watch = core::mainLoop()->watch(pw_loop_get_fd(m_loop), EPOLLIN, [this](uint32_t) { iterate(); });
}

void PipeWireCapture::disconnectFromDaemon()
{
    if (m_watch) {
        core::mainLoop()->unwatch(m_watch);
        m_watch = 0;
    }
    std::vector<uint32_t> ids;
    for (const auto &[id, proxy] : m_proxies)
        ids.push_back(id);
    for (uint32_t id : ids)
        removeGlobal(id);
    m_links.clear();
    if (m_registry) {
        spa_hook_remove(&m_hooks->registry);
        pw_proxy_destroy(reinterpret_cast<pw_proxy *>(m_registry));
        m_registry = nullptr;
    }
    if (m_core) {
        spa_hook_remove(&m_hooks->core);
        pw_core_disconnect(m_core);
        m_core = nullptr;
    }
    update();
}

void PipeWireCapture::iterate()
{
    if (!m_loop || m_iterating)
        return;
    m_iterating = true;
    const int result = pw_loop_iterate(m_loop, 0);
    m_iterating = false;
    if (result < 0 && result != -EINTR)
        failSoon();
}

void PipeWireCapture::fail()
{
    disconnectFromDaemon();
    m_retry.start(std::chrono::seconds(1));
}

void PipeWireCapture::addGlobal(uint32_t id, const char *type, uint32_t version, const void *dict)
{
    const auto *properties = static_cast<const spa_dict *>(dict);
    if (!properties)
        return;
    if (std::strcmp(type, PW_TYPE_INTERFACE_Link) == 0) {
        const char *output = spa_dict_lookup(properties, PW_KEY_LINK_OUTPUT_NODE);
        const char *input = spa_dict_lookup(properties, PW_KEY_LINK_INPUT_NODE);
        if (output && input && toId(output) && toId(input)) {
            m_links[id] = {toId(output), toId(input)};
            scheduleUpdate();
        }
        return;
    }
    const bool client = std::strcmp(type, PW_TYPE_INTERFACE_Client) == 0;
    if (!client) {
        const char *mediaClass = spa_dict_lookup(properties, PW_KEY_MEDIA_CLASS);
        if (std::strcmp(type, PW_TYPE_INTERFACE_Node) != 0 || !mediaClass || !isWatchedClass(mediaClass))
            return;
    }
    if (m_proxies.count(id))
        return;
    auto *bound = static_cast<pw_proxy *>(pw_registry_bind(
            m_registry, id, type, std::min(version, uint32_t(client ? PW_VERSION_CLIENT : PW_VERSION_NODE)), 0));
    if (!bound)
        return;
    auto proxy = std::make_unique<Proxy>();
    proxy->owner = this;
    proxy->id = id;
    proxy->client = client;
    proxy->proxy = bound;
    proxy->properties = toMap(properties);
    Proxy *raw = proxy.get();
    m_proxies[id] = std::move(proxy);
    if (client)
        pw_client_add_listener(reinterpret_cast<pw_client *>(bound), &raw->listener, &kClientEvents, raw);
    else
        pw_node_add_listener(reinterpret_cast<pw_node *>(bound), &raw->listener, &kNodeEvents, raw);
}

void PipeWireCapture::removeGlobal(uint32_t id)
{
    if (const auto it = m_proxies.find(id); it != m_proxies.end()) {
        spa_hook_remove(&it->second->listener);
        pw_proxy_destroy(it->second->proxy);
        m_proxies.erase(it);
        scheduleUpdate();
    }
    if (m_links.erase(id))
        scheduleUpdate();
}

void PipeWireCapture::scheduleUpdate()
{
    // A stream starting brings its node, links and state in one burst.
    if (m_updatePending)
        return;
    m_updatePending = true;
    core::mainLoop()->post([this, alive = m_alive] {
        if (*alive)
            update();
    });
}

void PipeWireCapture::update()
{
    m_updatePending = false;
    DeviceUses uses;
    for (const auto &[id, stream] : m_proxies) {
        if (stream->client || !stream->running)
            continue;
        const Properties &properties = stream->properties;
        if (!valueOf(properties, PW_KEY_MEDIA_CLASS).starts_with("Stream/Input/"))
            continue;
        // Our own level meters, peak meters, and the loopback and filter
        // plumbing that a real capture stream already stands for.
        if (valueOf(properties, PW_KEY_NODE_NAME).starts_with("lingmo.internal.")
            || valueOf(properties, PW_KEY_STREAM_MONITOR) == "true" || properties.count(PW_KEY_NODE_LINK_GROUP))
            continue;
        for (const auto &[linkId, link] : m_links) {
            if (link.second != stream->id)
                continue;
            const auto source = m_proxies.find(link.first);
            if (source == m_proxies.end() || source->second->client)
                continue;
            const std::string mediaClass = valueOf(source->second->properties, PW_KEY_MEDIA_CLASS);
            std::optional<DeviceUse::Kind> kind;
            if (mediaClass == "Video/Source" && isCamera(source->second->properties))
                kind = DeviceUse::Camera;
            else if (mediaClass.starts_with("Audio/Source"))
                kind = DeviceUse::Microphone;
            else if (mediaClass == "Audio/Sink")
                kind = DeviceUse::SystemAudio;
            if (!kind)
                continue;
            const DeviceUse use = useOf(stream.get(), *kind);
            if (std::find(uses.begin(), uses.end(), use) == uses.end())
                uses.push_back(use);
        }
    }
    std::sort(uses.begin(), uses.end(), [](const DeviceUse &a, const DeviceUse &b) {
        return std::tie(a.kind, a.pid, a.appId, a.name) < std::tie(b.kind, b.pid, b.appId, b.name);
    });
    if (uses != m_uses) {
        m_uses = std::move(uses);
        usesChanged();
    }
}

DeviceUse PipeWireCapture::useOf(const Proxy *stream, DeviceUse::Kind kind) const
{
    DeviceUse use;
    use.kind = kind;
    const auto client = m_proxies.find(toId(valueOf(stream->properties, PW_KEY_CLIENT_ID)));
    const Properties clientProperties =
            client != m_proxies.end() && client->second->client ? client->second->properties : Properties();
    // A PulseAudio application's connection is pipewire-pulse's, so its own
    // process id is only the one it reports. Any other reported id may be a
    // sandbox's own, so the socket's peer is trusted instead.
    const bool pulse = valueOf(clientProperties, PW_KEY_CLIENT_API) == "pipewire-pulse";
    use.pid = toId(valueOf(clientProperties, pulse ? PW_KEY_APP_PROCESS_ID : PW_KEY_SEC_PID));
    use.appId = valueOf(clientProperties, "pipewire.access.portal.app_id");
    use.name = valueOf(stream->properties, PW_KEY_APP_NAME);
    if (use.name.empty())
        use.name = valueOf(clientProperties, PW_KEY_APP_NAME);
    return use;
}

static bool isVideoDevice(const char *name)
{
    if (std::strncmp(name, "video", 5) != 0 || !name[5])
        return false;
    for (const char *c = name + 5; *c; ++c) {
        if (*c < '0' || *c > '9')
            return false;
    }
    return true;
}

VideoDeviceWatch::VideoDeviceWatch(std::string deviceDirectory, std::string processDirectory)
    : m_processDirectory(std::move(processDirectory))
    , m_settle(core::mainLoop())
{
    char resolved[PATH_MAX];
    m_deviceDirectory = realpath(deviceDirectory.c_str(), resolved) ? std::string(resolved) : deviceDirectory;
    m_settle.setSingleShot(true);
    m_settle.onTimeout = [this] { scan(); };
    m_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (m_fd < 0)
        return;
    m_directoryWatch =
            inotify_add_watch(m_fd, m_deviceDirectory.c_str(), IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM);
    m_watch = core::mainLoop()->watch(m_fd, EPOLLIN, [this](uint32_t) { readEvents(); });
    watchDevices();
    scan();
}

VideoDeviceWatch::~VideoDeviceWatch()
{
    if (m_watch)
        core::mainLoop()->unwatch(m_watch);
    if (m_fd >= 0)
        close(m_fd);
}

void VideoDeviceWatch::readEvents()
{
    alignas(inotify_event) char buffer[4096];
    bool devicesChanged = false;
    bool accessed = false;
    for (;;) {
        const ssize_t length = read(m_fd, buffer, sizeof(buffer));
        if (length <= 0)
            break;
        for (ssize_t offset = 0; offset < length;) {
            const auto *event = reinterpret_cast<const inotify_event *>(buffer + offset);
            offset += ssize_t(sizeof(inotify_event) + event->len);
            if (event->wd == m_directoryWatch) {
                if (event->len > 0 && isVideoDevice(event->name))
                    devicesChanged = true;
            } else if (event->mask & IN_IGNORED) {
                m_devices.erase(event->wd);
                devicesChanged = true;
            } else {
                accessed = true;
            }
        }
    }
    if (devicesChanged)
        watchDevices();
    if ((devicesChanged || accessed) && !m_settle.isActive())
        m_settle.start(kSettleDelay);
}

void VideoDeviceWatch::watchDevices()
{
    DIR *directory = opendir(m_deviceDirectory.c_str());
    if (!directory)
        return;
    while (const dirent *entry = readdir(directory)) {
        if (!isVideoDevice(entry->d_name))
            continue;
        const std::string path = m_deviceDirectory + '/' + entry->d_name;
        // A device already watched keeps its descriptor: inotify returns it again.
        const int watch = inotify_add_watch(m_fd, path.c_str(), IN_OPEN | IN_CLOSE_WRITE | IN_CLOSE_NOWRITE);
        if (watch >= 0)
            m_devices[watch] = path;
    }
    closedir(directory);
}

void VideoDeviceWatch::scan()
{
    std::vector<std::string> devices;
    for (const auto &[watch, path] : m_devices)
        devices.push_back(path);
    DeviceUses uses;
    DIR *processes = devices.empty() ? nullptr : opendir(m_processDirectory.c_str());
    while (processes) {
        const dirent *process = readdir(processes);
        if (!process)
            break;
        const uint32_t pid = toId(process->d_name);
        if (!pid)
            continue;
        const std::string descriptors = m_processDirectory + '/' + process->d_name + "/fd";
        DIR *directory = opendir(descriptors.c_str());
        if (!directory)
            continue;
        bool holds = false;
        while (const dirent *entry = readdir(directory)) {
            if (entry->d_name[0] == '.')
                continue;
            char target[PATH_MAX];
            const std::string path = descriptors + '/' + entry->d_name;
            const ssize_t length = readlink(path.c_str(), target, sizeof(target));
            if (length > 0
                && std::find(devices.begin(), devices.end(), std::string(target, size_t(length))) != devices.end()) {
                holds = true;
                break;
            }
        }
        closedir(directory);
        if (!holds)
            continue;
        std::string name;
        std::ifstream comm(m_processDirectory + '/' + process->d_name + "/comm");
        std::getline(comm, name);
        // PipeWire holds the devices it serves; its graph names the clients.
        if (name == "pipewire" || name == "wireplumber")
            continue;
        uses.push_back({DeviceUse::Camera, pid, {}, name});
    }
    if (processes)
        closedir(processes);
    std::sort(uses.begin(), uses.end(),
              [](const DeviceUse &a, const DeviceUse &b) { return std::tie(a.pid, a.name) < std::tie(b.pid, b.name); });
    if (uses != m_uses) {
        m_uses = std::move(uses);
        usesChanged();
    }
}

DeviceUsage::DeviceUsage(std::unique_ptr<PipeWireCapture> capture, std::unique_ptr<VideoDeviceWatch> videoDevices,
                         TaskManager *tasks)
    : m_capture(std::move(capture))
    , m_videoDevices(std::move(videoDevices))
    , m_tasks(tasks)
{
    if (m_capture)
        m_subscriptions.push_back(m_capture->usesChanged.connect([this] { update(); }));
    if (m_videoDevices)
        m_subscriptions.push_back(m_videoDevices->usesChanged.connect([this] { update(); }));
    if (m_tasks)
        m_subscriptions.push_back(m_tasks->windowEntryChanged.connect([this](uint64_t) { updateApplications(); }));
    update();
}

DeviceUsage::~DeviceUsage() = default;

std::unique_ptr<DeviceUsage> DeviceUsage::create(TaskManager *tasks)
{
    return std::make_unique<DeviceUsage>(std::make_unique<PipeWireCapture>(), std::make_unique<VideoDeviceWatch>(),
                                         tasks);
}

bool DeviceUsage::isUsed(DeviceUse::Kind kind) const
{
    return std::any_of(m_uses.begin(), m_uses.end(), [kind](const DeviceUse &use) { return use.kind == kind; });
}

void DeviceUsage::update()
{
    DeviceUses uses = m_capture ? m_capture->uses() : DeviceUses();
    if (m_videoDevices) {
        for (const DeviceUse &use : m_videoDevices->uses()) {
            if (std::find(uses.begin(), uses.end(), use) == uses.end())
                uses.push_back(use);
        }
    }
    setUses(std::move(uses));
}

void DeviceUsage::setUsesForTesting(DeviceUses uses)
{
    setUses(std::move(uses));
}

void DeviceUsage::setUses(DeviceUses uses)
{
    std::stable_sort(uses.begin(), uses.end(), [](const DeviceUse &a, const DeviceUse &b) { return a.kind < b.kind; });
    if (uses == m_uses)
        return;
    m_uses = std::move(uses);
    usesChanged();
    updateApplications();
}

void DeviceUsage::updateApplications()
{
    std::vector<Application> applications;
    std::vector<std::string> keys;
    for (const DeviceUse &use : m_uses) {
        const DesktopEntryPtr entry = m_tasks ? m_tasks->entryForProcess(use.pid, use.appId) : nullptr;
        const std::string name = entry ? entry->name : use.name;
        const std::string key = entry ? entry->id : name;
        auto it = std::find(keys.begin(), keys.end(), key);
        size_t index = size_t(it - keys.begin());
        if (it == keys.end()) {
            keys.push_back(key);
            applications.push_back({name, entry ? entry->icon : std::string(), false, false, false});
        }
        Application &application = applications[index];
        switch (use.kind) {
        case DeviceUse::Camera:
            application.camera = true;
            break;
        case DeviceUse::Microphone:
            application.microphone = true;
            break;
        case DeviceUse::SystemAudio:
            application.systemAudio = true;
            break;
        }
    }
    if (applications == m_applications)
        return;
    m_applications = std::move(applications);
    applicationsChanged();
}

} // namespace Lingmo::Shell
