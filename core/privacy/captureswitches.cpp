// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "captureswitches.h"

#include "base/event_loop.h"
#include "base/logging.h"

#include <algorithm>
#include <errno.h>
#include <filesystem>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <sound/asound.h>

#include <vector>

static const Lingmo::Base::LogCategory kLog("lingmo.privacy");

static std::vector<snd_ctl_elem_id> captureSwitches(int fd)
{
    snd_ctl_elem_list list = {};
    if (::ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LIST, &list) < 0)
        return {};

    std::vector<snd_ctl_elem_id> ids(list.count);
    list.space = list.count;
    list.pids = ids.data();
    if (::ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LIST, &list) < 0)
        return {};

    // ALSA names every capture mute "<source> Capture Switch", or just "Capture Switch".
    std::vector<snd_ctl_elem_id> switches;
    for (unsigned int i = 0; i < list.used; ++i) {
        const std::string name(reinterpret_cast<const char *>(ids[i].name));
        if (ids[i].iface == SNDRV_CTL_ELEM_IFACE_MIXER && name.ends_with("Capture Switch"))
            switches.push_back(ids[i]);
    }

    return switches;
}

static bool writeSwitch(int fd, const snd_ctl_elem_id &id, bool on)
{
    snd_ctl_elem_info info = {};
    info.id = id;
    if (::ioctl(fd, SNDRV_CTL_IOCTL_ELEM_INFO, &info) < 0 || info.type != SNDRV_CTL_ELEM_TYPE_BOOLEAN)
        return false;

    snd_ctl_elem_value value = {};
    value.id = info.id;
    for (unsigned int i = 0; i < info.count && i < std::size(value.value.integer.value); ++i)
        value.value.integer.value[i] = on ? 1 : 0;

    return ::ioctl(fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &value) >= 0;
}

CaptureSwitches::CaptureSwitches(const std::string &deviceDirectory)
    : m_deviceDirectory(deviceDirectory)
{
}

CaptureSwitches::~CaptureSwitches()
{
    for (const auto &[name, fd] : m_cards)
        ::close(fd);

    if (m_watch)
        Lingmo::Base::EventLoop::current()->unwatch(m_watch);

    if (m_inotify >= 0)
        ::close(m_inotify);
}

void CaptureSwitches::setLocked(bool locked)
{
    if (m_locked == locked)
        return;

    m_locked = locked;

    if (!locked) {
        std::vector<std::string> names;
        for (const auto &[name, fd] : m_cards)
            names.push_back(name);
        for (const std::string &name : names)
            unlockCard(name);

        return;
    }

    // Sound cards plugged in while locked are locked as their control node appears.
    if (m_inotify < 0) {
        m_inotify = ::inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
        if (m_inotify >= 0 && ::inotify_add_watch(m_inotify, m_deviceDirectory.c_str(), IN_CREATE | IN_DELETE) >= 0) {
            m_watch = Lingmo::Base::EventLoop::current()->watch(m_inotify, EPOLLIN, [this](uint32_t) {
                readEvents();
            });
        } else {
            LM_WARNING(kLog) << "lingmo-privacy: cannot watch" << m_deviceDirectory << ::strerror(errno);
        }
    }

    // Control nodes are character devices, never regular files.
    std::vector<std::string> names;
    std::error_code error;
    for (const auto &item : std::filesystem::directory_iterator(m_deviceDirectory, error)) {
        const std::string name = item.path().filename().string();
        std::error_code typeError;
        if (name.starts_with("controlC") && !item.is_directory(typeError) && !item.is_regular_file(typeError))
            names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    for (const std::string &name : names)
        lockCard(name);
}

void CaptureSwitches::lockCard(const std::string &name)
{
    if (m_cards.contains(name))
        return;

    const std::string path = m_deviceDirectory + '/' + name;
    const int fd = ::open(path.c_str(), O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        LM_WARNING(kLog) << "lingmo-privacy: cannot open" << path << ::strerror(errno);
        return;
    }

    const std::vector<snd_ctl_elem_id> switches = captureSwitches(fd);
    if (switches.empty()) {
        LM_INFO(kLog) << "lingmo-privacy:" << name << "has no capture switch";
        ::close(fd);
        return;
    }

    for (snd_ctl_elem_id id : switches) {
        // Locked before the write, so nothing turns it back on in between.
        if (::ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LOCK, &id) < 0)
            LM_WARNING(kLog) << "lingmo-privacy: cannot lock" << name << reinterpret_cast<const char *>(id.name)
                             << ::strerror(errno);

        if (!writeSwitch(fd, id, false))
            LM_WARNING(kLog) << "lingmo-privacy: cannot turn off" << name
                             << reinterpret_cast<const char *>(id.name) << ::strerror(errno);
    }

    m_cards[name] = fd;
}

void CaptureSwitches::unlockCard(const std::string &name)
{
    const auto found = m_cards.find(name);
    if (found == m_cards.end())
        return;
    const int fd = found->second;
    m_cards.erase(found);

    const std::vector<snd_ctl_elem_id> switches = captureSwitches(fd);
    for (snd_ctl_elem_id id : switches) {
        writeSwitch(fd, id, true);
        ::ioctl(fd, SNDRV_CTL_IOCTL_ELEM_UNLOCK, &id);
    }

    ::close(fd);
}

void CaptureSwitches::readEvents()
{
    alignas(inotify_event) char buffer[4096];

    for (;;) {
        const ssize_t length = ::read(m_inotify, buffer, sizeof(buffer));
        if (length <= 0)
            break;

        for (ssize_t offset = 0; offset < length;) {
            const auto *event = reinterpret_cast<const inotify_event *>(buffer + offset);
            offset += sizeof(inotify_event) + event->len;

            const std::string name = event->len ? std::string(event->name) : std::string();
            if (!name.starts_with("controlC"))
                continue;

            if (event->mask & IN_DELETE) {
                const auto found = m_cards.find(name);
                if (found != m_cards.end()) {
                    ::close(found->second);
                    m_cards.erase(found);
                }
            } else if (m_locked) {
                lockCard(name);
            }
        }
    }
}
