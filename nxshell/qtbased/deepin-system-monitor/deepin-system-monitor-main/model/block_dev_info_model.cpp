// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//#include "block_dev_info_model.h"

//#include "common/common.h"
//#include "block_dev_stat_model.h"
//#include "system/system_monitor.h"
//#include "system/device_db.h"
//#include "system/sys_info.h"
//#include "system/block_device_info_db.h"
//#include "system/block_device.h"
//#include "common/thread_manager.h"
//#include "system/system_monitor.h"
//#include "system/system_monitor_thread.h"

//using namespace common::format;

//BlockDevInfoModel::BlockDevInfoModel(const TimePeriod &period, QObject *parent)
//    : QAbstractTableModel(parent)
//    , m_period(period)
//    , m_sumIO(new IOSample(period))
//    , m_avgIOPS(new IOPSSample(period))
//    , m_statModelDB {}
//    , m_infoDB {}
//    , m_nr {0}
//{
//    auto *monitor = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread)->systemMonitorInstance();
//    connect(monitor, &SystemMonitor::statInfoUpdated, this, &BlockDevInfoModel::updateModel);
//}

//QVariant BlockDevInfoModel::summaryStat(enum StatRole role) const
//{
//    switch (role) {
//    case kStatTotalRead: {
//        if (m_sumIO && m_sumIO->recentSample())
//            return formatUnit(m_sumIO->recentSample()->data.inBytes);

//        break;
//    }
//    case kStatTotalWrite: {
//        if (m_sumIO && m_sumIO->recentSample())
//            return formatUnit(m_sumIO->recentSample()->data.outBytes);

//        break;
//    }
//    case kStatReadSpeed: {
//        if (m_avgIOPS && m_avgIOPS->recentSample())
//            return formatUnit(m_avgIOPS->recentSample()->data.inBps, B, 1, true);

//        break;
//    }
//    case kStatWriteSpeed: {
//        if (m_avgIOPS && m_avgIOPS->recentSample())
//            return formatUnit(m_avgIOPS->recentSample()->data.outBps, B, 1, true);

//        break;
//    }
//    } // ::switch

//    return {};
//}

//int BlockDevInfoModel::rowCount(const QModelIndex &) const
//{
//    return m_infoDB.size();
//}

//int BlockDevInfoModel::columnCount(const QModelIndex &) const
//{
//    return kPropCount;
//}

//QVariant BlockDevInfoModel::data(const QModelIndex &idx, int role) const
//{
//    if (!idx.isValid())
//        return {};

//    int row = idx.row();
//    if (row < 0 || row >= m_infoDB.count())
//        return {};

//    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
//        switch (idx.column()) {
//        case kPropDevName: {
//            return m_infoDB[row].deviceName();
//        }
//        case kPropModel: {
//            return m_infoDB[row].model();
//        }
//        case kPropCapacity: {
//            return formatUnit(m_infoDB[row].capacity(), B, 0);
//        }
//        case kPropUsage: {
//            return QString("%1%2").arg(m_infoDB[row].percentUtilization() * 100).arg('%');
//        }
//        case kPropTps: {
//            return m_infoDB[row].transferPerSecond();
//        }
//        case kPropReadBps: {
//            auto devName = m_infoDB[row].deviceName();
//            if (m_statModelDB.contains(devName)) {
//                auto *model = m_statModelDB[devName].get();
//                if (model) {
//                    auto rBps = model->data(model->index(0, BlockDevStatModel::kStatReadSpeed));
//                    return rBps;
//                }
//            }

//            break;
//        }
//        case kPropWriteBps: {
//            auto devName = m_infoDB[row].deviceName();
//            if (m_statModelDB.contains(devName)) {
//                auto *model = m_statModelDB[devName].get();
//                if (model) {
//                    auto wBps = model->data(model->index(0, BlockDevStatModel::kStatWriteSpeed));
//                    return wBps;
//                }
//            }

