// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cerrno>
#include <cstdint>
#include <string>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <vector>

struct DirectoryEntry
{
    std::string name;
    struct stat st = {};
};

// Credentials for a connection, as typed or found in the keyring.
struct Credentials
{
    std::string user;
    std::string domain;
    std::string password;
    bool anonymous = false;
    // A host key fingerprint the user accepted for this connection.
    std::string acceptedHostKey;
};

struct ConnectResult
{
    enum Status {
        Connected,
        NeedPassword,
        UnknownHostKey,
        Failed
    };

    Status status = Failed;
    std::string message;
    // NeedPassword: whether the server takes a domain and guests.
    bool needsDomain = false;
    bool anonymousAllowed = false;
    // UnknownHostKey: the key's SHA256 fingerprint, and whether another key was known.
    std::string fingerprint;
    bool hostKeyChanged = false;
};

// One remote file system under a FUSE mount. Paths are absolute within the mount and
// encoded as the remote end stores them. Calls are made one at a time from the mount's
// FUSE thread; each returns 0 (or a byte count) on success and a negative errno on failure.
class Backend
{
public:
    virtual ~Backend() = default;

    // Blocking; called once before the mount is made.
    virtual ConnectResult connect(const Credentials &credentials) = 0;
    // The folder a location opens in, relative to the mount root, such as the remote home.
    virtual std::string initialPath() const
    {
        return {};
    }
    // Mounted read only, with contents that never change under the kernel's caches.
    virtual bool readOnly() const
    {
        return false;
    }

    virtual int stat(const std::string &path, struct stat *st) = 0;
    virtual int list(const std::string &path, std::vector<DirectoryEntry> *entries) = 0;

    // flags as open(2); O_CREAT makes the file with mode.
    virtual int open(const std::string &path, int flags, mode_t mode, uint64_t *handle) = 0;
    virtual int read(uint64_t handle, char *buffer, size_t size, off_t offset) = 0;
    virtual int write(uint64_t handle, const char *buffer, size_t size, off_t offset) = 0;
    // Writes back what the handle holds; called on close(2) and fsync(2).
    virtual int flush(uint64_t handle)
    {
        (void)handle;
        return 0;
    }
    virtual int release(uint64_t handle) = 0;
    virtual int truncate(const std::string &path, off_t size, const uint64_t *handle) = 0;

    virtual int remove(const std::string &path) = 0;
    virtual int makeDirectory(const std::string &path, mode_t mode) = 0;
    virtual int removeDirectory(const std::string &path) = 0;
    // flags as renameat2(2): RENAME_NOREPLACE, RENAME_EXCHANGE.
    virtual int rename(const std::string &from, const std::string &to, unsigned flags) = 0;

    // A remote end without modes or settable times ignores them: copies set both and
    // would fail otherwise.
    virtual int setMode(const std::string &path, mode_t mode)
    {
        (void)path;
        (void)mode;
        return 0;
    }
    virtual int setTimes(const std::string &path, const struct timespec times[2])
    {
        (void)path;
        (void)times;
        return 0;
    }
    virtual int readLink(const std::string &path, std::string *target)
    {
        (void)path;
        (void)target;
        return -EINVAL;
    }
    virtual int makeLink(const std::string &target, const std::string &path)
    {
        (void)target;
        (void)path;
        return -EPERM;
    }
    virtual int statfs(const std::string &path, struct statvfs *st) = 0;
};
