// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "backend.h"
#include "diskimage.h"
#include "hfsplus.h"

#include <map>
#include <memory>

// The HFS+ volume in a disk image (.dmg), read only.
class DiskImageBackend : public Backend
{
public:
    explicit DiskImageBackend(const std::string &path);
    ~DiskImageBackend() override;

    ConnectResult connect(const Credentials &credentials) override;
    // The volume's own name, once connected.
    std::string volumeName() const;

    bool readOnly() const override
    {
        return true;
    }

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
    void fillStat(const HfsPlus::Item &item, struct stat *st) const;

    std::string m_path;
    DiskImage m_image;
    std::unique_ptr<HfsPlus> m_volume;
    std::map<uint64_t, std::unique_ptr<HfsPlus::File>> m_files;
    uint64_t m_nextHandle = 1;
};
