// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskreader.h"
#include "fileutil.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

DiskReader::DiskReader(std::string procRoot, std::string sysRoot)
    : m_procRoot(std::move(procRoot))
    , m_sysRoot(std::move(sysRoot))
{
}

void DiskReader::sample(Metrics* metrics, double elapsed)
{
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> disks;
    std::vector<Disk> devices;
    double diskRead = 0;
    double diskWrite = 0;
    const std::string statistics = readFile(m_procRoot + "/diskstats");
    for (const std::string_view line : split(statistics, '\n')) {
        const auto values = splitFields(line);
        if (values.size() < 14)
            continue;
        const std::string name = validUtf8(values[2]);
        const std::string path = m_sysRoot + "/block/" + name;
        if (!fileExists(path + "/device"))
            continue;
        const uint64_t readBytes = toUnsigned(values[5]) * 512;
        const uint64_t writeBytes = toUnsigned(values[9]) * 512;
        disks[name] = {readBytes, writeBytes};
        const auto previous = m_previous.find(name);
        const bool known = previous != m_previous.end();
        Disk disk;
        disk.name = name;
        disk.model = validUtf8(trimmed(readFile(path + "/device/model")));
        disk.read = known ? byteRate(readBytes, previous->second.first, elapsed) : 0;
        disk.write = known ? byteRate(writeBytes, previous->second.second, elapsed) : 0;
        disk.size = toUnsigned(trimmed(readFile(path + "/size"))) * 512;
        diskRead += disk.read;
        diskWrite += disk.write;
        devices.push_back(std::move(disk));
    }
    metrics->diskRead = diskRead;
    metrics->diskWrite = diskWrite;
    metrics->disks = std::move(devices);
    m_previous = std::move(disks);
}

std::string DiskReader::unescapePath(std::string_view path)
{
    std::string result;
    for (size_t i = 0; i < path.size(); ++i) {
        const auto octal = [&path](size_t index) { return index < path.size() && path[index] >= '0' && path[index] <= '7'; };
        if (path[i] == '\\' && octal(i + 1) && octal(i + 2) && octal(i + 3)) {
            result += char((path[i + 1] - '0') * 64 + (path[i + 2] - '0') * 8 + (path[i + 3] - '0'));
            i += 3;
        } else {
            result += path[i];
        }
    }
    return result;
}

static bool isWithin(std::string_view parent, std::string_view path)
{
    return path == parent || (path.starts_with(parent) && path[parent.size()] == '/');
}

// The mount a path is on now, which a later mount over it changes.
static uint64_t mountId(const std::string& path)
{
    struct statx info;
    if (statx(AT_FDCWD, path.c_str(), AT_NO_AUTOMOUNT | AT_STATX_DONT_SYNC, STATX_MNT_ID, &info) != 0
        || !(info.stx_mask & STATX_MNT_ID))
        return 0;
    return info.stx_mnt_id;
}

std::vector<Volume> DiskReader::volumes(const std::string& mountInfo)
{
    struct Mount
    {
        uint64_t id = 0;
        std::string point;
        std::string type;
        std::string device;
    };

    // The file systems a user would look for: none under /dev, /proc, /sys,
    // /var/run or /var/lock, no initrd rootfs, nothing hidden under a later mount.
    std::vector<Mount> mounts;
    const std::string contents = readFile(mountInfo);
    for (const std::string_view line : split(contents, '\n')) {
        const auto fields = split(line, ' ');
        size_t separator = 6;
        while (separator < fields.size() && fields[separator] != "-")
            ++separator;
        if (separator + 2 >= fields.size())
            continue;
        Mount mount;
        mount.id = toUnsigned(fields[0]);
        mount.point = unescapePath(fields[4]);
        mount.type = std::string(fields[separator + 1]);
        mount.device = unescapePath(fields[separator + 2]);
        bool special = mount.type == "rootfs";
        for (const std::string_view parent : {"/dev", "/proc", "/sys", "/var/run", "/var/lock"})
            special = special || isWithin(parent, mount.point);
        if (!special)
            mounts.push_back(std::move(mount));
    }

    std::vector<Volume> volumes;
    for (size_t index = 0; index < mounts.size(); ++index) {
        const Mount& mount = mounts[index];
        const uint64_t id = mountId(mount.point);
        if (id == 0) {
            bool covered = false;
            for (size_t later = index + 1; later < mounts.size() && !covered; ++later)
                covered = isWithin(mounts[later].point, mount.point);
            if (covered)
                continue;
        } else if (id != mount.id) {
            continue;
        }

        struct statvfs info;
        if (statvfs(mount.point.c_str(), &info) != 0)
            continue;
        const uint64_t total = uint64_t(info.f_blocks) * info.f_frsize;
        if (total == 0)
            continue;
        if (mount.type == "tmpfs" || mount.type == "devtmpfs" || mount.type == "squashfs"
            || mount.point.starts_with("/proc") || mount.point.starts_with("/sys"))
            continue;
        Volume volume;
        volume.mount = validUtf8(mount.point);
        volume.device = validUtf8(mount.device);
        volume.type = validUtf8(mount.type);
        volume.total = total;
        volume.available = uint64_t(info.f_bavail) * info.f_frsize;
        volume.used = total - uint64_t(info.f_bfree) * info.f_frsize;
        volumes.push_back(std::move(volume));
    }
    return volumes;
}
