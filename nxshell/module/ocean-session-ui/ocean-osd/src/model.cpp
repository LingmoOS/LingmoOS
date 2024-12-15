// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model.h"

#include <QDebug>

#include "abstractosdprovider.h"

Model::Model(QObject *parent)
    : QAbstractListModel(parent),
      m_provider(nullptr)
{

}

int Model::rowCount(const QModelIndex &parent) const
{
    if (!m_provider) return 0;

    return m_provider->rowCount(parent);
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    if (!m_provider) return QVariant();

    return m_provider->data(index, role);
}

void Model::setProvider(AbstractOSDProvider *provider)
{
    beginResetModel();
    m_provider = provider;
    endResetModel();
}

AbstractOSDProvider *Model::provider()
{
    return m_provider;
}
