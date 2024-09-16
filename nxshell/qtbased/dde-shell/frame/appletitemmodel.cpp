// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appletitemmodel.h"

DS_BEGIN_NAMESPACE

DAppletItemModel::DAppletItemModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

DAppletItemModel::~DAppletItemModel()
{
}

QList<QObject *> DAppletItemModel::rootObjects() const
{
    return m_rootObjects;
}

void DAppletItemModel::append(QObject *rootObject)
{
    const auto index = m_rootObjects.size();
    beginInsertRows(QModelIndex(), index, index);
    m_rootObjects.append(rootObject);
    endInsertRows();
}

void DAppletItemModel::remove(QObject *rootObject)
{
    const auto index = m_rootObjects.indexOf(rootObject);
    if (index < 0)
        return;
    beginRemoveRows(QModelIndex(), index, index);
    m_rootObjects.removeOne(rootObject);
    endRemoveRows();
}

int DAppletItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_rootObjects.count();
}

QVariant DAppletItemModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (row >= m_rootObjects.size() || !index.isValid())
        return {};

    switch (role) {
    case DAppletItemModel::Data:
        return QVariant::fromValue<QObject *>(m_rootObjects[row]);
    default:
        break;
    }
    return {};
}

QHash<int, QByteArray> DAppletItemModel::roleNames() const
{
    QHash<int, QByteArray> mapRoleNames;
    mapRoleNames[DAppletItemModel::Data] = "data";
    return mapRoleNames;
}

DS_END_NAMESPACE
