// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "backend.h"

#include <map>

typedef struct LIBMTP_mtpdevice_struct LIBMTP_mtpdevice_t;

// A phone or player over MTP: one folder per storage at the top of the mount.
class MtpBackend : public Backend
{
public:
    struct OpenFile;

    MtpBackend(uint32_t bus, uint8_t device);
    ~MtpBackend() override;

    ConnectResult connect(const Credentials &credentials) override;
    // The device's own name, once connected.
    std::string friendlyName() const;

    int stat(const std::string &path, struct stat *st) override;
    int list(const std::string &path, std::vector<DirectoryEntry> *entries) override;
    int open(const std::string &path, int flags, mode_t mode, uint64_t *handle) override;
    int read(uint64_t handle, char *buffer, size_t size, off_t offset) override;
    int write(uint64_t handle, const char *buffer, size_t size, off_t offset) override;
    int flush(uint64_t handle) override;
    int release(uint64_t handle) override;
    int truncate(const std::string &path, off_t size, const uint64_t *handle) override;
    int remove(const std::string &path) override;
    int makeDirectory(const std::string &path, mode_t mode) override;
    int removeDirectory(const std::string &path) override;
    int rename(const std::string &from, const std::string &to, unsigned flags) override;
    int statfs(const std::string &path, struct statvfs *st) override;

private:
    struct Object
    {
        uint32_t storage = 0;
        // 0xFFFFFFFF for a storage's top folder.
        uint32_t id = 0;
        bool folder = false;
        uint64_t size = 0;
        time_t modified = 0;
    };

    void loadStorages();
    // The object at path, listing the folders on the way as needed.
    bool lookup(const std::string &path, Object *object);
    int listFolder(const std::string &path, const Object &folder,
                   std::vector<DirectoryEntry> *entries);
    void forget(const std::string &path);
    int upload(OpenFile *file);
    int download(const Object &object, int fd);
    int lastError(int fallback = EIO);
    void fillStat(const Object &object, struct stat *st) const;

    uint32_t m_bus;
    uint8_t m_deviceNumber;
    LIBMTP_mtpdevice_t *m_device = nullptr;
    bool m_partialRead = false;
    bool m_android = false;
    bool m_move = false;
    // Storage folder name to its id.
    std::map<std::string, uint32_t> m_storages;
    std::map<std::string, Object> m_cache;
    // Files being written, by path, so they are seen before they reach the device.
    std::map<std::string, OpenFile *> m_writing;
    time_t m_connected = 0;
};
