// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcpudevice.h"
#include "dlsdevice.h"

#include "scan.h"
#include "hw.h"

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>

#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <errno.h>
#include <stdio.h>
#include <sys/sysinfo.h>


DDEVICE_BEGIN_NAMESPACE


#define PATH_TPL_CPU_CACHE "/sys/devices/system/cpu/%d/cache"
#define PATH_TPL_CPU_CACHE_ID "/sys/devices/system/cpu/%d/cache/index%d/id"
#define PATH_TPL_CPU_CACHE_TYPE "/sys/devices/system/cpu/%d/cache/index%d/type"
#define PATH_TPL_CPU_CACHE_SIZE "/sys/devices/system/cpu/%d/cache/index%d/size"
#define PATH_TPL_CPU_CACHE_LEVEL "/sys/devices/system/cpu/%d/cache/index%d/level"
#define PATH_TPL_CPU_CACHE_SHR_CPU_LIST "/sys/devices/system/cpu/%d/cache/index%d/shared_cpu_list"

#define PATH_TPL_CPU_FREQ_MAX "/sys/devices/system/cpu/cpu%1/cpufreq/cpuinfo_max_freq"
#define PATH_TPL_CPU_FREQ_MIN "/sys/devices/system/cpu/cpu%1/cpufreq/cpuinfo_min_freq"
#define PATH_TPL_CPU_FREQ_CUR "/sys/devices/system/cpu/cpu%1/cpufreq/scaling_cur_freq"

#define PATH_TPL_CPU_TOPO_CORE_ID "/sys/devices/system/cpu/%d/topology/core_id"
#define PATH_TPL_CPU_TOPO_CORE_SIBLINGS "/sys/devices/system/cpu/%d/topology/core_siblings_list"
#define PATH_TPL_CPU_TOPO_PHY_PKG_ID "/sys/devices/system/cpu/%d/topology/physical_package_id"
#define PATH_TPL_CPU_TOPO_THR_SIBLINGS "/sys/devices/system/cpu/%d/topology/thread_siblings_list"

#define PATH_CPU_TEMP_INPUT  "/sys/class/hwmon/hwmon1/temp1_input"

#define PROC_CPU_STAT_PATH "/proc/stat"
#define PROC_CPU_INFO_PATH "/proc/cpuinfo"

struct cpuInfoLst {
    int  processorID {0};
    int   physicalID  {0};
    int   coreID  {0};
    QString   minFreq  {""};
    QString   maxFreq  {""};
    QString   cache_All {""};
    QString   cache_L1 {""};
    QString   cache_L2 {""};
    QString   cache_L3 {""};
    QString   flags  {""};
    QString   stepping  {""};
    QString   family  {""};
    QString   bogoMIPS  {""};
    QString   currentFreq  {""};
    DCpuDevice::DCpuStat   stat ;
    DCpuDevice::DCpuUsage   usage ;
};

struct cpuBaseInfo{
    int       physicalCount{0};
    QString   vendor  {""};
    QString   model  {""};
    int   coreCount  {0};
    int   threadCount{0};
    QString   architecture  {""};
    QString   temperature  {""};
    DCpuDevice::DCpuStat   stat ;
    DCpuDevice::DCpuUsage   usage ;
};

class DCpuDevicePrivate
{
public:
    explicit DCpuDevicePrivate(DCpuDevice *parent)
        : q_ptr(parent)
        , m_hwNode("computer", hw::sys_tem)
    {

        m_listDeviceInfos.clear();
        m_cpuBaseInfos.clear();
        scan_system(m_hwNode);
        addDeviceInfo(m_hwNode, m_listDeviceInfos);
        addDeviceInfo(m_infos,m_cpuBaseInfos);
    }

    void addDeviceInfo(hwNode &node, QList< DlsDevice::DDeviceInfo >  &infoLst);
    void addDeviceInfo(QList< cpuInfoLst >  &infoLst, QList < cpuBaseInfo > & baseInfoLst);


    Q_DECLARE_PUBLIC(DCpuDevice)
private:
    DCpuDevice *q_ptr = nullptr;

    hwNode            m_hwNode ;
    QList<cpuInfoLst> m_infos;
    QList <cpuBaseInfo> m_cpuBaseInfos;
    QList< DlsDevice::DDeviceInfo > m_listDeviceInfos;
};

