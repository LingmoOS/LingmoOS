// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPUSET_H
#define CPUSET_H

#include "cpu.h"
#include "3rdparty/dmidecode/dmidecode.h"
#include <QList>
#include <QSharedDataPointer>

namespace core {
namespace system {

class CPUSetPrivate;
class CPUSet
{
    friend class Process;

public:
    CPUSet();

    CPUSet(const CPUSet &other);

    CPUSet &operator=(const CPUSet &rhs);

    virtual ~CPUSet();

public://info
    QString modelName() const;

    QString vendor() const;

    int cpuCount() const;

    int socketCount() const;

    QString virtualization() const;

    QString curFreq() const;

    QString minFreq() const;

    QString maxFreq() const;

    QString l1dCache() const;

    QString l1iCache() const;

    QString l2Cache() const;

    QString l3Cache() const;

public://core
    QString coreId(int index) const;

public://usage
    const CPUUsage usage() const;

    const CPUStat stat() const;

    QList<QByteArray> cpuLogicName() const;

    const CPUStat statDB(const QByteArray &cpu) const;

    const CPUUsage usageDB(const QByteArray &cpu) const;

    qulonglong getUsageTotalDelta() const;

public:
    void update();

private:
    void read_stats();
    /**
     * @brief read_dmidecode 通过dmidecod读取CPU的cache信息
     */
    void read_dmi_cache_info();
    /**
     * @brief read_lscpu 通过lscpu读取CPU信息
     */
    void read_lscpu();
    void read_overall_info();

private:
    QSharedDataPointer<CPUSetPrivate> d;

    //true:modelName为空; false:modelName非空
    bool mIsEmptyModelName = false;

};

} // namespace system
} // namespace core

#endif // CPUSET_H
