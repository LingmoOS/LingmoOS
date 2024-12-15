// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPU_STAT_MODEL_H
#define CPU_STAT_MODEL_H

#include "system/cpu.h"
#include "common/sample.h"
#include "common/time_period.h"
#include "common/common.h"

#include <QObject>
#include <QAbstractTableModel>

using namespace core::system;
using namespace common::time;

template<>
class SampleFrame<cpu_stat_t>
{
public:
    SampleFrame()
        : ts()
        , stat()
    {
    }
    SampleFrame(const struct timeval &tv, const CPUStat &st)
        : ts(tv)
        , stat(st)
    {
    }
    SampleFrame(const SampleFrame &other)
        : ts(other.ts)
        , stat(std::make_shared<cpu_stat_t>(*(other.stat)))
    {
    }

    struct timeval ts;
    CPUStat stat;
};

template<>
class SampleFrame<cpu_usage_t>
{
public:
    SampleFrame()
        : ts()
        , stat()
    {
    }
    SampleFrame(const struct timeval &tv, const CPUUsage &st)
        : ts(tv)
        , stat(st)
    {
    }
    SampleFrame(const SampleFrame &other)
        : ts(other.ts)
        , stat(std::make_shared<cpu_usage_t>(*(other.stat)))
    {
    }

    SampleFrame &operator-=(const SampleFrame &rhs)
    {
        ts -= rhs.ts;
        stat->total -= rhs.stat->total;
        stat->idle -= rhs.stat->idle;
        return *this;
    }
    const SampleFrame operator-(const SampleFrame &rhs)
    {
        return SampleFrame(*this) -= rhs;
    }

    static qreal cpupc(const SampleFrame<cpu_usage_t> *lhs, const SampleFrame<cpu_usage_t> *rhs)
    {
        if (!lhs)
            return 0;

        if (!rhs)
            return qreal(lhs->stat->total - lhs->stat->idle) * 1. / lhs->stat->total * 100;

        auto totald = (rhs->stat->total > lhs->stat->total) ? (rhs->stat->total - lhs->stat->total) : 0;
        auto idled = (rhs->stat->idle > lhs->stat->idle) ? (rhs->stat->idle - lhs->stat->idle) : 0;

        return qreal(totald - idled) * 1. / totald * 100;
    }

    struct timeval ts;
    CPUUsage stat;
};

using CPUStatSampleFrame = SampleFrame<cpu_stat_t>;
using CPUUsageSampleFrame = SampleFrame<cpu_usage_t>;
using CPUStatSample = Sample<cpu_stat_t>;
using CPUUsageSample = Sample<cpu_usage_t>;

class CPUInfoModel;

/**
 * @brief CPU stat model
 */
class CPUStatModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum StatProp {
        kStatUserTime,
        kStatNiceTime,
        kStatSystemTime,
        kStatIdleTime,
        kStatIOWaitTime,
        kStatHardIRQ,
        kStatSoftIRQ,
        kStatStealTime,
        kStatGuestTime,
        kStatGuestNice,
        kStatTotalUsage,
        kStatIdleUsage,
        kStatPropMax
    };

    /**
     * @brief Model constructor
     * @param parent Parent object
     */
    explicit CPUStatModel(const TimePeriod &period, QObject *parent = nullptr);
    virtual ~CPUStatModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    std::unique_ptr<Sample<cpu_stat_t>> m_statSampleDB;
    std::unique_ptr<Sample<cpu_usage_t>> m_usageSampleDB;

    friend class CPUInfoModel;
};

#endif // CPU_STAT_MODEL_H
