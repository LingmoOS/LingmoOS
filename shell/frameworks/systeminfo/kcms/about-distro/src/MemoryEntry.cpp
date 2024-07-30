/*
    SPDX-FileCopyrightText: 2012-2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "MemoryEntry.h"

#include <KFormat>

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#elif defined(Q_OS_FREEBSD)
// clang-format off
#include <sys/types.h>
#include <sys/sysctl.h>
// clang-format on
#endif

MemoryEntry::MemoryEntry()
    : Entry(ki18n("Memory:"), QString() /* overridden here */)
{
}

qlonglong MemoryEntry::calculateTotalRam()
{
    qlonglong ret = -1;
#ifdef Q_OS_LINUX
    struct sysinfo info;
    if (sysinfo(&info) == 0)
        // manpage "sizes are given as multiples of mem_unit bytes"
        ret = qlonglong(info.totalram) * info.mem_unit;
#elif defined(Q_OS_FREEBSD)
    /* Stuff for sysctl */
    size_t len;

    unsigned long memory;
    len = sizeof(memory);
    sysctlbyname("hw.physmem", &memory, &len, NULL, 0);

    ret = memory;
#endif
    return ret;
}

QString MemoryEntry::localizedValue(Language language) const
{
    const qlonglong totalRam = calculateTotalRam();
    if (totalRam > 0) {
        const auto string = ki18nc("@label %1 is the formatted amount of system memory (e.g. 7,7 GiB)", "%1 of RAM")
                                .subs(KFormat(localeForLanguage(language)).formatByteSize(totalRam));
        return localize(string, language);
    }
    return localize(ki18nc("Unknown amount of RAM", "Unknown"), language);
}
