// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dmemorydevice.h"
#include "dlsdevice.h"

#include "scan.h"
#include "hw.h"

#include <QFileInfo>
#include <QDebug>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <memory>

#define PROC_PATH_MEM "/proc/meminfo"

DDEVICE_BEGIN_NAMESPACE
struct uFDClose {
    void operator()(int *fd)
    {
        close(*fd);
    }
};

struct uFileClose {
    void operator()(FILE *fp)
    {
        fclose(fp);
    }
};

struct uCloseDir {
    void operator()(DIR *dir)
    {
        closedir(dir);
    }
};

using uFD   = std::unique_ptr<int, uFDClose>;
using uFile = std::unique_ptr<FILE, uFileClose>;
using uDir  = std::unique_ptr<DIR, uCloseDir>;

class DMemoryDevicePrivate
{
public:
    explicit DMemoryDevicePrivate(DMemoryDevice *parent)
        : m_hwNode("computer", hw::sys_tem)
        , q_ptr(parent)
    {
        m_listDeviceInfo.clear();
        scan_system(m_hwNode);
        // addDeviceInfo(m_hwNode, m_listDeviceInfo);
        addMemInfo();
    }
    hwNode                    m_hwNode ;
    QList< DlsDevice::DDeviceInfo >      m_listDeviceInfo;
    void addDeviceInfo(hwNode &node, QList< DlsDevice::DDeviceInfo >  &infoLst);
    void addMemInfo();
    Q_DECLARE_PUBLIC(DMemoryDevice)

    inline void print_errno(decltype(errno) e, const QString msg)
    {
        qWarning() << QString("Error: [%1] %2, ").arg(e).arg(strerror(e)) << msg;
    }


private:
    DMemoryDevice *q_ptr = nullptr;
    QString m_swapSize;
    QString m_available;
    QString m_buffers;
    QString m_cached;
    QString m_active;
    QString m_inactive;
    QString m_sharedSize;
    QString m_swapFree;
    QString m_swapCached;
    QString m_slab;
    QString m_dirty;
    QString m_mapped;
};

void DMemoryDevicePrivate::addMemInfo()
{
    unsigned long long mem_total_kb = 0; // MemTotal
    unsigned long long mem_free_kb = 0; // MemFree
    unsigned long long mem_avail_kb = 0; // MemAvailable
    unsigned long long buffers_kb = 0; // Buffers
    unsigned long long cached_kb = 0; // Cached
    unsigned long long active_kb = 0; // Active
    unsigned long long inactive_kb = 0; // Inactive

    unsigned long long swap_total_kb = 0; // SwapTotal
    unsigned long long swap_free_kb = 0; // SwapFree
    unsigned long long swap_cached_kb = 0; // SwapCached
    unsigned long long shmem_kb = 0; // Shared
    unsigned long long slab_kb = 0; // Slab
    unsigned long long dirty_kb = 0; // Dirty
    unsigned long long mapped_kb = 0; // Mapped

    FILE *fp;
    uFile ufp;
    const size_t BUFLEN = 512;
    QByteArray line(BUFLEN, '\0');

    if ((fp = fopen(PROC_PATH_MEM, "r"))) {
        ufp.reset(fp);
        while (fgets(line.data(), BUFLEN, fp)) {
            QString meminfo = line;

            QStringList meminfos = meminfo.split(' ', D_SPLIT_BEHAVIOR);
            if (meminfo.startsWith("MemTotal:") && 3 == meminfos.count())
                mem_total_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("MemFree:") && 3 == meminfos.count())
                mem_free_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("MemAvailable:") && 3 == meminfos.count())
                mem_avail_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("Buffers:") && 3 == meminfos.count())
                buffers_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("Cached:") && 3 == meminfos.count())
                cached_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("SwapCached:") && 3 == meminfos.count())
                swap_cached_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("Active:") && 3 == meminfos.count())
                active_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("Inactive:") && 3 == meminfos.count())
                inactive_kb = meminfos.value(1).toULongLong();

            else if (meminfo.startsWith("SwapTotal:") && 3 == meminfos.count())
                swap_total_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("SwapFree:") && 3 == meminfos.count())
                swap_free_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("Dirty:") && 3 == meminfos.count())
                dirty_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("Shmem:") && 3 == meminfos.count())
                shmem_kb = meminfos.value(1).toULongLong();

            else if (meminfo.startsWith("Slab:") && 3 == meminfos.count())
                slab_kb = meminfos.value(1).toULongLong();
            else if (meminfo.startsWith("Mapped:") && 3 == meminfos.count())
                mapped_kb = meminfos.value(1).toULongLong();
        } // ::while(fgets)

        if (ferror(fp)) {
            print_errno(errno, QString("read %1 failed").arg(PROC_PATH_MEM));
        }
    }
    Q_UNUSED(mem_total_kb);
    Q_UNUSED(mem_free_kb);
    m_swapSize    = QString::number(swap_total_kb);
    m_available   = QString::number(mem_avail_kb);
    m_buffers     = QString::number(buffers_kb);
    m_cached      = QString::number(cached_kb);
    m_active      = QString::number(active_kb);
    m_inactive    = QString::number(inactive_kb);
    m_sharedSize  = QString::number(shmem_kb);
    m_swapFree    = QString::number(swap_free_kb);
    m_swapCached  = QString::number(swap_cached_kb);
    m_slab        = QString::number(slab_kb);
    m_dirty       = QString::number(dirty_kb);
    m_mapped      = QString::number(mapped_kb);
    return;
}
void DMemoryDevicePrivate::addDeviceInfo(hwNode &node, QList<DlsDevice::DDeviceInfo> &infoLst)
{
    struct DlsDevice::DDeviceInfo entry;

    entry.deviceInfoLstMap.clear();
    entry.devClass = DlsDevice::DtkMemory;

    if (node.getBusInfo() != "") {
        entry.deviceBaseAttrisLst.append("SysFs_PATH");
        entry.deviceInfoLstMap["SysFs_PATH"] = QString::fromStdString(node.getBusInfo());
    }

    entry.subClassName = QString::fromStdString(node.getsubClassName());

    entry.deviceBaseAttrisLst.append("Description");
    entry.deviceInfoLstMap.insert("Description", QString::fromStdString(node.getDescription()));

    entry.deviceBaseAttrisLst.append("sysID");
    entry.deviceInfoLstMap.insert("sysID", QString::fromStdString(node.getId()));

    entry.deviceBaseAttrisLst.append("Vendor");
    entry.deviceInfoLstMap.insert("Vendor", QString::fromStdString(node.getVendor()));
    entry.vendorName = QString::fromStdString(node.getVendor());

    entry.deviceBaseAttrisLst.append("Name");
    entry.deviceInfoLstMap.insert("Name", QString::fromStdString(node.getProduct()));
    entry.productName = QString::fromStdString(node.getProduct());

    if (node.getSize() > 0) {
        string size = to_string(node.getSize());
        entry.deviceBaseAttrisLst.append("size");
        entry.deviceInfoLstMap.insert("size",  QString::fromStdString(size));
    }

//    entry.deviceInfoLstMap.insert("speed", QString::fromStdString(node.getClock()));
    entry.deviceInfoLstMap.insert("serial", QString::fromStdString(node.getSerial()));
//    entry.deviceInfoLstMap.insert("width", QString::fromStdString(node.getWidth()));
    entry.deviceInfoLstMap.insert("type", QString::fromStdString(node.getDescription()));
    entry.deviceInfoLstMap.insert("MemAvailable", QString::fromStdString(node.getConfig("MemAvailable")));

//--------------------------------ADD Children---------------------
    if (hw::ddr == node.getClass())
        infoLst.append(entry);
    for (int i = 0; i < node.countChildren(); i++) {
        addDeviceInfo(*node.getChild(uint(i)), infoLst);
    }
}

