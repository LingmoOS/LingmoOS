// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPUINFO_H
#define CPUINFO_H

#include<QMap>
#include<QDir>

#include "physicalcpu.h"
#include "corecpu.h"
#include "logicalcpu.h"

class CpuInfo
{
public:
    CpuInfo();
    ~CpuInfo();

    /**
     * @brief diagPrintInfo
     */
    void diagPrintInfo();
    /**
     * @brief loadCpuInfo
     * @return
     */
    bool loadCpuInfo();

    /**
     * @brief arch
     * @return
     */
    const QString &arch() const;

    /**
     * @brief logicalCpus
     * @param info
     */
    void logicalCpus(QString &info);

    /**
     * @brief physicalNum
     * @return
     */
    int physicalNum();

    /**
     * @brief coreNum
     * @return
     */
    int coreNum();

    /**
     * @brief logicalNum
     * @return
     */
    int logicalNum();

private:
    /**
     * @brief readCpuArchitecture
     */
    void readCpuArchitecture();

    /**
     * @brief readProcCpuinfo
     * @return
     */
    bool readProcCpuinfo();

    /**
     * @brief parseInfo
     * @param info
     * @return
     */
    bool parseInfo(const QString &info);

    /**
     * @brief logicalCpu
     * @param logical_id
     * @param core_id
     * @param physical_id
     * @return
     */
    LogicalCpu &logicalCpu(int logical_id);

    /**
     * @brief setProcCpuinfo
     * @param logical
     * @param mapInfo
     */
    void setProcCpuinfo(LogicalCpu &logical, const QMap<QString, QString> &mapInfo);

    /**
     * @brief readSysCpu : /sys/devices/system/cpu
     */
    void readSysCpu();

    /**
     * @brief readSysCpuN : /sys/devices/system/cpu/cpu* (cpu0 cpu1 cpu2 cpu3 cpu4)
     * @param path : /sys/devices/system/cpu
     */
    void readSysCpuN(int N, const QString &path);

    /**
     * @brief readPhysicalID
     * @param path
     * @return
     */
    int readPhysicalID(const QString &path);

    /**
     * @brief readCoreID
     * @param path
     * @return
     */
    int readCoreID(const QString &path);

    /**
        * @brief readThreadSiblingsListPath
        * @param path
        * @return
        */
    int readThreadSiblingsListPath(const QString &path);

    /**
     * @brief readCpuCache : /sys/devices/system/cpu/cpu0/cache
     * @param path : /sys/devices/system/cpu/cpu0
     * @param lcpu
     */
    void readCpuCache(const QString &path, LogicalCpu &lcpu);

    /**
     * @brief readCpuCacheIndex : /sys/devices/system/cpu/cpu0/cache/index* (index0 index1 index2 index3)
     * @param path : /sys/devices/system/cpu/cpu0/cache/index0
     * @param lcpu
     */
    void readCpuCacheIndex(const QString &path, LogicalCpu &lcpu);

    /**
     * @brief readCpuFreq
     * @param path
     * @param lcpu
     */
    void readCpuFreq(const QString &path, LogicalCpu &lcpu);


private:
    QMap<int, PhysicalCpu>     m_MapPhysicalCpu;
    QString                    m_Arch;
};

#endif // CPUINFO_H
