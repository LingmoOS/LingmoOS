// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskimagebackend.h"

#include "base/i18n.h"

#include <climits>
#include <fcntl.h>
#include <unistd.h>

using Lingmo::i18n::tr;

DiskImageBackend::DiskImageBackend(const std::string &path)
    : m_path(path)
{
}

DiskImageBackend::~DiskImageBackend() = default;

ConnectResult DiskImageBackend::connect(const Credentials &)
{
    ConnectResult result;
    if (!m_image.open(m_path, &result.message))
        return result;

    bool apfs = false;
    for (const DiskImage::Volume &volume : m_image.volumes()) {
        if (volume.format == DiskImage::Format::Apfs) {
            apfs = true;
            continue;
        }

        auto hfs = std::make_unique<HfsPlus>(&m_image, volume.offset, volume.size);
        if (!hfs->open(&result.message))
            return result;
        m_volume = std::move(hfs);
        result.status = ConnectResult::Connected;
        return result;
    }

    result.message = apfs ? tr("Disk images with an APFS volume cannot be opened yet.")
                          : tr("The disk image holds no volume that can be opened.");
    return result;
}

std::string DiskImageBackend::volumeName() const
{
    return m_volume ? m_volume->name() : std::string();
}

void DiskImageBackend::fillStat(const HfsPlus::Item &item, struct stat *st) const
{
    *st = {};
    st->st_ino = item.id;
    st->st_mode = item.mode;
    st->st_nlink = item.folder ? 1 : item.links;
    // Owners on the disk are accounts of the machine that made it.
    st->st_uid = ::getuid();
    st->st_gid = ::getgid();
    st->st_size = item.folder ? 0 : off_t(item.size);
    st->st_blksize = 4096;
    st->st_blocks = blkcnt_t((item.size + 511) / 512);
    st->st_mtim.tv_sec = item.modified;
    st->st_ctim.tv_sec = item.changed;
    st->st_atim.tv_sec = item.accessed;
}

int DiskImageBackend::stat(const std::string &path, struct stat *st)
{
    HfsPlus::Item item;
    if (const int error = m_volume->lookup(path, &item); error < 0)
        return error;
    fillStat(item, st);
    return 0;
}

int DiskImageBackend::list(const std::string &path, std::vector<DirectoryEntry> *entries)
{
    HfsPlus::Item folder;
    if (const int error = m_volume->lookup(path, &folder); error < 0)
        return error;

    std::vector<std::pair<std::string, HfsPlus::Item>> children;
    if (const int error = m_volume->list(folder, &children); error < 0)
        return error;

    entries->reserve(children.size());
    for (const auto &[name, item] : children) {
        DirectoryEntry entry;
        entry.name = name;
        fillStat(item, &entry.st);
        entries->push_back(std::move(entry));
    }
    return 0;
}

int DiskImageBackend::open(const std::string &path, int flags, mode_t, uint64_t *handle)
{
    if ((flags & O_ACCMODE) != O_RDONLY || (flags & (O_CREAT | O_TRUNC)))
        return -EROFS;

    HfsPlus::Item item;
    if (const int error = m_volume->lookup(path, &item); error < 0)
        return error;

    std::unique_ptr<HfsPlus::File> file;
    if (const int error = m_volume->openFile(item, &file); error < 0)
        return error;

    *handle = m_nextHandle++;
    m_files.emplace(*handle, std::move(file));
    return 0;
}

int DiskImageBackend::read(uint64_t handle, char *buffer, size_t size, off_t offset)
{
    const auto found = m_files.find(handle);
    if (found == m_files.end())
        return -EBADF;
    if (offset < 0)
        return -EINVAL;
    return found->second->read(buffer, std::min<size_t>(size, INT_MAX), uint64_t(offset));
}

int DiskImageBackend::write(uint64_t, const char *, size_t, off_t)
{
    return -EROFS;
}

int DiskImageBackend::release(uint64_t handle)
{
    m_files.erase(handle);
    return 0;
}

int DiskImageBackend::truncate(const std::string &, off_t, const uint64_t *)
{
    return -EROFS;
}

int DiskImageBackend::remove(const std::string &)
{
    return -EROFS;
}

int DiskImageBackend::makeDirectory(const std::string &, mode_t)
{
    return -EROFS;
}

int DiskImageBackend::removeDirectory(const std::string &)
{
    return -EROFS;
}

int DiskImageBackend::rename(const std::string &, const std::string &, unsigned)
{
    return -EROFS;
}

int DiskImageBackend::setMode(const std::string &, mode_t)
{
    return -EROFS;
}

int DiskImageBackend::setTimes(const std::string &, const struct timespec[2])
{
    return -EROFS;
}

int DiskImageBackend::readLink(const std::string &path, std::string *target)
{
    HfsPlus::Item item;
    if (const int error = m_volume->lookup(path, &item); error < 0)
        return error;
    return m_volume->readLink(item, target);
}

int DiskImageBackend::makeLink(const std::string &, const std::string &)
{
    return -EROFS;
}

int DiskImageBackend::statfs(const std::string &, struct statvfs *st)
{
    *st = {};
    st->f_bsize = m_volume->blockSize();
    st->f_frsize = m_volume->blockSize();
    st->f_blocks = m_volume->totalBlocks();
    st->f_bfree = m_volume->freeBlocks();
    st->f_files = m_volume->fileCount();
    st->f_flag = ST_RDONLY;
    st->f_namemax = 255;
    return 0;
}
