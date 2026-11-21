// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "backend.h"

#include <string>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

struct fuse;

// A backend served at a local folder by its own FUSE session and thread.
class FuseMount
{
public:
    // finished runs on the session's thread once the mount has gone, whoever unmounted it.
    FuseMount(std::unique_ptr<Backend> backend, const std::string &mountPoint, const std::string &source,
              std::function<void(FuseMount *)> finished);
    ~FuseMount();

    // Mounts and starts serving; false with message when the kernel mount fails.
    bool start(std::string *message);
    // Unmounts unless something has files open there; force detaches it regardless, and the
    // session ends once the last file there is closed.
    bool stop(bool force, std::string *message);

    const std::string &mountPoint() const
    {
        return m_mountPoint;
    }

    // Whether the session ended by itself, as when someone ran fusermount -u.
    bool finished() const;

    Backend *backend() const
    {
        return m_backend.get();
    }

    std::mutex &lock()
    {
        return m_lock;
    }

private:
    std::unique_ptr<Backend> m_backend;
    std::string m_mountPoint;
    std::string m_source;
    struct fuse *m_fuse = nullptr;
    std::thread m_thread;
    std::mutex m_lock;
    std::atomic_bool m_finished = false;
    std::function<void(FuseMount *)> m_onFinished;
};
