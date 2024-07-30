/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2022 Mirco Miranda

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kmemoryinfo.h"

#include <QLoggingCategory>
#include <QSharedData>

Q_DECLARE_LOGGING_CATEGORY(LOG_KMEMORYINFO)
Q_LOGGING_CATEGORY(LOG_KMEMORYINFO, "kf.coreaddons.kmemoryinfo", QtWarningMsg)

// clang-format off
#if defined(Q_OS_WINDOWS)
    #include <windows.h>    // Windows.h must stay above Pspapi.h
    #include <psapi.h>
#elif defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
    #include <QByteArray>
    #include <QFile>
    #include <QByteArrayView>
#elif defined(Q_OS_MACOS)
    #include <mach/mach.h>
    #include <sys/sysctl.h>
#elif defined(Q_OS_FREEBSD)
    #include <fcntl.h>
    #include <kvm.h>
    #include <sys/sysctl.h>
#elif defined(Q_OS_OPENBSD)
     #include <sys/mount.h>
     #include <sys/param.h>	/* DEV_BSIZE PZERO */
     #include <sys/swap.h>
     #include <sys/syscall.h>
     #include <sys/sysctl.h>

     #include <stdio.h>
     #include <stdlib.h>
     #include <strings.h>
     #include <unistd.h>
#endif
// clang-format on

class KMemoryInfoPrivate : public QSharedData
{
public:
    KMemoryInfoPrivate()
    {
    }

    quint64 m_totalPhysical = 0;
    quint64 m_availablePhysical = 0;
    quint64 m_freePhysical = 0;
    quint64 m_totalSwapFile = 0;
    quint64 m_freeSwapFile = 0;
    quint64 m_cached = 0;
    quint64 m_buffers = 0;
};

KMemoryInfo::KMemoryInfo()
    : d(new KMemoryInfoPrivate)
{
    update();
}

KMemoryInfo::~KMemoryInfo()
{
}

KMemoryInfo::KMemoryInfo(const KMemoryInfo &other)
    : d(other.d)
{
}

KMemoryInfo &KMemoryInfo::operator=(const KMemoryInfo &other)
{
    d = other.d;
    return *this;
}

bool KMemoryInfo::operator==(const KMemoryInfo &other) const
{
    if (this == &other) {
        return true;
    }
    // clang-format off
    return (d->m_availablePhysical == other.d->m_availablePhysical
            && d->m_freePhysical == other.d->m_freePhysical
            && d->m_freeSwapFile == other.d->m_freeSwapFile
            && d->m_cached == other.d->m_cached
            && d->m_buffers == other.d->m_buffers
            && d->m_totalSwapFile == other.d->m_totalSwapFile
            && d->m_totalPhysical == other.d->m_totalPhysical);
    // clang-format on
}

bool KMemoryInfo::operator!=(const KMemoryInfo &other) const
{
    return !operator==(other);
}

bool KMemoryInfo::isNull() const
{
    return d->m_totalPhysical == 0;
}

quint64 KMemoryInfo::totalPhysical() const
{
    return d->m_totalPhysical;
}

quint64 KMemoryInfo::freePhysical() const
{
    return d->m_freePhysical;
}

quint64 KMemoryInfo::availablePhysical() const
{
    return d->m_availablePhysical;
}

quint64 KMemoryInfo::cached() const
{
    return d->m_cached;
}

quint64 KMemoryInfo::buffers() const
{
    return d->m_buffers;
}

quint64 KMemoryInfo::totalSwapFile() const
{
    return d->m_totalSwapFile;
}

quint64 KMemoryInfo::freeSwapFile() const
{
    return d->m_freeSwapFile;
}

#if defined(Q_OS_WINDOWS)
/*****************************************************************************
 * Windows
 ****************************************************************************/

struct SwapInfo {
    quint64 totalPageFilePages = 0;
    quint64 freePageFilePages = 0;
};

BOOL __stdcall pageInfo(LPVOID pContext, PENUM_PAGE_FILE_INFORMATION pPageFileInfo, LPCWSTR lpFilename)
{
    Q_UNUSED(lpFilename)
    if (auto sw = static_cast<SwapInfo *>(pContext)) {
        sw->totalPageFilePages += pPageFileInfo->TotalSize;
        sw->freePageFilePages += (pPageFileInfo->TotalSize - pPageFileInfo->TotalInUse);
        return true;
    }
    return false;
}

