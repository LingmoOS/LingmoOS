// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPU_INFO_MODEL_H
#define CPU_INFO_MODEL_H

#include "common/sample.h"
#include "common/common.h"
#include "system/sys_info.h"
#include "cpu_stat_model.h"

#include <QObject>
#include <QMap>

#include <memory>

using namespace common::core;
using namespace common::format;
using namespace core::system;

template<>
class SampleFrame<load_avg_t>
{
public:
    SampleFrame()
        : ts()
        , stat()
    {
    }
    SampleFrame(const struct timeval &tv, const LoadAvg &st)
        : ts(tv)
        , stat(st)
    {
    }
    SampleFrame(const SampleFrame &other)
        : ts(other.ts)
        , stat(std::make_shared<load_avg_t>(*(other.stat)))
    {
    }

    struct timeval ts;
    LoadAvg stat;
};
using LoadAvgSampleFrame = SampleFrame<load_avg_t>;
using LoadAvgSample = Sample<load_avg_t>;

class CPUListModel;

class CPUInfoModel : public QObject
{
    Q_OBJECT

public:
    explicit CPUInfoModel();
    virtual ~CPUInfoModel() override = default;

    static CPUInfoModel *instance();

    std::weak_ptr<CPUListModel> cpuListModel() const;

    QList<qreal> cpuPercentList() const;
    qreal cpuAllPercent() const;

    QString loadavg() const;
    uint nProcesses() const;
    uint nThreads() const;
    uint nFileDescriptors() const;
    QString hostname() const;
    QString osType() const;
    QString osVersion() const;
    QString uptime() const;

    SysInfo *sysInfo();
    CPUSet *cpuSet();

signals:
    void modelUpdated();

public slots:
    void updateModel();

private:
    TimePeriod m_period;
    std::unique_ptr<Sample<cpu_stat_t>> m_overallStatSample;
    std::unique_ptr<Sample<cpu_usage_t>> m_overallUsageSample;
    std::unique_ptr<Sample<load_avg_t>> m_loadAvgSampleDB; // for loadavg monitoring extends

    QMap<QByteArray, std::shared_ptr<Sample<cpu_usage_t>>> m_singleUsageSample;

    SysInfo *m_sysInfo;
    CPUSet *m_cpuSet;
};

#endif // CPU_INFO_MODEL_H
