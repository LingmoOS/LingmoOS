// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "mountinfo.h"
#include "utils.h"
#include "fsinfo.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

#include <limits.h>
#include <mntent.h>

namespace DiskManager {
static MountInfo::MountMapping mountInfo;
static MountInfo::MountMapping fstabInfo;

void MountInfo::loadCache(QString &rootFs)
{
    mountInfo.clear();
    fstabInfo.clear();

    readMountpointsFromFile("/proc/mounts", mountInfo, rootFs);
    readMountpointsFromFileSwaps("/proc/swaps", mountInfo);

    if (!haveRootfsDev(mountInfo))
        // Old distributions only contain 'rootfs' and '/dev/root' device names
        // for the / (root) file system in /proc/mounts with '/dev/root' being a
        // block device rather than a symlink to the true device.  This prevents
        // identification, and therefore busy detection, of the device containing
        // the / (root) file system.  Used to read /etc/mtab to get the root file
        // system device name, but this contains an out of date device name after
        // the mounting device has been dynamically removed from a multi-device
        // btrfs, thus identifying the wrong device as busy.  Instead fall back
        // to reading mounted file systems from the output of the mount command,
        // but only when required.
        readMountpointsFromMountCommand(mountInfo);

    readMountpointsFromFile("/etc/fstab", fstabInfo, rootFs);

    // Sort the mount points and remove duplicates ... (no need to do this for fstab_info)
    MountMapping::iterator iterMp;
    for (iterMp = mountInfo.begin(); iterMp != mountInfo.end(); ++iterMp) {
        std::sort(iterMp.value().mountpoints.begin(), iterMp.value().mountpoints.end());

        iterMp.value().mountpoints.erase(
            std::unique(iterMp.value().mountpoints.begin(), iterMp.value().mountpoints.end()),
            iterMp.value().mountpoints.end());
    }
}

bool MountInfo::isDevMounted(const QString &path)
{
    return isDevMounted(BlockSpecial(path));
}

bool MountInfo::isDevMounted(const BlockSpecial &blockSpecial)
{
    MountMapping::const_iterator iterMp = mountInfo.find(blockSpecial);
    return iterMp != mountInfo.end();
}

bool MountInfo::isDevMountedReadonly(const QString &path)
{
    return isDevMountedReadonly(BlockSpecial(path));
}

bool MountInfo::isDevMountedReadonly(const BlockSpecial &blockSpecial)
{
    MountMapping::const_iterator iterMp = mountInfo.find(blockSpecial);
    if (iterMp == mountInfo.end()) {
        return false;
    }
    return iterMp.value().readonly;
}

const QVector<QString> &MountInfo::getMountedMountpoints(const QString &path)
{
    return find(mountInfo, path).mountpoints;
}

const QVector<QString> &MountInfo::getFileSystemTableMountpoints(const QString &path)
{
    return find(fstabInfo, path).mountpoints;
}

void MountInfo::readMountpointsFromFile(const QString &fileName, MountInfo::MountMapping &map, QString &rootFs)
{
    FILE *fp = setmntent(fileName.toStdString().c_str(), "r");
    if (fp == nullptr) {
        return;
    }
    struct mntent *p = nullptr;
    while ((p = getmntent(fp)) != nullptr) {
        QString node = p->mnt_fsname;
        QString mountpoint = p->mnt_dir;
        qDebug() << __FUNCTION__ << p->mnt_dir << " FsName: " << p->mnt_fsname;
        if (0 == rootFs.length() &&
                (0 == strcmp(p->mnt_dir, "/root")
                 || 0 == strcmp(p->mnt_dir, "/home")
                 || 0 == strcmp(p->mnt_dir, "/opt")
                 || 0 == strcmp(p->mnt_dir, "/var"))) {
            rootFs = p->mnt_fsname;
            qDebug() <<"Set RootFS:" << rootFs;
        }

        QString uuid = Utils::regexpLabel(node, "(?<=UUID\\=).*");
        if (!uuid.isEmpty()) {
            node = FsInfo::getPathByUuid(uuid);
        }

        QString label = Utils::regexpLabel(node, "(?<=UUID\\=).*");
        if (!label.isEmpty()) {
            node = FsInfo::getPathByLabel(label);
        }

        if (!node.isEmpty()) {
            addMountpointEntry(map, node, mountpoint, parseReadonlyFlag(p->mnt_opts));
        }
    }

    endmntent(fp);
}

void MountInfo::addMountpointEntry(MountInfo::MountMapping &map, QString &node, QString &mountPoint, bool readonly)
{
    // Only add node path if mount point exists
    QFile file(mountPoint);
    if (file.exists()) {
        // Map::operator[] default constructs MountEntry for new keys (nodes).
        MountEntry &mountentry = map[BlockSpecial(node)];
        mountentry.readonly = mountentry.readonly || readonly;
        mountentry.mountpoints.push_back(mountPoint);
    }
}

bool MountInfo::parseReadonlyFlag(const QString &str)
{
    QStringList mntopts = str.split(",");
    for (int i = 0; i < mntopts.size(); i++) {
        if (mntopts[i] == "rw") {
            return false;
        }
        else if (mntopts[i] == "ro") {
            return true;
        }
    }
    return false; // Default is read-write mount
}

void MountInfo::readMountpointsFromFileSwaps(const QString &fileName, MountInfo::MountMapping &map)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine();
        QString node;
        while (!in.atEnd() || !line.isEmpty()) {
//            qDebug() << line;
            node = Utils::regexpLabel(line, "^(/[^ ]+)");

            if (node.size() > 0) {
                map[BlockSpecial(node)].mountpoints.push_back("" /* no mountpoint for swap */);
            }
            line = in.readLine();
        }
    }
}

bool MountInfo::haveRootfsDev(MountInfo::MountMapping &)
{
    MountMapping::const_iterator iterMp;
    for (iterMp = mountInfo.begin(); iterMp != mountInfo.end(); iterMp++) {
        if (!iterMp.value().mountpoints.isEmpty() && iterMp.value().mountpoints[0] == "/") {
            if (iterMp.key().m_name != "rootfs" && iterMp.key().m_name != "/dev/root") {
                return true;
            }
        }
    }
    return false;
}

void MountInfo::readMountpointsFromMountCommand(MountInfo::MountMapping &map)
{
    QString output;
    QString error;
    if (!Utils::executCmd("mount", output, error)) {
        QStringList lines;
        lines = output.split("\n");
        for (int i = 0; i < lines.size(); i++) {
            // Process line like "/dev/sda3 on / type ext4 (rw)"
            QString node = Utils::regexpLabel(lines[i], ".*?(?= )");
            QString mountpoint = Utils::regexpLabel(lines[i], "(?<=on ).*?(?= type)");
            QString mntopts = Utils::regexpLabel(lines[i], "(?<=\\().*?(?=\\))");
            // qDebug() << node << mountpoint << mntopts;
            if (!node.isEmpty()) {
                addMountpointEntry(map, node, mountpoint, parseReadonlyFlag(mntopts));
            }
        }
    }
}

const MountEntry &MountInfo::find(const MountInfo::MountMapping &map, const QString &path)
{
    MountMapping::const_iterator iterMp = map.find(BlockSpecial(path));

    if (iterMp != map.end()) {
        return iterMp.value();
    }

    static MountEntry not_mounted = MountEntry();
    return not_mounted;
}

} // namespace DiskManager
