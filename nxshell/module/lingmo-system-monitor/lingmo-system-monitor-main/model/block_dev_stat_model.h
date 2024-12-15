// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCK_DEV_STAT_MODEL_H
#define BLOCK_DEV_STAT_MODEL_H

#include "common/sample.h"
#include "common/time_period.h"

#include <QAbstractTableModel>

#include <memory>

using namespace common::core;

class BlockDevInfoModel;

/**
 * @brief Block device performance stat model
 */
class BlockDevStatModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum StatProp {
        kStatTotalRead,
        kStatTotalWrite,
        kStatReadSpeed,
        kStatWriteSpeed,

        kStatPropMax
    };

    enum DataRole {
        kValueRole = Qt::UserRole + 0x0001
    };

    /**
     * @brief Model constructor
     * @param parent Parent object
     */
    explicit BlockDevStatModel(const TimePeriod &period, QObject *parent = nullptr);
    virtual ~BlockDevStatModel() override = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    std::unique_ptr<IOSample> m_ioSampleDB;
    std::unique_ptr<IOPSSample> m_iopsSampleDB;

    friend class BlockDevInfoModel;
};

#endif // BLOCK_DEV_STAT_MODEL_H
