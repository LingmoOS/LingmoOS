// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "combinemodelb.h"

DataB::DataB(int id, TestModelB* parent)
    : m_id(id)
{
}

DataB::DataB(int id, const QString &data, TestModelB* model)
    : DataB(id, model)
{
    m_data = data;
}

int DataB::id()
{
    return m_id;
}

QString DataB::data()
{
    return m_data;
}

void DataB::setData(const QString &data)
{
    if (data == m_data) return;
    m_data = data;

    auto index = m_model->match(QModelIndex(), TestModelB::idRole, m_id);
    if (index.size() > 0) {
        Q_EMIT m_model->dataChanged(index.first(), index.last(), {TestModelB::dataRole});
    }
}

TestModelB::TestModelB(QObject *parent)
    : QAbstractListModel(parent)
{

}

QHash<int, QByteArray> TestModelB::roleNames() const
{
    return {
        {idRole, "bId"},
        {dataRole, "bData"}
    };
}

int TestModelB::rowCount(const QModelIndex &parent) const
{
    return m_list.size();
}

QVariant TestModelB::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_list.size()) return {};
    switch (role) {
        case idRole:
            return m_list[index.row()]->id();
        case dataRole:
            return m_list[index.row()]->data();
    }

    return {};
}

void TestModelB::addData(DataB *data)
{
    beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
    m_list.append(data);
    endInsertRows();
}

void TestModelB::removeData(DataB *data)
{
    auto pos = m_list.indexOf(data);
    if (pos == -1) return;

    beginRemoveRows(QModelIndex(), pos, pos);
    m_list.removeAt(pos);
    delete data;
    endRemoveRows();
}
