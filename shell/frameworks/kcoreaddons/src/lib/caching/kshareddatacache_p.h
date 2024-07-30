/*
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2010 Michael Pyne <mpyne@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSHAREDDATACACHE_P_H
#define KSHAREDDATACACHE_P_H

#include "kcoreaddons_debug.h"

#include <cerrno>
#include <fcntl.h>
#include <unistd.h> // for _POSIX_ADVISORY_INFO

// posix_fallocate is used to ensure that the file used for the cache is
// actually fully committed to disk before attempting to use the file.
#if defined(_POSIX_ADVISORY_INFO) && ((_POSIX_ADVISORY_INFO == 0) || (_POSIX_ADVISORY_INFO >= 200112L))
#define KSDC_POSIX_FALLOCATE_SUPPORTED 1
#endif
#ifdef Q_OS_OSX
#include "posix_fallocate_mac.h"
#define KSDC_POSIX_FALLOCATE_SUPPORTED 1
#endif

static bool ensureFileAllocated(int fd, size_t fileSize)
{
#ifdef KSDC_POSIX_FALLOCATE_SUPPORTED
    int result;
    while ((result = ::posix_fallocate(fd, 0, fileSize)) == EINTR) {
        ;
    }

    if (result != 0) {
        if (result == ENOSPC) {
            qCCritical(KCOREADDONS_DEBUG) << "No space left on device. Check filesystem free space at your XDG_CACHE_HOME!";
        }
        qCCritical(KCOREADDONS_DEBUG) << "The operating system is unable to promise" << fileSize
                                      << "bytes for mapped cache, "
                                         "abandoning the cache for crash-safety.";
        return false;
    }

    return true;
#else

#ifdef __GNUC__
#warning                                                                                                                                                       \
    "This system does not seem to support posix_fallocate, which is needed to ensure KSharedDataCache's underlying files are fully committed to disk to avoid crashes with low disk space."
#endif
    qCWarning(KCOREADDONS_DEBUG) << "This system misses support for posix_fallocate()"
                                    " -- ensure this partition has room for at least"
                                 << fileSize << "bytes.";

    // TODO: It's possible to emulate the functionality, but doing so
    // overwrites the data in the file so we don't do this. If you were to add
    // this emulation, you must ensure it only happens on initial creation of a
    // new file and not just mapping an existing cache.

    return true;
#endif // KSDC_POSIX_FALLOCATE_SUPPORTED
}

#endif /* KSHAREDDATACACHE_P_H */
