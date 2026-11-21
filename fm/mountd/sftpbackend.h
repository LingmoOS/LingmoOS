// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "backend.h"

#include "remoteurl.h"

typedef struct ssh_session_struct *ssh_session;
typedef struct sftp_session_struct *sftp_session;

class SftpBackend : public Backend
{
public:
    explicit SftpBackend(const RemoteUrl &url);
    ~SftpBackend() override;

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
    int setMode(const std::string &path, mode_t mode) override;
    int setTimes(const std::string &path, const struct timespec times[2]) override;
    int readLink(const std::string &path, std::string *target) override;
    int makeLink(const std::string &target, const std::string &path) override;
    int statfs(const std::string &path, struct statvfs *st) override;

private:
    ConnectResult establish(const Credentials &credentials);
    void close();
    // Connects again after the link dropped, with the credentials that worked before.
    bool ensureConnected();
    int error(int fallback = EIO);

    RemoteUrl m_url;
    Credentials m_credentials;
    ssh_session m_ssh = nullptr;
    sftp_session m_sftp = nullptr;
    std::string m_home;
    size_t m_readChunk = 32768;
    size_t m_writeChunk = 32768;
    // Handles opened before a reconnect belong to the old session.
    uint64_t m_generation = 0;
};
