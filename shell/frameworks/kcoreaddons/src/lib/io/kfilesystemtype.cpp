/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2011 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#include "kfilesystemtype.h"
#include "knetworkmounts.h"

#include <config-kfilesystemtype.h>
#if HAVE_UDEV
#include <libudev.h>
#endif

#include <QCoreApplication>
#include <QFile>

#include <array>

struct FsInfo {
    KFileSystemType::Type type = KFileSystemType::Unknown;
    const char *name = nullptr;
};

#ifndef Q_OS_WIN
// clang-format off
static const std::array<FsInfo, 19> s_fsMap = {{
    {KFileSystemType::Nfs, "nfs"},
    {KFileSystemType::Nfs, "nfs4"},
    {KFileSystemType::Smb, "smb"},
    {KFileSystemType::Fat, "fat"},
    {KFileSystemType::Ramfs, "ramfs"},
    {KFileSystemType::Other, "other"},
    {KFileSystemType::Ntfs, "ntfs"},
    {KFileSystemType::Ntfs, "ntfs3"},
    {KFileSystemType::Exfat, "exfat"},
    {KFileSystemType::Unknown, "unknown"},
    {KFileSystemType::Nfs, "autofs"},
    {KFileSystemType::Nfs, "cachefs"},
    {KFileSystemType::Nfs, "fuse.sshfs"},
    {KFileSystemType::Nfs, "xtreemfs@"}, // #178678
    {KFileSystemType::Smb, "smbfs"},
    {KFileSystemType::Smb, "cifs"},
    {KFileSystemType::Fat, "vfat"},
    {KFileSystemType::Fat, "msdos"},
    {KFileSystemType::Fuse, "fuseblk"},
}};
// clang-format on

inline KFileSystemType::Type kde_typeFromName(const QLatin1String name)
{
    auto it = std::find_if(s_fsMap.cbegin(), s_fsMap.cend(), [name](const auto &fsInfo) {
        return QLatin1String(fsInfo.name) == name;
    });
    return it != s_fsMap.cend() ? it->type : KFileSystemType::Other;
}

inline KFileSystemType::Type kde_typeFromName(const char *c)
{
    return kde_typeFromName(QLatin1String(c));
}

#if defined(Q_OS_BSD4) && !defined(Q_OS_NETBSD)
#include <sys/mount.h>
#include <sys/param.h>

KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray &path)
{
    struct statfs buf;
    if (statfs(path.constData(), &buf) != 0) {
        return KFileSystemType::Unknown;
    }
    return kde_typeFromName(buf.f_fstypename);
}

#elif defined(Q_OS_LINUX) || defined(Q_OS_HURD)
#include <sys/statfs.h>

#ifdef Q_OS_LINUX
#include <linux/magic.h> // A lot of the filesystem superblock MAGIC numbers
#include <sys/stat.h>
#endif

// Add known missig magics
// Can use https://github.com/systemd/systemd/blob/main/src/basic/missing_magic.h
// as reference

// From linux/fs/ntfs/ntfs.h
#ifndef NTFS_SB_MAGIC
#define NTFS_SB_MAGIC 0x5346544e
#endif

// From linux/fs/ntfs3/super.c
#ifndef NTFS3_MAGIC
#define NTFS3_MAGIC 0x7366746E
#endif

// From linux/fs/exfat/exfat_fs.h
#ifndef EXFAT_SUPER_MAGIC
#define EXFAT_SUPER_MAGIC 0x2011BAB0UL
#endif

// From linux/fs/cifs/smb2glob.h
#ifndef SMB2_MAGIC_NUMBER
#define SMB2_MAGIC_NUMBER 0xFE534D42
#endif

// From linux/fs/cifs/cifsglob.h
#ifndef CIFS_MAGIC_NUMBER
#define CIFS_MAGIC_NUMBER 0xFF534D42
#endif

// From linux/fs/fuse/inode.c
#ifndef FUSE_SUPER_MAGIC
#define FUSE_SUPER_MAGIC 0x65735546
#endif

#ifndef AUTOFSNG_SUPER_MAGIC
#define AUTOFSNG_SUPER_MAGIC 0x7d92b1a0
#endif

#ifdef Q_OS_HURD
#ifndef NFS_SUPER_MAGIC
#define NFS_SUPER_MAGIC 0x00006969
#endif
#ifndef AUTOFS_SUPER_MAGIC
#define AUTOFS_SUPER_MAGIC 0x00000187
#endif
#ifndef MSDOS_SUPER_MAGIC
#define MSDOS_SUPER_MAGIC 0x00004d44
#endif
#ifndef SMB_SUPER_MAGIC
#define SMB_SUPER_MAGIC 0x0000517B
#endif
#ifndef RAMFS_MAGIC
#define RAMFS_MAGIC 0x858458F6
#endif
#endif

