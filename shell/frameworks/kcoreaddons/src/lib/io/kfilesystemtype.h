/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2011 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#ifndef KFILESYSTEMTYPE_P_H
#define KFILESYSTEMTYPE_P_H

#include <kcoreaddons_export.h>

#include <QString>

/**
 * @namespace KFileSystemType
 * Provides utility functions for the type of file systems.
 */
namespace KFileSystemType
{
enum Type {
    Unknown,
    Nfs, ///< NFS or other full-featured networked filesystems (autofs, subfs, cachefs, sshfs)
    Smb, ///< SMB/CIFS mount (networked but with some FAT-like behavior)
    Fat, ///< FAT or similar (msdos, FAT, VFAT)
    Ramfs, ///< RAMDISK mount
    Other, ///< Ext3, Ext4, ReiserFs, and so on. "Normal" local filesystems.
    Ntfs, ///< NTFS filesystem @since 5.85
    Exfat, ///< ExFat filesystem @since 5.86
    /**
     * FUSE (Filesystem in USErspace), this is used for a variety of underlying
     * filesystems.
     *
     * @since 5.100
     */
    Fuse,
};

/**
 * For a given @p path, returns the filesystem type, one of @ref KFileSystemType::Type
 * values. If the type can't be determined, @c KFileSystemType::Unknown is returned.
 *
 * @since 5.0
 */
KCOREADDONS_EXPORT Type fileSystemType(const QString &path);

/**
 * Returns the possibly translated name of a filesystem corresponding to a
 * value from @ref KFileSystemType::Type.
 *
 * @since 5.86
 */
KCOREADDONS_EXPORT QString fileSystemName(KFileSystemType::Type type);
}

#endif