void DCpuDevicePrivate::addDeviceInfo(hwNode &node, QList< DlsDevice::DDeviceInfo> &infoLst)
{
    DlsDevice::DDeviceInfo entry;
    entry.deviceInfoLstMap.clear();
    entry.devClass = DlsDevice::DtkCpu;

    entry.deviceBaseAttrisLst.append("Vendor");
    entry.deviceInfoLstMap.insert("Vendor", QString::fromStdString(node.getVendor()));
    entry.vendorName = QString::fromStdString(node.getVendor());

    entry.deviceBaseAttrisLst.append("Name");
    entry.deviceInfoLstMap.insert("Name", QString::fromStdString(node.getProduct()));
    entry.productName = QString::fromStdString(node.getProduct());

//--------------------------------ADD Children---------------------
    if (hw::processor == node.getClass())
        infoLst.append(entry);
    for (int i = 0; i < node.countChildren(); i++) {
        addDeviceInfo(*node.getChild(i), infoLst);
    }
}

void DCpuDevicePrivate::addDeviceInfo(QList<cpuInfoLst> &infoLst, QList<cpuBaseInfo> &baseInfoLst)
{
    QFile file(PROC_CPU_INFO_PATH);
    QString vendor = "";
    QString Product = "";
    int prePhysicalID = -1;

    if (file.exists() && file.open(QFile::ReadOnly)) {
        // 计算总的Cpu占用率，只需要读取第一行数据
        QString cpuinfo = file.readLine();
        file.close();

        QStringList processors = cpuinfo.split("\n\n", D_SPLIT_BEHAVIOR);
        for (int i = 0; i < processors.count(); ++i) {
            struct cpuInfoLst info;
            QStringList list = processors[i].split("\n", D_SPLIT_BEHAVIOR);
            for (QString text : list) {
                if (text.startsWith("processor")) {
                    info.processorID = (text.split(":").value(1).toInt());
                } else if (text.startsWith("vendor_id")) {
                    if (vendor.isEmpty())
                        vendor = (text.split(":").value(1));
                } else if (text.startsWith("model name")) {
                    if (Product.isEmpty())
                        Product = (text.split(":").value(1));
                } else if (text.startsWith("core id")) {
                    info.coreID = (text.split(":").value(1).toInt());
                } else if (text.startsWith("physical id")) {
                    info.physicalID = (text.split(":").value(1).toInt());
                } else if (text.startsWith("cache size")) {
                    info.cache_All = (text.split(":").value(1));
                } else if (text.startsWith("flags")) {
                    info.flags = (text.split(":").value(1));
                } else if (text.startsWith("stepping")) {
                    info.stepping = (text.split(":").value(1));
                } else if (text.startsWith("cpu family")) {
                    info.family = (text.split(":").value(1));
                } else if (text.startsWith("bogomips")) {
                    info.bogoMIPS = (text.split(":").value(1));
                } else if (text.startsWith("cpu MHz")) {
                    info.currentFreq = (text.split(":").value(1));
                } else if (text.startsWith("stepping")) {
                    info.stepping = (text.split(":").value(1));
                } else if (text.startsWith("stepping")) {
                    info.stepping = (text.split(":").value(1));
                }

                QFile freqmaxfile(QString(PATH_TPL_CPU_FREQ_MAX).arg(info.processorID));
                if (freqmaxfile.exists() && freqmaxfile.open(QFile::ReadOnly)) {
                    // 计算总的Cpu占用率，只需要读取第一行数据
                    QString freqmaxinfo = freqmaxfile.readLine();
                    freqmaxfile.close();
                    int freqmax = freqmaxinfo.toInt() / 1000;
                    freqmaxinfo = QString::number(freqmax) + "Mhz";
                    info.maxFreq = freqmaxinfo;
                }

                QFile freqminfile(QString(PATH_TPL_CPU_FREQ_MIN).arg(info.processorID));
                if (freqminfile.exists() && freqminfile.open(QFile::ReadOnly)) {
                    // 计算总的Cpu占用率，只需要读取第一行数据
                    QString freqmininfo = freqminfile.readLine();
                    freqminfile.close();
                    int freqmin = freqmininfo.toInt() / 1000;
                    freqmininfo = QString::number(freqmin) + "Mhz";
                    info.minFreq = freqmininfo;
                }
                if (prePhysicalID != info.physicalID) {
                    prePhysicalID = info.physicalID;
                    struct cpuBaseInfo baseInfo;
                    baseInfo.vendor = vendor;
                    baseInfo.model = Product;
                    baseInfo.physicalCount = prePhysicalID;

                    QString buffer{};
                    struct utsname os
                    {};
                    auto rc = uname(&os);
                    if (!rc) {
                        buffer = os.machine;
                        baseInfo.architecture = buffer;
                    }

                    QDir dir("/proc");
                    QFileInfoList infoList = dir.entryInfoList();
                    int threads = 0;
                    for (QFileInfo info : infoList) {
                        if (info.isDir() && info.fileName().toInt() > 0) {
                            QDir taskDir("/proc/" + info.fileName() + "/task");
                            threads += taskDir.entryInfoList().count();
                        }
                    }
                    baseInfo.threadCount = threads;

                    // Linux下获取cpu温度 cat /sys/class/hwmon/hwmon1/temp1_input
                    // https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface
                    // cat /sys/class/hwmon/hwmon1/temp1_label      Package id 0

                    QFile tempfile(PATH_CPU_TEMP_INPUT);
                    int temperature = 0;
                    if (tempfile.exists() && tempfile.open(QFile::ReadOnly)) {
                        QString tempinfo = tempfile.readLine();
                        tempfile.close();
                        temperature = tempinfo.toInt() / 1000;
                        tempinfo = QString::number(temperature) + "°C";
                        baseInfo.temperature = tempinfo;
                    }

                    QFile file(PROC_CPU_STAT_PATH);
                    if (file.exists() && file.open(QFile::ReadOnly)) {
                        // 计算总的Cpu占用率，只需要读取第一行数据
                        QByteArray lineData = file.readLine();
                        file.close();
                        // 样例数据 ： cpu  7048360 4246 3733400 801045435 846386 0 929664 0 0 0
                        //         |user|nice|sys|idle|iowait|hardqirq|softirq|steal|guest|guest_nice|
                        // 分割行数据

                        QStringList cpuStatus = QString(lineData).split(" ", D_SPLIT_BEHAVIOR);

                        // CPU状态应包含10个数据片段，有效数据 1-10，位置0不使用
                        if (cpuStatus.size() >= 11) {
                            // 构建数据map，便于后期数据计算方式需求变更s

                            baseInfo.stat.user = cpuStatus.at(1).toULongLong();
                            baseInfo.stat.nice = cpuStatus.at(2).toULongLong();
                            baseInfo.stat.sys = cpuStatus.at(3).toULongLong();
                            baseInfo.stat.idle = cpuStatus.at(4).toULongLong();
                            baseInfo.stat.iowait = cpuStatus.at(5).toULongLong();
                            baseInfo.stat.hardirq = cpuStatus.at(6).toULongLong();
                            baseInfo.stat.softirq = cpuStatus.at(7).toULongLong();
                            baseInfo.stat.steal = cpuStatus.at(8).toULongLong();
                            baseInfo.stat.guest = cpuStatus.at(9).toULongLong();
                            baseInfo.stat.guestNice = cpuStatus.at(10).toULongLong();

                            quint64 curTotalCpu = 0;
                            for (int i = 1; i <= 10; i++) {
                                curTotalCpu = curTotalCpu + cpuStatus.at(i).toULongLong();
                            }
                            baseInfo.usage.total = curTotalCpu;
                            baseInfo.usage.idle = baseInfo.stat.idle + baseInfo.stat.iowait;
                        }
                    }

                    baseInfoLst.append(baseInfo);
                }
            }

            infoLst.append(info);
        }
    }

    for (uint nn = 0; nn < infoLst.count(); nn++) {
        QFile file(PROC_CPU_STAT_PATH);
        if (file.exists() && file.open(QFile::ReadOnly)) {
            // 计算总的Cpu占用率，只需要读取第一行数据
            QString lines = file.readLine();
            file.close();

            QStringList lineData = lines.split("\n", D_SPLIT_BEHAVIOR);
            for (int i = 0; i < lineData.count(); ++i) {
                // 样例数据 ： cpu  7048360 4246 3733400 801045435 846386 0 929664 0 0 0
                //         |user|nice|sys|idle|iowait|hardqirq|softirq|steal|guest|guest_nice|
                // 分割行数据
                QStringList cpuStatus = lineData[i].split(" ", D_SPLIT_BEHAVIOR);

                // CPU状态应包含10个数据片段，有效数据 1-10，位置0不使用

                QRegularExpression reg = QRegularExpression("cpu[0-9]{1,2}");
                if ((cpuStatus.size() >= 11) && reg.match(cpuStatus.at(0)).hasMatch()) {
                    infoLst[nn].stat.user = cpuStatus.at(1).toULongLong();
                    infoLst[nn].stat.nice = cpuStatus.at(2).toULongLong();
                    infoLst[nn].stat.sys = cpuStatus.at(3).toULongLong();
                    infoLst[nn].stat.idle = cpuStatus.at(4).toULongLong();
                    infoLst[nn].stat.iowait = cpuStatus.at(5).toULongLong();
                    infoLst[nn].stat.hardirq = cpuStatus.at(6).toULongLong();
                    infoLst[nn].stat.softirq = cpuStatus.at(7).toULongLong();
                    infoLst[nn].stat.steal = cpuStatus.at(8).toULongLong();
                    infoLst[nn].stat.guest = cpuStatus.at(9).toULongLong();
                    infoLst[nn].stat.guestNice = cpuStatus.at(10).toULongLong();

                    quint64 curTotalCpu = 0;
                    for (int i = 1; i <= 10; i++) {
                        curTotalCpu = curTotalCpu + cpuStatus.at(i).toULongLong();
                    }
                    infoLst[nn].usage.total = curTotalCpu;
                    infoLst[nn].usage.idle = infoLst[nn].stat.idle + infoLst[nn].stat.iowait;
                }
            }
        }
    }
}

