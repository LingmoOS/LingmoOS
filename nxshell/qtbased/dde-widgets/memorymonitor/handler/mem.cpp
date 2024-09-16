// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mem.h"
#include "mem_p.h"

#include <stdio.h>

#include <memory>

#include <QDebug>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>

#define PROC_PATH_MEM "/proc/meminfo"

inline void print_errno(decltype(errno) e, const QString &msg)
{
    qWarning() << QString("Error: [%1] %2, ").arg(e).arg(strerror(e)) << msg;
}

struct uFileClose {
    void operator()(FILE *fp)
    {
        fclose(fp);
    }
};
using uFile = std::unique_ptr<FILE, uFileClose>;

namespace core {
namespace system {

MemInfo::MemInfo()
    : d(new MemInfoPrivate())
{
}
MemInfo::MemInfo(const MemInfo &other)
    : d(other.d)
{
}
MemInfo &MemInfo::operator=(const MemInfo &rhs)
{
    if (this == &rhs)
        return *this;

    d = rhs.d;
    return *this;
}
MemInfo::~MemInfo()
{
}

qulonglong MemInfo::memTotal() const
{
    return d->mem_total_kb;
}

qulonglong MemInfo::memAvailable() const
{
    return d->mem_avail_kb;
}

qulonglong MemInfo::buffers() const
{
    return d->buffers_kb;
}

qulonglong MemInfo::cached() const
{
    return d->cached_kb;
}

qulonglong MemInfo::active() const
{
    return d->active_kb;
}

qulonglong MemInfo::inactive() const
{
    return d->inactive_kb;
}

qulonglong MemInfo::swapTotal() const
{
    return d->swap_total_kb;
}

qulonglong MemInfo::swapFree() const
{
    return d->swap_free_kb;
}

qulonglong MemInfo::swapCached() const
{
    return d->swap_cached_kb;
}

qulonglong MemInfo::shmem() const
{
    return d->shmem_kb;
}

qulonglong MemInfo::slab() const
{
    return d->slab_kb;
}

qulonglong MemInfo::dirty() const
{
    return d->dirty_kb;
}

qulonglong MemInfo::mapped() const
{
    return d->mapped_kb;
}

void MemInfo::readMemInfo()
{
    FILE *fp;

    if ((fp = fopen(PROC_PATH_MEM, "r"))) {
        uFile ufp(fp);
        Q_UNUSED(ufp);

        const size_t BUFLEN = 512;
        QByteArray line(BUFLEN, '\0');
        while (fgets(line.data(), BUFLEN, fp)) {
            if (!strncmp(line.data(), "MemTotal:", 9)) {
                int nr = sscanf(line.data() + 9, "%llu", &d->mem_total_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("MemTotal"));

            } else if (!strncmp(line.data(), "MemFree:", 8)) {
                int nr = sscanf(line.data() + 8, "%llu", &d->mem_free_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("MemFree"));

            } else if (!strncmp(line.data(), "MemAvailable:", 13)) {
                int nr = sscanf(line.data() + 13, "%llu", &d->mem_avail_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("MemAvailable"));

            } else if (!strncmp(line.data(), "Buffers:", 8)) {
                int nr = sscanf(line.data() + 8, "%llu", &d->buffers_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("Buffers"));

            } else if (!strncmp(line.data(), "Cached:", 7)) {
                int nr = sscanf(line.data() + 7, "%llu", &d->cached_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("Cached"));

            } else if (!strncmp(line.data(), "SwapCached:", 11)) {
                int nr = sscanf(line.data() + 11, "%llu", &d->swap_cached_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("SwapCached"));

            } else if (!strncmp(line.data(), "Active:", 7)) {
                int nr = sscanf(line.data() + 7, "%llu", &d->active_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("Active"));

            } else if (!strncmp(line.data(), "Inactive:", 9)) {
                int nr = sscanf(line.data() + 9, "%llu", &d->inactive_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("Inactive"));

            } else if (!strncmp(line.data(), "SwapTotal:", 10)) {
                int nr = sscanf(line.data() + 10, "%llu", &d->swap_total_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("SwapTotal"));

            } else if (!strncmp(line.data(), "SwapFree:", 9)) {
                int nr = sscanf(line.data() + 9, "%llu", &d->swap_free_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("SwapFree"));

            } else if (!strncmp(line.data(), "Dirty:", 6)) {
                int nr = sscanf(line.data() + 6, "%llu", &d->dirty_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("Dirty"));

            } else if (!strncmp(line.data(), "Shmem:", 6)) {
                int nr = sscanf(line.data() + 6, "%llu", &d->shmem_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("Shmem"));

            } else if (!strncmp(line.data(), "Slab:", 5)) {
                int nr = sscanf(line.data() + 5, "%llu", &d->slab_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("Slab"));

            } else if (!strncmp(line.data(), "Mapped:", 7)) {
                int nr = sscanf(line.data() + 7, "%llu", &d->mapped_kb);

                if (nr != 1)
                    print_errno(errno, QString("parse %1 -> %2 failed").arg(PROC_PATH_MEM).arg("Mapped"));
            }
        } // ::while(fgets)

        if (ferror(fp)) {
            print_errno(errno, QString("read %1 failed").arg(PROC_PATH_MEM));
        }

        return;
    } // ::if(fopen)

    print_errno(errno, QString("open %1 failed").arg(PROC_PATH_MEM));
}

} // namespace system
} // namespace core
