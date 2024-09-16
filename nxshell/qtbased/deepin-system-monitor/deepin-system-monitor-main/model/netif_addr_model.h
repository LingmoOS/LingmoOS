// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_ADDR_MODEL_H
#define NETIF_ADDR_MODEL_H

#include "system/netif_info_db.h"
#include "system/netif.h"

#include <QAbstractTableModel>

using namespace core::system;

class NetifInfoModel;

class NetifAddrModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum AddrType {
        kINet4Addr,
        kINet6Addr,
        kOtherAddr,

        kAddrTypeMax
    };

    explicit NetifAddrModel(QObject *parent = nullptr);
    virtual ~NetifAddrModel() override = default;

    int inet4AddrCount() const;
    int inet6AddrCount() const;
    int otherAddrCount() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QList<INetAddr> m_addr4DB; // inet
    QList<INetAddr> m_addr6DB; // inet4
    QList<INetAddr> m_addrDB; // others

    friend class NetifInfoModel;
};

inline int NetifAddrModel::inet4AddrCount() const
{
    return m_addr4DB.size();
}

inline int NetifAddrModel::inet6AddrCount() const
{
    return m_addr6DB.size();
}

inline int NetifAddrModel::otherAddrCount() const
{
    return m_addrDB.size();
}

#endif // NETIF_ADDR_MODEL_H