bool KMemoryInfo::update()
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex)) {
        return false;
    }

    PERFORMANCE_INFORMATION pi;
    DWORD pisz = sizeof(pi);
    if (!GetPerformanceInfo(&pi, pisz)) {
        return false;
    }

    SwapInfo si;
    if (!EnumPageFiles(pageInfo, &si)) {
        return false;
    }

    d->m_totalPhysical = statex.ullTotalPhys;
    d->m_availablePhysical = statex.ullAvailPhys;
    d->m_freePhysical = statex.ullAvailPhys;
    d->m_totalSwapFile = si.totalPageFilePages * pi.PageSize;
    d->m_freeSwapFile = si.freePageFilePages * pi.PageSize;
    d->m_cached = pi.SystemCache * pi.PageSize;
    d->m_buffers = 0;

    return true;
}

#elif defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
/*****************************************************************************
 * GNU/Linux
 ****************************************************************************/

using ByteArrayView = QByteArrayView;

bool extractBytes(quint64 &value, const QByteArray &buffer, const ByteArrayView &beginPattern, qsizetype &from)
{
    ByteArrayView endPattern("kB");
    auto beginIdx = buffer.indexOf(beginPattern, from);
    if (beginIdx > -1) {
        auto start = beginIdx + beginPattern.size();
        auto endIdx = buffer.indexOf(endPattern, start);
        if (endIdx > -1) {
            from = endIdx + endPattern.size();
            auto ok = false;
            value = buffer.mid(start, endIdx - start).toULongLong(&ok) * 1024;
            return ok;
        }
    }
    if (from) { // Wrong order? Restart from the beginning
        qCWarning(LOG_KMEMORYINFO) << "KMemoryInfo: extractBytes: wrong order when extracting" << beginPattern;
        from = 0;
        return extractBytes(value, buffer, beginPattern, from);
    }
    return false;
}

bool KMemoryInfo::update()
{
    QFile file(QStringLiteral("/proc/meminfo"));
    if (!file.open(QFile::ReadOnly)) {
        return false;
    }
    auto meminfo = file.readAll();
    file.close();

    qsizetype miFrom = 0;
    quint64 totalPhys = 0;
    if (!extractBytes(totalPhys, meminfo, "MemTotal:", miFrom)) {
        return false;
    }
    quint64 freePhys = 0;
    if (!extractBytes(freePhys, meminfo, "MemFree:", miFrom)) {
        return false;
    }
    quint64 availPhys = 0;
    if (!extractBytes(availPhys, meminfo, "MemAvailable:", miFrom)) {
        return false;
    }
    quint64 buffers = 0;
    if (!extractBytes(buffers, meminfo, "Buffers:", miFrom)) {
        return false;
    }
    quint64 cached = 0;
    if (!extractBytes(cached, meminfo, "Cached:", miFrom)) {
        return false;
    }
    quint64 swapTotal = 0;
    if (!extractBytes(swapTotal, meminfo, "SwapTotal:", miFrom)) {
        return false;
    }
    quint64 swapFree = 0;
    if (!extractBytes(swapFree, meminfo, "SwapFree:", miFrom)) {
        return false;
    }
    quint64 sharedMem = 0;
    if (!extractBytes(sharedMem, meminfo, "Shmem:", miFrom)) {
        return false;
    }
    quint64 sReclaimable = 0;
    if (!extractBytes(sReclaimable, meminfo, "SReclaimable:", miFrom)) {
        return false;
    }

    // Source HTOP: https://github.com/htop-dev/htop/blob/main/linux/LinuxProcessList.c
    d->m_totalPhysical = totalPhys;
    // NOTE: another viable solution: d->m_availablePhysical = std::min(availPhys, totalPhys - (committedAs - cached - (swapTotal - swapFree)))
    d->m_availablePhysical = availPhys ? std::min(availPhys, totalPhys) : freePhys;
    d->m_freePhysical = freePhys;
    d->m_totalSwapFile = swapTotal;
    d->m_freeSwapFile = swapFree;
    d->m_cached = cached + sReclaimable - sharedMem;
    d->m_buffers = buffers;

    return true;
}

