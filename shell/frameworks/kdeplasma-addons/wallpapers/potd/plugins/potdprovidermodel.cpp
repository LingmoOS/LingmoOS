/*
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "potdprovidermodel.h"

PotdProviderModel::PotdProviderModel(QObject *parent)
    : QAbstractListModel(parent)
{
    loadPluginMetaData();
}

int PotdProviderModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_providers.size();
}

QVariant PotdProviderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_providers.size())) {
        return QVariant();
    }

    const KPluginMetaData &item = m_providers.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return item.name();
    case Qt::DecorationRole:
        return item.iconName();
    case Roles::Id:
        return item.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier"));
    case Roles::NotSafeForWork: {
        return item.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-NotSafeForWork"), false);
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> PotdProviderModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
        {Qt::DecorationRole, "decoration"},
        {Roles::Id, "id"},
        {Roles::NotSafeForWork, "notSafeForWork"},
    };
}

int PotdProviderModel::indexOf(const QString &identifier)
{
    auto it = std::find_if(m_providers.cbegin(), m_providers.cend(), [&identifier](const KPluginMetaData &metadata) {
        return identifier == metadata.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier"));
    });

    if (it == m_providers.cend()) {
        return 0;
    }

    return std::distance(m_providers.cbegin(), it);
}

bool PotdProviderModel::isNSFW(int row)
{
    return index(row, 0).data(Roles::NotSafeForWork).toBool();
}

void PotdProviderModel::loadPluginMetaData()
{
    const QList<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("potd"));

    beginResetModel();

    m_providers.clear();
    m_providers.reserve(plugins.size());

    std::copy_if(plugins.cbegin(), plugins.cend(), std::back_inserter(m_providers), [](const KPluginMetaData &metadata) {
        return !metadata.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier")).isEmpty();
    });

    endResetModel();
}
