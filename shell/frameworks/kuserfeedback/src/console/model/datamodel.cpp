/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "datamodel.h"

#include <rest/restapi.h>
#include <rest/restclient.h>
#include <core/sample.h>

#include <QDebug>
#include <QNetworkReply>

#include <algorithm>

using namespace KUserFeedback::Console;

static QString mapToString(const QVariantMap &map)
{
    QStringList l;
    l.reserve(map.size());
    for (auto it = map.begin(); it != map.end(); ++it) {
        if (it.value().type() == QVariant::Map)
            l.push_back(it.key() + QLatin1String(": {") + mapToString(it.value().toMap()) + QLatin1Char('}'));
        else
            l.push_back(it.key() + QLatin1String(": ") + it.value().toString());
    }
    return l.join(QLatin1String(", "));
}

static QString listToString(const QVariantList &list)
{
    QStringList l;
    l.reserve(list.size());
    for (const auto &v : list)
        l.push_back(mapToString(v.toMap()));

    return QLatin1String("[{") + l.join(QLatin1String("}, {")) + QLatin1String("}]");
}

QString DataModel::Column::name() const
{
    if (entry.dataType() == SchemaEntry::Scalar)
        return entry.name() + QLatin1Char('.') + element.name();
    return entry.name();
}

DataModel::DataModel(QObject *parent) : QAbstractTableModel(parent)
{
}

DataModel::~DataModel() = default;

void DataModel::setRESTClient(RESTClient* client)
{
    Q_ASSERT(client);
    m_restClient = client;
    connect(client, &RESTClient::clientConnected, this, &DataModel::reload);
    if (client->isConnected())
        reload();
}

Product DataModel::product() const
{
    return m_product;
}

void DataModel::setProduct(const Product& product)
{
    beginResetModel();
    m_product = product;
    m_columns.clear();
    for (const auto &entry : product.schema()) {
        if (entry.dataType() == SchemaEntry::Scalar) {
            for (const auto &elem : entry.elements())
                m_columns.push_back({entry, elem});
        } else {
            m_columns.push_back({entry, {}});
        }
    }
    m_data.clear();
    reload();
    endResetModel();
}

void DataModel::setSamples(const QVector<Sample> &samples)
{
    beginResetModel();
    m_data = samples;
    std::sort(m_data.begin(), m_data.end(), [](const Sample &lhs, const Sample &rhs) {
        return lhs.timestamp() < rhs.timestamp();
    });
    endResetModel();
}

void DataModel::reload()
{
    if (!m_restClient || !m_restClient->isConnected() || !m_product.isValid())
        return;

    auto reply = RESTApi::listSamples(m_restClient, m_product);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            const auto samples = Sample::fromJson(reply->readAll(), m_product);
            setSamples(samples);
        }
        reply->deleteLater();
    });
}

int DataModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_columns.size() + 1;
}

int DataModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_data.size();
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (role == Qt::DisplayRole) {
        const auto sample = m_data.at(index.row());
        if (index.column() == 0)
            return sample.timestamp();
        const auto col = m_columns.at(index.column() - 1);
        const auto v = sample.value(col.name());
        switch (col.entry.dataType()) {
            case SchemaEntry::Scalar:
                return v;
            case SchemaEntry::List:
                return listToString(v.toList());
            case SchemaEntry::Map:
                return mapToString(v.toMap());
        }
    } else if (role == SampleRole) {
        return QVariant::fromValue(m_data.at(index.row()));
    } else if (role == AllSamplesRole) {
        return QVariant::fromValue(m_data);
    }

    return {};
}

QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && m_product.isValid()) {
        if (section == 0)
            return tr("Timestamp");
        const auto col = m_columns.at(section - 1);
        return QString(col.name());
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

#include "moc_datamodel.cpp"
