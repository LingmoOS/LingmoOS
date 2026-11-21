// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/file_watcher.h"

#include <filesystem>
#include <map>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "base/event_loop.h"

namespace Lingmo::base {

static constexpr uint32_t kMask = IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_CLOSE_WRITE | IN_ATTRIB
    | IN_DELETE_SELF | IN_MOVE_SELF | IN_ONLYDIR;

struct FileWatch::Inotify
{
    struct Watcher
    {
        int descriptor;
        // Empty for a directory watch.
        std::string name;
        std::function<void(const std::string& name)> callback;
        // On a queue overflow; null to hear nothing of it.
        std::function<void()> overflowed;
    };

    explicit Inotify(EventLoop& loop);
    ~Inotify();

    int add(const std::string& directory, std::string name, std::function<void(const std::string&)> callback,
            std::function<void()> overflowed);
    void overflow();
    void remove(int id);
    void read();

    // Null once the loop is gone.
    EventLoop* loop;
    int fd = -1;
    int watch = 0;
    int nextId = 1;
    std::map<int, Watcher> watchers;
    // Watchers per inotify descriptor; inotify gives one per directory.
    std::map<int, int> users;
};

FileWatch::Inotify::Inotify(EventLoop& eventLoop)
    : loop(&eventLoop)
{
    fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (fd >= 0)
        watch = loop->watch(fd, EPOLLIN, [this](uint32_t) { read(); });
}

FileWatch::Inotify::~Inotify()
{
    if (loop && watch)
        loop->unwatch(watch);
    if (fd >= 0)
        close(fd);
}

int FileWatch::Inotify::add(const std::string& directory, std::string name,
                            std::function<void(const std::string&)> callback, std::function<void()> overflowed)
{
    if (fd < 0)
        return 0;
    const int descriptor = inotify_add_watch(fd, directory.c_str(), kMask);
    if (descriptor < 0)
        return 0;
    ++users[descriptor];
    const int id = nextId++;
    watchers.emplace(id, Watcher{descriptor, std::move(name), std::move(callback), std::move(overflowed)});
    return id;
}

void FileWatch::Inotify::remove(int id)
{
    const auto it = watchers.find(id);
    if (it == watchers.end())
        return;
    const int descriptor = it->second.descriptor;
    watchers.erase(it);
    if (--users[descriptor] == 0) {
        users.erase(descriptor);
        inotify_rm_watch(fd, descriptor);
    }
}

void FileWatch::Inotify::read()
{
    alignas(inotify_event) char buffer[8192];
    while (true) {
        const ssize_t size = ::read(fd, buffer, sizeof(buffer));
        if (size <= 0)
            return;

        // Callbacks may add and remove watchers, so each event looks them up again.
        for (ssize_t offset = 0; offset < size;) {
            const auto* event = reinterpret_cast<const inotify_event*>(buffer + offset);
            offset += ssize_t(sizeof(inotify_event) + event->len);
            if (event->mask & IN_Q_OVERFLOW) {
                overflow();
                continue;
            }
            const std::string name = event->len ? std::string(event->name) : std::string();
            const bool self = event->mask & (IN_DELETE_SELF | IN_MOVE_SELF | IN_IGNORED);

            std::vector<int> ids;
            for (const auto& [id, watcher] : watchers) {
                if (watcher.descriptor != event->wd)
                    continue;
                if (watcher.name.empty() || self || watcher.name == name)
                    ids.push_back(id);
            }
            for (const int id : ids) {
                const auto it = watchers.find(id);
                if (it == watchers.end())
                    continue;
                // The callback may remove its own watcher.
                auto callback = it->second.callback;
                callback(self ? std::string() : name);
            }
        }
    }
}

void FileWatch::Inotify::overflow()
{
    std::vector<int> ids;
    for (const auto& [id, watcher] : watchers)
        ids.push_back(id);
    for (const int id : ids) {
        const auto it = watchers.find(id);
        if (it == watchers.end() || !it->second.overflowed)
            continue;
        auto callback = it->second.overflowed;
        callback();
    }
}

FileWatch::~FileWatch()
{
    reset();
}

FileWatch::FileWatch(FileWatch&& other) noexcept
    : m_inotify(std::move(other.m_inotify))
    , m_id(std::exchange(other.m_id, 0))
{
}

FileWatch& FileWatch::operator=(FileWatch&& other) noexcept
{
    if (this != &other) {
        reset();
        m_inotify = std::move(other.m_inotify);
        m_id = std::exchange(other.m_id, 0);
    }
    return *this;
}

void FileWatch::reset()
{
    if (m_inotify && m_id)
        m_inotify->remove(m_id);
    m_inotify.reset();
    m_id = 0;
}

std::shared_ptr<FileWatch::Inotify> FileWatch::inotify(EventLoop* loop)
{
    // The instance shared per loop, on the loop's thread. It stays with the
    // loop: closing an inotify descriptor waits for the kernel to settle,
    // which takes milliseconds.
    static thread_local std::map<EventLoop*, std::shared_ptr<Inotify>> s_instances;

    if (!loop)
        loop = EventLoop::current();
    if (!loop)
        return nullptr;
    auto& instance = s_instances[loop];
    if (instance)
        return instance;
    instance = std::make_shared<Inotify>(*loop);
    loop->addDestroyHook([loop] {
        const auto it = s_instances.find(loop);
        if (it == s_instances.end())
            return;
        loop->unwatch(it->second->watch);
        it->second->loop = nullptr;
        s_instances.erase(it);
    });
    return instance;
}

FileWatch FileWatch::file(const std::string& path, std::function<void()> changed, EventLoop* loop)
{
    FileWatch result;
    result.m_inotify = inotify(loop);
    if (!result.m_inotify)
        return {};
    const std::filesystem::path file(path);
    std::string directory = file.parent_path().string();
    if (directory.empty())
        directory = ".";
    auto shared = std::make_shared<std::function<void()>>(std::move(changed));
    result.m_id = result.m_inotify->add(directory, file.filename().string(),
                                        [shared](const std::string&) { (*shared)(); }, [shared] { (*shared)(); });
    if (!result.m_id)
        result.m_inotify.reset();
    return result;
}

FileWatch FileWatch::directory(const std::string& path, std::function<void(const std::string& name)> changed,
                               EventLoop* loop)
{
    return directory(path, std::move(changed), nullptr, loop);
}

FileWatch FileWatch::directory(const std::string& path, std::function<void(const std::string& name)> changed,
                               std::function<void()> overflowed, EventLoop* loop)
{
    FileWatch result;
    result.m_inotify = inotify(loop);
    if (!result.m_inotify)
        return {};
    result.m_id = result.m_inotify->add(path, std::string(), std::move(changed), std::move(overflowed));
    if (!result.m_id)
        result.m_inotify.reset();
    return result;
}

} // namespace Lingmo::base