DCpuDevice::DCpuDevice(QObject *parent)
    : QObject(parent)
    , d_ptr(new DCpuDevicePrivate(this))
{
}

DCpuDevice::~DCpuDevice()
{
}

int DCpuDevice::physicalCount()
{
    Q_D(DCpuDevice);
    return  d->m_cpuBaseInfos.count();
}

int DCpuDevice::coreCount(int physicalID)
{
    Q_D(DCpuDevice);
    if(physicalID < physicalCount())
        return  d->m_cpuBaseInfos[physicalID].coreCount;
    return -1;
}

int DCpuDevice::threadCount(int physicalID, int /*coreID*/)
{  
    Q_D(DCpuDevice); 
    if(physicalID < physicalCount())
        return  d->m_cpuBaseInfos[physicalID].threadCount;
    return -1;
}

int DCpuDevice::physicalID(int processorID)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].physicalID;
    return -1;
}

int DCpuDevice::coreID(int processorID)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].coreID;
    return -1;
}

QString DCpuDevice::architecture(int physicalID)
{
    Q_D(DCpuDevice);
    if(physicalID < physicalCount())
        return  d->m_cpuBaseInfos[physicalID].architecture;
    return QString();
}

QString DCpuDevice::vendor(int physicalID)
{
    Q_D(DCpuDevice);
    if(physicalID <physicalCount())
        return  d->m_cpuBaseInfos[physicalID].vendor;
    return QString();
}

