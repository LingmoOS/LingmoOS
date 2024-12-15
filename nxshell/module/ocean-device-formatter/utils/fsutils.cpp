// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fsutils.h"

#include <ddiskmanager.h>

int FsUtils::maxLabelLength(const QString &fs)
{
    if (fs == "vfat") {
        // man 8 mkfs.fat
        return 11;
    }
    if (fs.startsWith("ext")) {
        // man 8 mke2fs
        return 16;
    }
    if (fs == "btrfs") {
        // https://btrfs.wiki.kernel.org/index.php/Manpage/btrfs-filesystem
        return 255;
    }
    if (fs == "f2fs") {
        // https://www.kernel.org/doc/Documentation/filesystems/f2fs.txt
        // https://git.kernel.org/pub/scm/linux/kernel/git/jaegeuk/f2fs-tools.git/tree/mkfs/f2fs_format_main.c
        return 512;
    }
    if (fs == "jfs") {
        // jfsutils/mkfs/mkfs.c:730
        return 16;
    }
    if (fs == "exfat") {
        // man 8 mkexfatfs
        return 15;
    }
    if (fs == "nilfs2") {
        // man 8 mkfs.nilfs2
        return 80;
    }
    if (fs == "ntfs") {
        // https://docs.microsoft.com/en-us/dotnet/api/system.io.driveinfo.volumelabel?view=netframework-4.8
        return 32;
    }
    if (fs == "reiserfs") {
        // man 8 mkreiserfs
        return 16;
    }
    if (fs == "reiser4") {
        // https://github.com/edward6/reiser4progs/blob/master/include/reiser4/types.h fs_hint_t
        return 16;
    }
    if (fs == "xfs") {
        // man 8 mkfs.xfs
        return 12;
    }
    return -1;
}

QStringList FsUtils::supportedFilesystems()
{
    QStringList ret;
    DDiskManager diskmgr;
    for (auto &fs : diskmgr.supportedFilesystems()) {
        if (diskmgr.canFormat(fs) && !fs.contains("minix")) { // 底层目前不支持minix，格式化的时候UDISK2模块会一直阻塞
            ret.push_back(fs);
        }
    }
    return ret;
}
