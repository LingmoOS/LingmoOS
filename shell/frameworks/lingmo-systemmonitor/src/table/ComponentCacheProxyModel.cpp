/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ComponentCacheProxyModel.h"

#include <QTimer>

ComponentCacheAttached::ComponentCacheAttached(QObject *parent)
    : QObject(parent)
{
}

ComponentCacheProxyModel::ComponentCacheProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
{
}

ComponentCacheProxyModel::~ComponentCacheProxyModel()
{
}

QHash<int, QByteArray> ComponentCacheProxyModel::roleNames() const
{
    auto names = QIdentityProxyModel::roleNames();
    names.insert(CachedComponentRole, "cachedComponent");
    return names;
}

QVariant ComponentCacheProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (role == CachedComponentRole) {
        if (m_instances.contains(proxyIndex)) {
            return QVariant::fromValue(m_instances.value(proxyIndex));
        }

        // This bit of trickery is to get around the fact that we are in a const
        // method here but still want to create the instances only on demand.
        m_pendingInstances.append(proxyIndex);
        if (m_pendingInstances.size() == 1) {
            QTimer::singleShot(0, this, &ComponentCacheProxyModel::createPendingInstance);
        }
        return QVariant{};
    }

    return QIdentityProxyModel::data(proxyIndex, role);
}

void ComponentCacheProxyModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    if (sourceModel()) {
        sourceModel()->disconnect(this);
    }

    QIdentityProxyModel::setSourceModel(newSourceModel);

    if (newSourceModel) {
        connect(newSourceModel, &QAbstractItemModel::rowsRemoved, this, &ComponentCacheProxyModel::onRowsRemoved);
        connect(newSourceModel, &QAbstractItemModel::columnsRemoved, this, &ComponentCacheProxyModel::onColumnsRemoved);
        connect(newSourceModel, &QAbstractItemModel::modelReset, this, &ComponentCacheProxyModel::clear);
    }
}

QQmlComponent *ComponentCacheProxyModel::component() const
{
    return m_component;
}

void ComponentCacheProxyModel::setComponent(QQmlComponent *newComponent)
{
    if (newComponent == m_component) {
        return;
    }

    m_component = newComponent;
    clear();
    Q_EMIT componentChanged();
}

void ComponentCacheProxyModel::clear()
{
    qDeleteAll(m_instances);
    m_instances.clear();
}

void ComponentCacheProxyModel::onRowsRemoved(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row < end; ++row) {
        for (int column = 0; column < columnCount(); ++column) {
            m_instances.remove(index(row, column, parent));
        }
    }
}

void ComponentCacheProxyModel::onColumnsRemoved(const QModelIndex &parent, int start, int end)
{
    for (int column = start; column < end; ++column) {
        for (int row = 0; row < rowCount(); ++row) {
            m_instances.remove(index(row, column, parent));
        }
    }
}

void ComponentCacheProxyModel::createPendingInstance()
{
    if (!m_component) {
        return;
    }

    while (!m_pendingInstances.isEmpty()) {
        auto index = m_pendingInstances.takeFirst();
        if (!index.isValid()) {
            continue;
        }

        auto context = qmlContext(this);

        auto instance = m_component->beginCreate(context);
        instance->setParent(this);
        auto attached = static_cast<ComponentCacheAttached *>(qmlAttachedPropertiesObject<ComponentCacheProxyModel>(instance));
        attached->m_model = this;
        attached->m_row = index.row();
        attached->m_column = index.column();
        m_component->completeCreate();

        m_instances.insert(index, instance);
        Q_EMIT dataChanged(index, index, {CachedComponentRole});
    }
}

#include "moc_ComponentCacheProxyModel.cpp"