QString DCpuDevice::model(int physicalID)
{
    Q_D(DCpuDevice);
    if(physicalID <physicalCount())
        return  d->m_cpuBaseInfos[physicalID].model;
    return QString();
}

QString DCpuDevice::minFreq(int processorID)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].minFreq;
    return QString();
}

QString DCpuDevice::maxFreq(int processorID)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].maxFreq;
    return QString();
}

QString DCpuDevice::cache(int processorID, QString /*type*/)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].cache_All;
    return QString();
}

QString DCpuDevice::flags(int processorID)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].flags;
    return QString();
}

QString DCpuDevice::stepping(int processorID)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].stepping;
    return QString();
}

QString DCpuDevice::family(int processorID)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].family;
    return QString();
}

QString DCpuDevice::bogoMIPS(int processorID)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].bogoMIPS; 
    return QString();
}

QString DCpuDevice::temperature(int physicalID)
{
    Q_D(DCpuDevice);
    if(physicalID <physicalCount())
        return  d->m_cpuBaseInfos[physicalID].temperature;
    return QString();
}

QString DCpuDevice::currentFreq(int processorID)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].currentFreq;
    return QString();
}

DCpuDevice::DCpuStat DCpuDevice::stat()
{
    Q_D(DCpuDevice);
    return  d->m_cpuBaseInfos[0].stat; 
}

DCpuDevice::DCpuUsage DCpuDevice::usage()
{
     Q_D(DCpuDevice);
    return  d->m_cpuBaseInfos[0].usage;
}

DCpuDevice::DCpuStat DCpuDevice::stat(int processorID)
{
        Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
        return  d->m_infos[processorID].stat; 
    return DCpuStat();
}

DCpuDevice::DCpuUsage DCpuDevice::usage(int processorID)
{
    Q_D(DCpuDevice);
    if(processorID < d->m_infos.count())
    return  d->m_infos[processorID].usage;

    return DCpuUsage();
}

DDEVICE_END_NAMESPACE
