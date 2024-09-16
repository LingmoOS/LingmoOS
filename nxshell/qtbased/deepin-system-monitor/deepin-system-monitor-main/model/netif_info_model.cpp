// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netif_info_model.h"

#include "common/common.h"
#include "netif_stat_model.h"
#include "netif_addr_model.h"
#include "system/system_monitor.h"
#include "system/device_db.h"
#include "system/netif_info_db.h"
#include "system/netif.h"
#include "system/nl_link.h"
#include "system/sys_info.h"
#include "common/thread_manager.h"
#include "system/system_monitor.h"
#include "system/system_monitor_thread.h"

//using namespace common::format;

//NetifInfoModel::NetifInfoModel(const TimePeriod &period, QObject *parent)
//    : QAbstractTableModel(parent)
//    , m_period(period)
//    , m_sumIO(new IOSample(period))
//    , m_avgIOPS(new IOPSSample(period))
//    , m_statModelDB {}
//    , m_addrModelDB {}
//    , m_infoDB {}
//    , m_nr {0}
//{
//    auto *monitor = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread)->systemMonitorInstance();
//    connect(monitor, &SystemMonitor::statInfoUpdated, this, &NetifInfoModel::updateModel);
//}

//QVariant NetifInfoModel::summaryStat(enum StatRole role) const
//{
//    switch (role) {
//    case kStatTotalRecv: {
//        if (m_sumIO && m_sumIO->recentSample())
//            return formatUnit(m_sumIO->recentSample()->data.inBytes);

//        break;
//    }
//    case kStatTotalSent: {
//        if (m_sumIO && m_sumIO->recentSample())
//            return formatUnit(m_sumIO->recentSample()->data.outBytes);

//        break;
//    }
//    case kStatRecvSpeed: {
//        if (m_avgIOPS && m_avgIOPS->recentSample())
//            return formatUnit(m_avgIOPS->recentSample()->data.inBps, B, 1, true);

//        break;
//    }
//    case kStatSentSpeed: {
//        if (m_avgIOPS && m_avgIOPS->recentSample())
//            return formatUnit(m_avgIOPS->recentSample()->data.outBps, B, 1, true);

//        break;
//    }
//    default:
//        break;
//    } // ::switch

//    return {};
//}

//int NetifInfoModel::rowCount(const QModelIndex &) const
//{
//    return m_infoDB.size();
//}

//int NetifInfoModel::columnCount(const QModelIndex &) const
//{
//    return kPropCount;
//}

//QVariant NetifInfoModel::data(const QModelIndex &idx, int role) const
//{
//    if (!idx.isValid())
//        return {};

//    auto row = idx.row();
//    if (row < 0 || row >= m_infoDB.size())
//        return {};

//    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
//        switch (idx.column()) {
//        case kPropDevName: {
//            return m_infoDB[row].ifname();
//        }
//        case kPropIndex: {
//            return m_infoDB[row].index();
//        }
//        case kPropBrand: {
//            return m_infoDB[row].brand();
//        }
//        case kPropEther: {
//            return m_infoDB[row].linkAddress();
//        }
//        case kPropConnection: { // ethernet or wifi
//            return m_infoDB[row].connectionType();
//        }
//        case kPropFlags: {
//            return NLLink::formatFlags(m_infoDB[row].flags());
//        }
//        case kPropESSID: {
//            return m_infoDB[row].essid();
//        }
//        case kPropRecvBps: {
//            auto iface = m_infoDB[row].ifname();
//            if (m_statModelDB.contains(iface)) {
//                auto *model = m_statModelDB[iface].get();
//                if (model) {
//                    auto rBps = model->data(index(0, NetifStatModel::kStatRecvSpeed));
//                    return rBps;
//                }
//            }

//            break;
//        }
//        case kPropSentBps: {
//            auto iface = m_infoDB[row].ifname();
//            if (m_statModelDB.contains(iface)) {
//                auto *model = m_statModelDB[iface].get();
//                if (model) {
//                    auto wBps = model->data(index(0, NetifStatModel::kStatSentSpeed));
//                    return wBps;
//                }
//            }

