// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>

namespace Lingmo::base {

class EventLoop;

// Follows a file or directory through inotify, which one instance per loop
// serves; stops when it goes. Changes arrive once a writer closes the file
// or renames it into place, not for every write.
class [[nodiscard]] FileWatch
{
public:
    FileWatch() = default;
    ~FileWatch();

    FileWatch(FileWatch&& other) noexcept;
    FileWatch& operator=(FileWatch&& other) noexcept;

    // Calls back when the file is written, replaced, created or removed. The
    // directory is what is watched, so a file replaced by a rename, removed
    // or not there yet keeps being followed; the directory must exist.
    static FileWatch file(const std::string& path, std::function<void()> changed, EventLoop* loop = nullptr);
    // Calls back with the name of an entry created, removed, renamed or
    // written, or an empty name when the directory itself goes.
    static FileWatch directory(const std::string& path, std::function<void(const std::string& name)> changed,
                               EventLoop* loop = nullptr);
    // The same, and `overflowed` when the kernel's event queue overflowed
    // and changes were lost, so a listing should be read again. A file watch
    // takes an overflow as a change.
    static FileWatch directory(const std::string& path, std::function<void(const std::string& name)> changed,
                               std::function<void()> overflowed, EventLoop* loop = nullptr);

    bool isActive() const { return m_id != 0; }
    void reset();

private:
    struct Inotify;
    static std::shared_ptr<Inotify> inotify(EventLoop* loop);

    std::shared_ptr<Inotify> m_inotify;
    int m_id = 0;
};

} // namespace Lingmo::base
