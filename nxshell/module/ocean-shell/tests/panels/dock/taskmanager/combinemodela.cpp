// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "combinemodela.h"

DataA::DataA(int id, TestModelA* parent)
    : m_id(id)
{
}

DataA::DataA(int id, const QString &data, TestModelA* model)
    : DataA(id, model)
{
    m_data = data;
}

int DataA::id()
{
    return m_id;
}

QString DataA::data()
{
    return m_data;
}

void DataA::setData(const QString &data)
{
    if (data == m_data) return;
    m_data = data;

    auto index = m_model->match(QModelIndex(), TestModelA::idRole, m_id);
    if (index.size() > 0) {
        Q_EMIT m_model->dataChanged(index.first(), index.last(), {TestModelA::dataRole});
    }
}

TestModelA::TestModelA(QObject *parent)
    : QAbstractListModel(parent)
{

}

QHash<int, QByteArray> TestModelA::roleNames() const
{
    return {
        {idRole, "aId"},
        {dataRole, "aData"}
    };
}

int TestModelA::rowCount(const QModelIndex &parent) const
{
    return m_list.size();
}

QVariant TestModelA::data(const QModelIndex &index, int role) const
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

void TestModelA::addData(DataA *data)
{
    beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
    m_list.append(data);
    endInsertRows();
}

void TestModelA::removeData(DataA *data)
{
    auto pos = m_list.indexOf(data);
    if (pos == -1) return;

    beginRemoveRows(QModelIndex(), pos, pos);
    m_list.removeAt(pos);
    delete data;
    endRemoveRows();
}
