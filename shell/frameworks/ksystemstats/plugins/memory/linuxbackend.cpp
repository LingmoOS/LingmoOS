
/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "linuxbackend.h"

#include <systemstats/SensorObject.h>
#include <systemstats/SensorProperty.h>

#include <QFile>

LinuxMemoryBackend::LinuxMemoryBackend(KSysGuard::SensorContainer *container)
    : MemoryBackend(container)
{
}

void LinuxMemoryBackend::update()
{
    if (!m_physicalObject->isSubscribed() && !m_swapObject->isSubscribed()) {
        return;
    }

    QFile meminfo(QStringLiteral("/proc/meminfo"));
    meminfo.open(QIODevice::ReadOnly);
    // The format of the file is as follows:
    // Fieldname:[whitespace]value kB
    // A description of the fields can be found at 
    // https://www.kernel.org/doc/html/latest/filesystems/proc.html#meminfo
    unsigned long long total, free, available, buffer, cache, slab, swapTotal, swapFree;
    for (QByteArray line = meminfo.readLine(); !line.isEmpty(); line = meminfo.readLine()) {
        int colonIndex = line.indexOf(':');
        const auto fields = line.simplified().split(' ');

        const QByteArray name = line.left(colonIndex);
        const unsigned long long value = std::strtoull(line.mid(colonIndex + 1), nullptr, 10) * 1024;
        if (name == "MemTotal") {
            total = value;
        } else if (name == "MemFree") {
            free = value;
        } else if (name == "MemAvailable") {
            available = value;
        } else if (name == "Buffers") {
            buffer = value;
        } else if (name == "Cached") {
            cache = value;
        } else if (name == "Slab") {
            slab = value;
        } else if (name == "SwapTotal") {
            swapTotal = value;
        } else if (name == "SwapFree") {
            swapFree = value;
        }
    }
    m_total->setValue(total);
    m_used->setValue(total - available);
    m_free->setValue(available);
    m_application->setValue(total - free - cache - buffer - slab);
    m_cache->setValue(cache + slab);
    m_buffer->setValue(buffer);
    m_swapTotal->setValue(swapTotal);
    m_swapUsed->setValue(swapTotal - swapFree);
    m_swapFree->setValue(swapFree);
}

