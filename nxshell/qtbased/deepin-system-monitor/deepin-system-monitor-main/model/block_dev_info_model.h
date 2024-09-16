// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCK_DEV_INFO_MODEL_H
#define BLOCK_DEV_INFO_MODEL_H

//#include "common/sample.h"
//#include "system/block_device.h"

//#include <QAbstractTableModel>
//#include <QHash>
//#include <QList>

//#include <memory>

//using namespace common::core;
//using namespace core::system;

//class BlockDevStatModel;

///**
// * @brief The BlockDevInfoModel class
// */
//class BlockDevInfoModel : public QAbstractTableModel
//{
//    Q_OBJECT

//public:
//    enum Properties {
//        kPropDevName = 0,
//        kPropModel,
//        kPropCapacity,
//        kPropUsage,
//        kPropTps,
//        kPropReadBps,
//        kPropWriteBps,
//        kPropBlockRead,
//        kPropBlockWrtn,
//        kPropTotalRead,
//        kPropTotalWrtn,
//        kPropReadOpsSpeed,
//        kPropWriteOpsSpeed,
//        kPropSectorsReadSpeed,
//        kPropSectorsWriteSpeed,
//        kPropReadsMergedSpeed,
//        kPropWritesMergedSpeed,
//        kPropReadReqMergedRatio,
//        kPropWriteReqMergedRatio,

//        kPropCount
//    };

//    enum StatRole {
//        kStatTotalRead,
//        kStatTotalWrite,
//        kStatReadSpeed,
//        kStatWriteSpeed
//    };

//    enum DataRole {
//        kValueRole = Qt::UserRole + 0x0001
//    };

//    explicit BlockDevInfoModel(const TimePeriod &period, QObject *parent = nullptr);
//    virtual ~BlockDevInfoModel() override = default;

//    QVariant summaryStat(enum StatRole role) const;
//    std::weak_ptr<BlockDevStatModel> statModel(const QByteArray &name) const;

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

//    QHash<QByteArray, std::shared_ptr<BlockDevStatModel>> m_statModelDB;
//    QList<BlockDevice> m_infoDB;

//    int m_nr;
//};

//inline std::weak_ptr<BlockDevStatModel> BlockDevInfoModel::statModel(const QByteArray &name) const
//{
//    return m_statModelDB[name];
//}

#endif // BLOCK_DEV_INFO_MODEL_H
