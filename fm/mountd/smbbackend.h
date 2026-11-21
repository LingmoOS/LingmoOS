// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "backend.h"

#include "remoteurl.h"

typedef struct _SMBCCTX SMBCCTX;

// A server's shares, one folder each at the top of the mount.
class SmbBackend : public Backend
{
public:
    explicit SmbBackend(const RemoteUrl &url);
    ~SmbBackend() override;

    ConnectResult connect(const Credentials &credentials) override;
    std::string initialPath() const override;

    int stat(const std::string &path, struct stat *st) override;
    int list(const std::string &path, std::vector<DirectoryEntry> *entries) override;
    int open(const std::string &path, int flags, mode_t mode, uint64_t *handle) override;
    int read(uint64_t handle, char *buffer, size_t size, off_t offset) override;
    int write(uint64_t handle, const char *buffer, size_t size, off_t offset) override;
    int release(uint64_t handle) override;
    int truncate(const std::string &path, off_t size, const uint64_t *handle) override;
    int remove(const std::string &path) override;
    int makeDirectory(const std::string &path, mode_t mode) override;
    int removeDirectory(const std::string &path) override;
    int rename(const std::string &from, const std::string &to, unsigned flags) override;
    int setTimes(const std::string &path, const struct timespec times[2]) override;
    int statfs(const std::string &path, struct statvfs *st) override;

private:
    static void authenticate(SMBCCTX *context, const char *server, const char *share,
                             char *workgroup, int workgroupLength, char *user, int userLength,
                             char *password, int passwordLength);

    SMBCCTX *createContext(const Credentials &credentials);
    std::string urlFor(const std::string &path) const;
    // The top of the mount and the shares on it, which have no files of their own there.
    static bool isRoot(const std::string &path);
    static bool isShare(const std::string &path);
    int listShares(std::vector<DirectoryEntry> *entries);
    void fillStat(struct stat *st) const;

    RemoteUrl m_url;
    Credentials m_credentials;
    SMBCCTX *m_context = nullptr;
    uid_t m_uid;
    gid_t m_gid;
};
