/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpluginmodel.h"
#include "kpluginproxymodel.h"

#include <QPluginLoader>

#include <KCategorizedSortFilterProxyModel>
#include <KConfigGroup>

#include <utility>

#include "kcmutilscore_debug.h"

class KPluginModelPrivate
{
public:
    bool isDefaulted()
    {
        return std::all_of(m_plugins.cbegin(), m_plugins.cend(), [this](const KPluginMetaData &data) {
            return isPluginEnabled(data) == data.isEnabledByDefault();
        });
    }
    bool isPluginEnabled(const KPluginMetaData &plugin) const
    {
        auto pendingState = m_pendingStates.constFind(plugin.pluginId());
        if (pendingState != m_pendingStates.constEnd()) {
            return pendingState.value();
        }

        if (m_config.isValid()) {
            return m_config.readEntry(plugin.pluginId() + QLatin1String("Enabled"), plugin.isEnabledByDefault());
        }
        return plugin.isEnabledByDefault();
    }
    KPluginMetaData findConfig(const KPluginMetaData &plugin) const
    {
        const QString metaDataKCM = plugin.value(QStringLiteral("X-KDE-ConfigModule"));

        if (!metaDataKCM.isEmpty()) {
            const QString absoluteKCMPath = QPluginLoader(metaDataKCM).fileName();
            // If we have a static plugin the file does not exist on disk
            // instead we query in the plugin namespace
            if (absoluteKCMPath.isEmpty()) {
                const int idx = metaDataKCM.lastIndexOf(QLatin1Char('/'));
                const QString pluginNamespace = metaDataKCM.left(idx);
                const QString pluginId = metaDataKCM.mid(idx + 1);
                return KPluginMetaData::findPluginById(pluginNamespace, pluginId);
            } else {
                return KPluginMetaData(plugin.rawData(), absoluteKCMPath);
            }
        }

        return KPluginMetaData();
    }

    QList<KPluginMetaData> m_plugins;
    QSet<KPluginMetaData> m_unsortablePlugins;
    QHash<QString, KPluginMetaData> m_pluginKcms;
    KConfigGroup m_config;
    QList<QString> m_orderedCategories; // Preserve order of categories in which they were added
    QHash<QString, QString> m_categoryLabels;
    QHash<QString, bool> m_pendingStates;
};

KPluginModel::KPluginModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new KPluginModelPrivate())
{
}

KPluginModel::~KPluginModel() = default;

QVariant KPluginModel::data(const QModelIndex &index, int role) const
{
    const KPluginMetaData &plugin = d->m_plugins[index.row()];

    switch (role) {
    case Roles::NameRole:
        return plugin.name();
    case Roles::DescriptionRole:
        return plugin.description();
    case Roles::IconRole:
        return plugin.iconName();
    case Roles::EnabledRole:
        return d->isPluginEnabled(plugin);
    case Roles::IsChangeableRole:
        if (d->m_unsortablePlugins.contains(plugin)) {
            return false;
        }
        if (d->m_config.isValid()) {
            return !d->m_config.isEntryImmutable(plugin.pluginId() + QLatin1String("Enabled"));
        }
        return true;
    case MetaDataRole:
        return QVariant::fromValue(plugin);
    case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
    case KCategorizedSortFilterProxyModel::CategorySortRole:
        return d->m_categoryLabels[plugin.pluginId()];
    case ConfigRole:
        return QVariant::fromValue(d->m_pluginKcms.value(plugin.pluginId()));
    case IdRole:
        return plugin.pluginId();
    case EnabledByDefaultRole:
        return plugin.isEnabledByDefault();
    case SortableRole:
        return !d->m_unsortablePlugins.contains(plugin);
    }

    return {};
}

bool KPluginModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Roles::EnabledRole) {
        const QString pluginId = d->m_plugins[index.row()].pluginId();

        // If we already have a pending state and the user reverts it remove it from the map
        auto pendingStateIt = d->m_pendingStates.constFind(pluginId);
        if (pendingStateIt != d->m_pendingStates.constEnd()) {
            if (pendingStateIt.value() != value.toBool()) {
                d->m_pendingStates.erase(pendingStateIt);
            }
        } else {
            d->m_pendingStates[pluginId] = value.toBool();
        }

        Q_EMIT dataChanged(index, index, {Roles::EnabledRole});
        Q_EMIT defaulted(d->isDefaulted());
        Q_EMIT isSaveNeededChanged();

        return true;
    }

    return false;
}

int KPluginModel::rowCount(const QModelIndex & /*parent*/) const
{
    return d->m_plugins.count();
}

QHash<int, QByteArray> KPluginModel::roleNames() const
{
    return {
        {KCategorizedSortFilterProxyModel::CategoryDisplayRole, "category"},
        {Roles::NameRole, "name"},
        {Roles::IconRole, "icon"},
        {Roles::EnabledRole, "enabled"},
        {Roles::DescriptionRole, "description"},
        {Roles::IsChangeableRole, "changable"},
        {Roles::EnabledByDefaultRole, "enabledByDefault"},
        {Roles::MetaDataRole, "metaData"},
        {Roles::ConfigRole, "config"},
    };
};