#elif defined(Q_OS_MACOS)
/*****************************************************************************
 * macOS
 ****************************************************************************/

template<class T>
bool sysctlread(const char *name, T &var)
{
    auto sz = sizeof(var);
    return (sysctlbyname(name, &var, &sz, NULL, 0) == 0);
}

bool KMemoryInfo::update()
{
    quint64 memSize = 0;
    quint64 pageSize = 0;
    xsw_usage swapUsage;

    int mib[2];
    size_t sz = 0;

    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    sz = sizeof(memSize);
    if (sysctl(mib, 2, &memSize, &sz, NULL, 0) != KERN_SUCCESS) {
        return false;
    }

    mib[0] = CTL_HW;
    mib[1] = HW_PAGESIZE;
    sz = sizeof(pageSize);
    if (sysctl(mib, 2, &pageSize, &sz, NULL, 0) != KERN_SUCCESS) {
        return false;
    }

    mib[0] = CTL_VM;
    mib[1] = VM_SWAPUSAGE;
    sz = sizeof(swapUsage);
    if (sysctl(mib, 2, &swapUsage, &sz, NULL, 0) != KERN_SUCCESS) {
        return false;
    }

    quint64 zfs_arcstats_size = 0;
    if (!sysctlread("kstat.zfs.misc.arcstats.size", zfs_arcstats_size)) {
        zfs_arcstats_size = 0; // no ZFS used
    }

    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmstat;
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vmstat, &count) != KERN_SUCCESS) {
        return false;
    }

    d->m_totalPhysical = memSize;
    d->m_availablePhysical = memSize - (vmstat.internal_page_count + vmstat.compressor_page_count + vmstat.wire_count) * pageSize;
    d->m_freePhysical = vmstat.free_count * pageSize;
    d->m_totalSwapFile = swapUsage.xsu_total;
    d->m_freeSwapFile = swapUsage.xsu_avail;
    d->m_cached = vmstat.external_page_count * pageSize + zfs_arcstats_size;
    d->m_buffers = 0;

    return true;
}

#elif defined(Q_OS_FREEBSD)
/*****************************************************************************
 * FreeBSD
 ****************************************************************************/

template<class T>
bool sysctlread(const char *name, T &var)
{
    auto sz = sizeof(var);
    return (sysctlbyname(name, &var, &sz, NULL, 0) == 0);
}

bool KMemoryInfo::update()
{
    quint64 memSize = 0;
    quint64 pageSize = 0;

    int mib[4];
    size_t sz = 0;

    mib[0] = CTL_HW;
    mib[1] = HW_PHYSMEM;
    sz = sizeof(memSize);
    if (sysctl(mib, 2, &memSize, &sz, NULL, 0) != 0) {
        return false;
    }

    mib[0] = CTL_HW;
    mib[1] = HW_PAGESIZE;
    sz = sizeof(pageSize);
    if (sysctl(mib, 2, &pageSize, &sz, NULL, 0) != 0) {
        return false;
    }

    quint32 v_pageSize = 0;
    if (sysctlread("vm.stats.vm.v_page_size", v_pageSize)) {
        pageSize = v_pageSize;
    }
    quint64 zfs_arcstats_size = 0;
    if (!sysctlread("kstat.zfs.misc.arcstats.size", zfs_arcstats_size)) {
        zfs_arcstats_size = 0; // no ZFS used
    }
    quint32 v_cache_count = 0;
    if (!sysctlread("vm.stats.vm.v_cache_count", v_cache_count)) {
        return false;
    }
    quint32 v_inactive_count = 0;
    if (!sysctlread("vm.stats.vm.v_inactive_count", v_inactive_count)) {
        return false;
    }
    quint32 v_free_count = 0;
    if (!sysctlread("vm.stats.vm.v_free_count", v_free_count)) {
        return false;
    }
    quint64 vfs_bufspace = 0;
    if (!sysctlread("vfs.bufspace", vfs_bufspace)) {
        return false;
    }

    quint64 swap_tot = 0;
    quint64 swap_free = 0;
    if (auto kd = kvm_open("/dev/null", "/dev/null", "/dev/null", O_RDONLY, "kvm_open")) {
        struct kvm_swap swap;
        // if you specify a maxswap value of 1, the function will typically return the
        // value 0 and the single kvm_swap structure will be filled with the grand total over all swap devices.
        auto nswap = kvm_getswapinfo(kd, &swap, 1, 0);
        if (nswap == 0) {
            swap_tot = swap.ksw_total;
            swap_free = swap.ksw_used;
        }
        swap_free = (swap_tot - swap_free) * pageSize;
        swap_tot *= pageSize;
    }

    // Source HTOP: https://github.com/htop-dev/htop/blob/main/freebsd/FreeBSDProcessList.c
    d->m_totalPhysical = memSize;
    d->m_availablePhysical = pageSize * (v_cache_count + v_free_count + v_inactive_count) + vfs_bufspace + zfs_arcstats_size;
    d->m_freePhysical = pageSize * v_free_count;
    d->m_totalSwapFile = swap_tot;
    d->m_freeSwapFile = swap_free;
    d->m_cached = pageSize * v_cache_count + zfs_arcstats_size;
    d->m_buffers = vfs_bufspace;

    return true;
}