//            break;
//        }
//        case kPropLinkQuality: {
//            return m_infoDB[row].linkQuality();
//        }
//        case kPropSignalLevel: {
//            return m_infoDB[row].signalLevel();
//        }
//        case kPropNoiseLevel: {
//            return m_infoDB[row].noiseLevel();
//        }
//        case kPropRxPackets: {
//            return m_infoDB[row].rxPackets();
//        }
//        case kPropTxPackets: {
//            return m_infoDB[row].txPackets();
//        }
//        case kPropRxBytes: {
//            return formatUnit(m_infoDB[row].rxBytes());
//        }
//        case kPropTxBytes: {
//            return formatUnit(m_infoDB[row].txBytes());
//        }
//        case kPropRxErrors: {
//            return m_infoDB[row].rxErrors();
//        }
//        case kPropTxErrors: {
//            return m_infoDB[row].txErrors();
//        }
//        case kPropRxDropped: {
//            return m_infoDB[row].rxDropped();
//        }
//        case kPropTxDropped: {
//            return m_infoDB[row].txDropped();
//        }
//        case kPropRxOverruns: {
//            return m_infoDB[row].rxFIFO();
//        }
//        case kPropTxOverruns: {
//            return m_infoDB[row].txFIFO();
//        }
//        case kPropRxFrame: {
//            return m_infoDB[row].rxFrame();
//        }
//        case kPropTxCarrier: {
//            return m_infoDB[row].txCarrier();
//        }
//        case kPropCollisions: {
//            return m_infoDB[row].collisions();
//        }
//        default:
//            break;
//        } // ::switch
//    } // ::if(displayRole || accessibleRole)
//    else if (role == kValueRole) {
//        switch (idx.column()) {
//        case kPropRecvBps: {
//            auto iface = m_infoDB[row].ifname();
//            if (m_statModelDB.contains(iface)) {
//                auto *model = m_statModelDB[iface].get();
//                if (model) {
//                    auto rBps = model->data(index(0, NetifStatModel::kStatRecvSpeed),
//                                            NetifInfoModel::kValueRole);
//                    return rBps;
//                }
//            }

//            break;
//        }
//        case kPropSentBps: {
//            auto iface = m_infoDB[row].ifname();
//            if (m_statModelDB.contains(iface)) {
//                auto *model = m_statModelDB[iface].get();
//                if (model) {
//                    auto wBps = model->data(index(0, NetifStatModel::kStatSentSpeed),
//                                            NetifInfoModel::kValueRole);
//                    return wBps;
//                }
//            }

//            break;
//        }
//        case kPropRxBytes: {
//            return m_infoDB[row].rxBytes();
//        }
//        case kPropTxBytes: {
//            return m_infoDB[row].txBytes();
//        }
//        default:
//            break;
//        } // ::switch
//    } // ::if(valueRole)

//    return {};
//}

//Qt::ItemFlags NetifInfoModel::flags(const QModelIndex &) const
//{
//    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
//}

//void NetifInfoModel::fetchMore(const QModelIndex &parent)
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

//bool NetifInfoModel::canFetchMore(const QModelIndex &parent) const
//{
//    if (parent.isValid())
//        return false;

//    return (m_nr < m_infoDB.size());
//}

//void NetifInfoModel::updateModel()
//{
//    auto monitor = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread)->systemMonitorInstance();
//    auto devDB = monitor->deviceDB();
//    auto db = devDB->netifInfoDB();

//    if (db) {
//        beginResetModel();

//        m_infoDB = db->infoDB();

//        struct IO sumIO {
//        };

//        for (auto &info : m_infoDB) {
//            // stats db
//            if (!m_statModelDB.contains(info.ifname())
//                    || !m_statModelDB[info.ifname()]) {
//                std::unique_ptr<NetifStatModel> tmp(new NetifStatModel(m_period, this));
//                m_statModelDB[info.ifname()] = std::move(tmp);
//            }

//            auto *statModel = m_statModelDB[info.ifname()].get();
//            if (statModel->m_ioSampleDB) {
//                struct IO io = {info.txBytes(), info.txBytes()};
//                statModel->m_ioSampleDB->addSample(new IOSampleFrame(monitor->sysInfo()->uptime(), io));
//                sumIO += io;
//            }
//            if (statModel->m_iopsSampleDB) {
//                auto pair = statModel->m_ioSampleDB->recentSamplePair();
//                struct IOPS iops = IOSampleFrame::iops(pair.first, pair.second);
//                statModel->m_iopsSampleDB->addSample(new IOPSSampleFrame(iops));
//            }

//            // addrs db
//            if (!m_addrModelDB.contains(info.ifname()) || !m_addrModelDB[info.ifname()]) {
//                std::unique_ptr<NetifAddrModel> tmp(new NetifAddrModel(this));
//                m_addrModelDB[info.ifname()] = std::move(tmp);
//            }

//            auto addrModel = m_addrModelDB[info.ifname()].get();
//            auto addrList = db->addrList(info.ifname());
//            for (auto &addr : addrList) {
//                if (addr->family == AF_INET)
//                    addrModel->m_addr4DB << addr;
//                else if (addr->family == AF_INET6)
//                    addrModel->m_addr6DB << addr;
//                else
//                    addrModel->m_addrDB << addr;
//            }
//        } // ::for

//        // sum io & avg iops
//        if (m_sumIO) {
//            m_sumIO->addSample(new IOSampleFrame(monitor->sysInfo()->uptime(), sumIO));
//        }
//        if (m_avgIOPS) {
//            auto pair = m_sumIO->recentSamplePair();
//            struct IOPS iops = IOSampleFrame::iops(pair.first, pair.second);
//            m_avgIOPS->addSample(new IOPSSampleFrame(iops));
//        }

//        endResetModel();
//    } // ::netifInfoDB
//} // ::updateModel
