// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_INFO_MODEL_H
#define NETIF_INFO_MODEL_H

#include "common/sample.h"

#include <QAbstractTableModel>
#include <QMap>
#include <QList>

//namespace core {
//namespace system {
//class NetifInfo;
//}
//} // namespace core

//class NetifStatModel;
//class NetifAddrModel;

//using namespace common::core;
//using namespace core::system;

//class NetifInfoModel : public QAbstractTableModel
//{
//    Q_OBJECT

//public:
//    enum Properties {
//        kPropDevName = 0,
//        kPropIndex,
//        kPropBrand,
//        kPropEther,
//        kPropConnection,
//        kPropFlags,
//        kPropESSID,
//        kPropRecvBps,
//        kPropSentBps,
//        kPropLinkQuality,
//        kPropSignalLevel,
//        kPropNoiseLevel,
//        kPropRxPackets,
//        kPropTxPackets,
//        kPropRxBytes,
//        kPropTxBytes,
//        kPropRxErrors,
//        kPropTxErrors,
//        kPropRxDropped,
//        kPropTxDropped,
//        kPropRxOverruns,
//        kPropTxOverruns,
//        kPropRxFrame,
//        kPropTxCarrier,
//        kPropCollisions,

//        kPropCount
//    };

//    enum StatRole {
//        kStatTotalRecv,
//        kStatTotalSent,
//        kStatRecvSpeed,
//        kStatSentSpeed,

//        kStatRoleMax
//    };

//    enum DataRole {
//        kValueRole = Qt::UserRole + 0x0001
//    };

//    explicit NetifInfoModel(const TimePeriod &period, QObject *parent = nullptr);
//    virtual ~NetifInfoModel() override = default;

//    QVariant summaryStat(enum StatRole role) const;
//    std::weak_ptr<NetifStatModel> statModel(const QByteArray &iface) const;
//    std::weak_ptr<NetifAddrModel> addrModel(const QByteArray &iface) const;

//    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
//    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
//    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
//    Qt::ItemFlags flags(const QModelIndex &index) const override;
//    void fetchMore(const QModelIndex &parent) override;
//    bool canFetchMore(const QModelIndex &parent) const override;

//public slots:
//    void updateModel();

//private:
//    TimePeriod m_period;

//    std::unique_ptr<IOSample> m_sumIO;
//    std::unique_ptr<IOPSSample> m_avgIOPS;

//    QMap<QByteArray, std::shared_ptr<NetifStatModel>> m_statModelDB;
//    QMap<QByteArray, std::shared_ptr<NetifAddrModel>> m_addrModelDB;

//    QMap<QByteArray,NetifInfo> m_infoDB;

//    int m_nr;
//};

//inline std::weak_ptr<NetifStatModel> NetifInfoModel::statModel(const QByteArray &iface) const
//{
//    return m_statModelDB[iface];
//}

//inline std::weak_ptr<NetifAddrModel> NetifInfoModel::addrModel(const QByteArray &iface) const
//{
//    return m_addrModelDB[iface];
//}

#endif // NETIF_INFO_MODEL_H