#elif defined(Q_OS_OPENBSD)
/*****************************************************************************
 * OpenBSD
 ****************************************************************************/
// From src/usr.bin/top/machine.c
static int swap_usage(int *used, int *total)
{
    struct swapent *swdev;
    int nswap, rnswap, i;

    nswap = swapctl(SWAP_NSWAP, nullptr, 0);
    if (nswap == 0)
        return 0;

    swdev = static_cast<struct swapent *>(calloc(nswap, sizeof(*swdev)));
    if (swdev == NULL)
        return 0;

    rnswap = swapctl(SWAP_STATS, swdev, nswap);
    if (rnswap == -1) {
        free(swdev);
        return 0;
    }
    /* Total things up */
    *total = *used = 0;
    for (i = 0; i < nswap; i++) {
        if (swdev[i].se_flags & SWF_ENABLE) {
            *used += (swdev[i].se_inuse / (1024 / DEV_BSIZE));
            *total += (swdev[i].se_nblks / (1024 / DEV_BSIZE));
        }
    }
    free(swdev);
    return 1;
}

bool KMemoryInfo::update()
{
    // TODO: compute m_availablePhysical on OpenBSD

    // tota phsycial memory
    const long phys_pages = sysconf(_SC_PHYS_PAGES);
    const long pagesize = sysconf(_SC_PAGESIZE);
    if (phys_pages != -1 && pagesize != -1)
        d->m_totalPhysical = ((uint64_t)phys_pages * (uint64_t)pagesize / 1024);

    int swap_free = 0;
    int swap_tot = 0;
    if (swap_usage(&swap_free, &swap_tot)) {
        d->m_totalSwapFile = swap_tot;
        d->m_freeSwapFile = swap_free;
    }

    int uvmexp_mib[] = {CTL_VM, VM_UVMEXP};
    struct uvmexp uvmexp;
    size_t size = sizeof(uvmexp);
    if (sysctl(uvmexp_mib, 2, &uvmexp, &size, NULL, 0) == -1) {
        bzero(&uvmexp, sizeof(uvmexp));
        return false;
    }
    d->m_freePhysical = uvmexp.free * pagesize / 1024;

    int bcstats_mib[] = {CTL_VFS, VFS_GENERIC, VFS_BCACHESTAT};
    struct bcachestats bcstats;
    size = sizeof(bcstats);
    if (sysctl(bcstats_mib, 3, &bcstats, &size, NULL, 0) == -1) {
        bzero(&bcstats, sizeof(bcstats));
        return false;
    }
    d->m_cached = bcstats.numbufpages * pagesize / 1024;

    return true;
}
#else
/*****************************************************************************
 * Unsupported platform
 ****************************************************************************/

bool KMemoryInfo::update()
{
    qCWarning(LOG_KMEMORYINFO) << "KMemoryInfo: unsupported platform!";
    return false;
}

#endif