void KPluginModel::addUnsortablePlugins(const QList<KPluginMetaData> &newPlugins, const QString &categoryLabel)
{
    d->m_unsortablePlugins.unite(QSet(newPlugins.begin(), newPlugins.end()));
    addPlugins(newPlugins, categoryLabel);
}

bool KPluginModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent.isValid() || destinationParent.isValid()) {
        return false;
    }
    if ((sourceRow + count - 1) >= d->m_plugins.size()) {
        return false;
    }

    const bool isMoveDown = destinationChild > sourceRow;
    if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, isMoveDown ? destinationChild + 1 : destinationChild)) {
        return false;
    }
    for (int i = 0; i < count; i++) {
        d->m_plugins.insert(destinationChild, d->m_plugins.takeAt(sourceRow + i));
    }
    endMoveRows();
    return true;
}

void KPluginModel::addPlugins(const QList<KPluginMetaData> &newPlugins, const QString &categoryLabel)
{
    beginInsertRows({}, d->m_plugins.size(), d->m_plugins.size() + newPlugins.size() - 1);
    d->m_orderedCategories << categoryLabel;
    d->m_plugins.append(newPlugins);

    for (const KPluginMetaData &plugin : newPlugins) {
        d->m_categoryLabels[plugin.pluginId()] = categoryLabel;
        d->m_pluginKcms.insert(plugin.pluginId(), d->findConfig(plugin));
    }

    endInsertRows();

    Q_EMIT defaulted(d->isDefaulted());
}

void KPluginModel::removePlugin(const KPluginMetaData &data)
{
    if (const int index = d->m_plugins.indexOf(data); index != -1) {
        beginRemoveRows({}, index, index);
        d->m_plugins.removeAt(index);
        d->m_unsortablePlugins.remove(data);
        endRemoveRows();
    }
}

void KPluginModel::setConfig(const KConfigGroup &config)
{
    d->m_config = config;

    if (!d->m_plugins.isEmpty()) {
        Q_EMIT dataChanged(index(0, 0), index(d->m_plugins.size() - 1, 0), {Roles::EnabledRole, Roles::IsChangeableRole});
    }
}

void KPluginModel::clear()
{
    if (d->m_plugins.isEmpty()) {
        return;
    }
    beginRemoveRows({}, 0, d->m_plugins.size() - 1);
    d->m_plugins.clear();
    d->m_pluginKcms.clear();
    // In case of the "Reset"-button of the KCMs load is called again with the goal
    // of discarding all local changes. Consequently, the pending states have to be cleared here.
    d->m_pendingStates.clear();
    endRemoveRows();
}

void KPluginModel::save()
{
    if (d->m_config.isValid()) {
        for (auto it = d->m_pendingStates.cbegin(); it != d->m_pendingStates.cend(); ++it) {
            d->m_config.writeEntry(it.key() + QLatin1String("Enabled"), it.value());
        }

        d->m_config.sync();
    }
    d->m_pendingStates.clear();
}

KPluginMetaData KPluginModel::findConfigForPluginId(const QString &pluginId) const
{
    for (const KPluginMetaData &plugin : std::as_const(d->m_plugins)) {
        if (plugin.pluginId() == pluginId) {
            return d->findConfig(plugin);
        }
    }
    return KPluginMetaData();
}

void KPluginModel::load()
{
    if (!d->m_config.isValid()) {
        return;
    }

    d->m_pendingStates.clear();
    Q_EMIT dataChanged(index(0, 0), index(d->m_plugins.size() - 1, 0), {Roles::EnabledRole});
}

void KPluginModel::defaults()
{
    for (int pluginIndex = 0, count = d->m_plugins.count(); pluginIndex < count; ++pluginIndex) {
        const KPluginMetaData plugin = d->m_plugins.at(pluginIndex);
        const bool changed = d->isPluginEnabled(plugin) != plugin.isEnabledByDefault();

        if (changed) {
            // If the entry was marked as changed, but we flip the value it is unchanged again
            if (d->m_pendingStates.remove(plugin.pluginId()) == 0) {
                // If the entry was not changed before, we have to mark it as changed
                d->m_pendingStates.insert(plugin.pluginId(), plugin.isEnabledByDefault());
            }
            Q_EMIT dataChanged(index(pluginIndex, 0), index(pluginIndex, 0), {Roles::EnabledRole});
        }
    }

    Q_EMIT defaulted(true);
}

bool KPluginModel::isSaveNeeded()
{
    return !d->m_pendingStates.isEmpty();
}

QStringList KPluginModel::getOrderedCategoryLabels()
{
    return d->m_orderedCategories;
}

#include "moc_kpluginmodel.cpp"