//            break;
//        }
//        case kPropBlockRead: {
//            return m_infoDB[row].blocksRead();
//        }
//        case kPropBlockWrtn: {
//            return m_infoDB[row].blocksWritten();
//        }
//        case kPropTotalRead: {
//            return formatUnit(m_infoDB[row].bytesRead());
//        }
//        case kPropTotalWrtn: {
//            return formatUnit(m_infoDB[row].bytesWritten());
//        }
//        case kPropReadOpsSpeed: {
//            return m_infoDB[row].readRequestIssuedPerSecond();
//        }
//        case kPropWriteOpsSpeed: {
//            return m_infoDB[row].writeRequestIssuedPerSecond();
//        }
//        case kPropSectorsReadSpeed: {
//            return m_infoDB[row].sectorsReadPerSecond();
//        }
//        case kPropSectorsWriteSpeed: {
//            return m_infoDB[row].sectorsWrittenPerSecond();
//        }
//        case kPropReadsMergedSpeed: {
//            return m_infoDB[row].readRequestMergedPerSecond();
//        }
//        case kPropWritesMergedSpeed: {
//            return m_infoDB[row].writeRequestMergedPerSecond();
//        }
//        case kPropReadReqMergedRatio: {
//            return QString("%1%2").arg(m_infoDB[row].readRequestMergedPercent() * 100).arg('%');
//        }
//        case kPropWriteReqMergedRatio: {
//            return QString("%1%2").arg(m_infoDB[row].writeRequestMergedPercent() * 100).arg('%');
//        }
//        default:
//            break;
//        } // ::switch
//    } // ::if(displayRole || accessibleRole)
//    else if (role == kValueRole) {
//        switch (idx.column()) {
//        case kPropCapacity: {
//            return m_infoDB[row].capacity();
//        }
//        case kPropReadBps: {
//            auto devName = m_infoDB[row].deviceName();
//            if (m_statModelDB.contains(devName)) {
//                auto *model = m_statModelDB[devName].get();
//                if (model) {
//                    auto rBps = model->data(model->index(0, BlockDevStatModel::kStatReadSpeed),
//                                            BlockDevStatModel::kValueRole);
//                    return rBps;
//                }
//            }

//            break;
//        }
//        case kPropWriteBps: {
//            auto devName = m_infoDB[row].deviceName();
//            if (m_statModelDB.contains(devName)) {
//                auto *model = m_statModelDB[devName].get();
//                if (model) {
//                    auto wBps = model->data(model->index(0, BlockDevStatModel::kStatWriteSpeed),
//                                            BlockDevStatModel::kValueRole);
//                    return wBps;
//                }
//            }

//            break;
//        }
//        case kPropTotalRead: {
//            return m_infoDB[row].bytesRead();
//        }
//        case kPropTotalWrtn: {
//            return m_infoDB[row].bytesWritten();
//        }
//        default:
//            break;
//        } // ::switch
//    } // ::if(valueRole)

//    return {};
//}

//Qt::ItemFlags BlockDevInfoModel::flags(const QModelIndex &) const
//{
//    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
//}

//void BlockDevInfoModel::fetchMore(const QModelIndex &parent)
//{
//    if (parent.isValid())
//        return;

//    // fetch at most 32 items at a time
//    int left = m_infoDB.size() - m_nr;
//    int more = qMin(32, left);

//    if (more <= 0)
//        return;

//    beginInsertRows(QModelIndex(), m_nr, m_nr + more - 1);
//    m_nr += more;
//    endInsertRows();
//}

//bool BlockDevInfoModel::canFetchMore(const QModelIndex &parent) const
//{
//    if (parent.isValid())
//        return false;

//    return (m_nr < m_infoDB.size());
//}

//void BlockDevInfoModel::updateModel()
//{
//    auto *monitor = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread)->systemMonitorInstance();
//    beginResetModel();

//    m_infoDB = monitor->deviceDB()->blockDeviceInfoDB()->deviceList();

//    struct IO sumIO {
//    };

//    // updata stat data
//    for (auto &info : m_infoDB) {
//        if (!m_statModelDB.contains(info.deviceName())
//                || !m_statModelDB[info.deviceName()]) {
//            std::unique_ptr<BlockDevStatModel> tmp(new BlockDevStatModel(m_period, this));
//            m_statModelDB[info.deviceName()] = std::move(tmp);
//        }

//        auto *statModel = m_statModelDB[info.deviceName()].get();
//        if (statModel->m_ioSampleDB) {
//            struct IO io = {info.bytesRead(), info.bytesWritten()};
//            statModel->m_ioSampleDB->addSample(new IOSampleFrame(monitor->sysInfo()->uptime(), io));
//            sumIO += io;
//        }
//        if (statModel->m_iopsSampleDB) {
//            auto pair = statModel->m_ioSampleDB->recentSamplePair();
//            struct IOPS iops = IOSampleFrame::iops(pair.first, pair.second);
//            statModel->m_iopsSampleDB->addSample(new IOPSSampleFrame(iops));
//        }
//    } // ::for

//    // sum io & avg iops
//    if (m_sumIO) {
//        m_sumIO->addSample(new IOSampleFrame(monitor->sysInfo()->uptime(), sumIO));
//    }
//    if (m_avgIOPS) {
//        auto pair = m_sumIO->recentSamplePair();
//        struct IOPS iops = IOSampleFrame::iops(pair.first, pair.second);
//        m_avgIOPS->addSample(new IOPSSampleFrame(iops));
//    }

//    endResetModel();
//}