DMemoryDevice::DMemoryDevice(QObject *parent)
    : QObject(parent)
    , d_ptr(new DMemoryDevicePrivate(this))
{
}

DMemoryDevice::~DMemoryDevice()
{
}

int DMemoryDevice::count()
{
    Q_D(DMemoryDevice);
    return int(d->m_listDeviceInfo.count());
}

QString DMemoryDevice::vendor(int index)
{
    Q_D(DMemoryDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].vendorName;
    } else
        return QString();
}

QString DMemoryDevice::model(int index)
{
    Q_D(DMemoryDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].productName;
    } else
        return QString();
}

QString DMemoryDevice::totalWidth(int /*index*/)
{
    return QString();
}

QString DMemoryDevice::dataWidth(int /*index*/)
{
    return QString();
}

QString DMemoryDevice::type(int index)
{
    Q_D(DMemoryDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("type");
    } else
        return QString();
}

QString DMemoryDevice::speed(int index)
{
    Q_D(DMemoryDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("speed");
    } else
        return QString();
}

QString DMemoryDevice::serialNumber(int index)
{
    Q_D(DMemoryDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("serial");
    } else
        return QString();
}

QString DMemoryDevice::size(int index)
{
    Q_D(DMemoryDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("size");
    } else
        return QString();
}

QString DMemoryDevice::swapSize()
{
    Q_D(DMemoryDevice);
    return d->m_swapSize;
}

QString DMemoryDevice::available()
{
    Q_D(DMemoryDevice);
    return d->m_available;
}

QString DMemoryDevice::buffers()
{
    Q_D(DMemoryDevice);
    return d->m_buffers;
}

QString DMemoryDevice::cached()
{
    Q_D(DMemoryDevice);
    return d->m_cached;
}

QString DMemoryDevice::active()
{
    Q_D(DMemoryDevice);
    return d->m_active ;
}

QString DMemoryDevice::inactive()
{
    Q_D(DMemoryDevice);
    return d->m_inactive ;

}

QString DMemoryDevice::sharedSize()
{
    Q_D(DMemoryDevice);
    return d->m_sharedSize ;
}

QString DMemoryDevice::swapFree()
{
    Q_D(DMemoryDevice);
    return d->m_swapFree ;
}

QString DMemoryDevice::swapCached()
{
    Q_D(DMemoryDevice);
    return d->m_swapCached ;
}

QString DMemoryDevice::slab()
{
    Q_D(DMemoryDevice);
    return d->m_slab ;
}

QString DMemoryDevice::dirty()
{
    Q_D(DMemoryDevice);
    return d->m_dirty ;
}

QString DMemoryDevice::mapped()
{
    Q_D(DMemoryDevice);
    return d->m_mapped ;
}

DDEVICE_END_NAMESPACE
