/*
    SPDX-FileCopyrightText: 2010 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fsutils.h"
#include "enginedebug.h"

#ifdef Q_OS_LINUX
#include <cerrno>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

using namespace Baloo;

void FSUtils::disableCoW(const QString &path)
{
#ifndef Q_OS_LINUX
    Q_UNUSED(path);
#else
    // from linux/fs.h, so that Baloo does not depend on Linux header files
#ifndef FS_IOC_GETFLAGS
#define FS_IOC_GETFLAGS     _IOR('f', 1, long)
#endif
#ifndef FS_IOC_SETFLAGS
#define FS_IOC_SETFLAGS     _IOW('f', 2, long)
#endif

    // Disable COW on file
#ifndef FS_NOCOW_FL
#define FS_NOCOW_FL         0x00800000
#endif

    ulong flags = 0;
    const int fd = open(qPrintable(path), O_RDONLY);
    if (fd == -1) {
        qCWarning(ENGINE) << "Failed to open" << path << "to modify flags (" << errno << ")";
        return;
    }

    if (ioctl(fd, FS_IOC_GETFLAGS, &flags) == -1) {
        const int errno_ioctl = errno;
        // ignore ENOTTY, filesystem does not support attrs (and likely neither supports COW)
        if (errno_ioctl != ENOTTY) {
            qCWarning(ENGINE) << "ioctl error: failed to get file flags (" << errno_ioctl << ")";
        }
        close(fd);
        return;
    }
    if (!(flags & FS_NOCOW_FL)) {
        flags |= FS_NOCOW_FL;
        if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == -1) {
            const int errno_ioctl = errno;
            // ignore EOPNOTSUPP, returned on filesystems not supporting COW
            if (errno_ioctl != EOPNOTSUPP) {
                qCWarning(ENGINE) << "ioctl error: failed to set file flags (" << errno_ioctl << ")";
            }
            close(fd);
            return;
        }
    }
    close(fd);
#endif
}
