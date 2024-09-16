// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpu_stat_model.h"

// Model constructor
CPUStatModel::CPUStatModel(const TimePeriod &period, QObject *parent)
    : QAbstractTableModel(parent)
    , m_statSampleDB(new CPUStatSample(period))
    , m_usageSampleDB(new CPUUsageSample(period))
{
}

CPUStatModel::~CPUStatModel()
{
}

int CPUStatModel::rowCount(const QModelIndex &) const
{
    if (m_statSampleDB && m_usageSampleDB
            && m_statSampleDB->count() == m_usageSampleDB->count())
        return m_usageSampleDB->count();
    else
        return 0;
}

int CPUStatModel::columnCount(const QModelIndex &) const
{
    return kStatPropMax;
}

QVariant CPUStatModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_usageSampleDB || !m_statSampleDB)
        return {};

    int row = index.row();
    if (row < 0 || row >= int(m_usageSampleDB->count()))
        return {};

    auto *stat = m_statSampleDB->sample(index.row());
    auto *usage = m_usageSampleDB->sample(index.row());
    if (!stat || !usage)
        return {};

    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
        switch (index.column()) {
        case kStatUserTime: {
            return stat->stat->user;
        }
        case kStatNiceTime: {
            return stat->stat->nice;
        }
        case kStatSystemTime: {
            return stat->stat->sys;
        }
        case kStatIdleTime: {
            return stat->stat->idle;
        }
        case kStatIOWaitTime: {
            return stat->stat->iowait;
        }
        case kStatHardIRQ: {
            return stat->stat->hardirq;
        }
        case kStatSoftIRQ: {
            return stat->stat->softirq;
        }
        case kStatStealTime: {
            return stat->stat->steal;
        }
        case kStatGuestTime: {
            return stat->stat->guest;
        }
        case kStatGuestNice: {
            return stat->stat->guest_nice;
        }
        case kStatTotalUsage: {
            return usage->stat->total;
        }
        case kStatIdleUsage: {
            return usage->stat->idle;
        }
        default:
            break;
        } // ::switch
    } // ::if

    return {};
}