KFileSystemType::Type probeFuseBlkType(const QByteArray &path)
{
    using namespace KFileSystemType;

#if HAVE_UDEV
    struct stat buf;
    if (stat(path.constData(), &buf) != 0) {
        return Fuse;
    }

    using UdevPtr = std::unique_ptr<struct udev, decltype(&udev_unref)>;
    using UDevicePtr = std::unique_ptr<struct udev_device, decltype(&udev_device_unref)>;

    // Code originally copied from util-linux/misc-utils/lsblk.c
    auto udevP = UdevPtr(udev_new(), udev_unref);
    if (!udevP) {
        return Fuse;
    }

    // 'b' for block devices
    auto devPtr = UDevicePtr(udev_device_new_from_devnum(udevP.get(), 'b', buf.st_dev), udev_device_unref);
    if (!devPtr) {
        // If is not block device, assume conservatively it is a remote FS under FUSE.
        return Nfs;
    }

    const QLatin1String fsType(udev_device_get_property_value(devPtr.get(), "ID_FS_TYPE"));
    return kde_typeFromName(fsType);
#endif

    return Fuse;
}

// Reverse-engineering without C++ code:
// strace stat -f /mnt 2>&1|grep statfs|grep mnt, and look for f_type
//
// Also grep'ing in /usr/src/<kernel-version>/fs/

static KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray &path)
{
    struct statfs buf;
    if (statfs(path.constData(), &buf) != 0) {
        return KFileSystemType::Unknown;
    }

    switch (static_cast<unsigned long>(buf.f_type)) {
    case NFS_SUPER_MAGIC:
    case AUTOFS_SUPER_MAGIC:
    case AUTOFSNG_SUPER_MAGIC:
        return KFileSystemType::Nfs;
    case FUSE_SUPER_MAGIC:
        return probeFuseBlkType(path);
    case SMB_SUPER_MAGIC:
    case SMB2_MAGIC_NUMBER:
    case CIFS_MAGIC_NUMBER:
        return KFileSystemType::Smb;
    case MSDOS_SUPER_MAGIC:
        return KFileSystemType::Fat;
    case NTFS_SB_MAGIC:
    case NTFS3_MAGIC:
        return KFileSystemType::Ntfs;
    case EXFAT_SUPER_MAGIC:
        return KFileSystemType::Exfat;
    case RAMFS_MAGIC:
        return KFileSystemType::Ramfs;
    default:
        return KFileSystemType::Other;
    }
}

#elif defined(Q_OS_AIX) || defined(Q_OS_HPUX) || defined(Q_OS_QNX) || defined(Q_OS_SCO) || defined(Q_OS_UNIXWARE) || defined(Q_OS_RELIANT)                     \
    || defined(Q_OS_NETBSD)
#include <sys/statvfs.h>

KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray &path)
{
    struct statvfs buf;
    if (statvfs(path.constData(), &buf) != 0) {
        return KFileSystemType::Unknown;
    }
#if defined(Q_OS_NETBSD)
    return kde_typeFromName(buf.f_fstypename);
#else
    return kde_typeFromName(buf.f_basetype);
#endif
}
#endif
#else
KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray &path)
{
    Q_UNUSED(path);
    return KFileSystemType::Unknown;
}
#endif

KFileSystemType::Type KFileSystemType::fileSystemType(const QString &path)
{
    if (KNetworkMounts::self()->isSlowPath(path, KNetworkMounts::KNetworkMountsType::SmbPaths)) {
        return KFileSystemType::Smb;
    } else if (KNetworkMounts::self()->isSlowPath(path, KNetworkMounts::KNetworkMountsType::NfsPaths)) {
        return KFileSystemType::Nfs;
    } else {
        return determineFileSystemTypeImpl(QFile::encodeName(path));
    }
}

QString KFileSystemType::fileSystemName(KFileSystemType::Type type)
{
    switch (type) {
    case KFileSystemType::Nfs:
        return QCoreApplication::translate("KFileSystemType", "NFS");
    case KFileSystemType::Smb:
        return QCoreApplication::translate("KFileSystemType", "SMB");
    case KFileSystemType::Fat:
        return QCoreApplication::translate("KFileSystemType", "FAT");
    case KFileSystemType::Ramfs:
        return QCoreApplication::translate("KFileSystemType", "RAMFS");
    case KFileSystemType::Other:
        return QCoreApplication::translate("KFileSystemType", "Other");
    case KFileSystemType::Ntfs:
        return QCoreApplication::translate("KFileSystemType", "NTFS");
    case KFileSystemType::Exfat:
        return QCoreApplication::translate("KFileSystemType", "ExFAT");
    case KFileSystemType::Fuse:
        return QCoreApplication::translate("KFileSystemType", "FUSE");
    case KFileSystemType::Unknown:
        return QCoreApplication::translate("KFileSystemType", "Unknown");
    }

    Q_UNREACHABLE();
    return {};
}
