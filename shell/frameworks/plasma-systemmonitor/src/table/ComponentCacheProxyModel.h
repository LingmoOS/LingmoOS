/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QIdentityProxyModel>
#include <QQmlComponent>
#include <qqmlregistration.h>

/**
 * Attached property object for ComponentCacheProxyModel
 */
class ComponentCacheAttached : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model MEMBER m_model CONSTANT)
    Q_PROPERTY(int row MEMBER m_row CONSTANT)
    Q_PROPERTY(int column MEMBER m_column CONSTANT)
    QML_ANONYMOUS

public:
    explicit ComponentCacheAttached(QObject *parent = nullptr);

    QAbstractItemModel *m_model = nullptr;
    int m_row = -1;
    int m_column = -1;
};

/**
 * A proxy model that adds a cached component as an extra role.
 *
 * This proxy model will return an instance of \property component when data is
 * requested for the CachedComponentRole. If the instance does not yet exist,
 * it will be created.
 *
 * The referenced component can make use of the ComponentCacheAttached
 * attached property.
 *
 * This is mostly a helper for dealing with TableView and QSortFilterProxyModel,
 * which will cause some annoying reordering of cell delegates when sorting is
 * applied.
 */
class ComponentCacheProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QQmlComponent *component READ component WRITE setComponent NOTIFY componentChanged)
    QML_ELEMENT
    QML_ATTACHED(ComponentCacheAttached)

public:
    enum Roles {
        CachedComponentRole = Qt::UserRole + 88,
    };
    Q_ENUM(Roles)

    explicit ComponentCacheProxyModel(QObject *parent = nullptr);
    ~ComponentCacheProxyModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &proxyIndex, int role) const override;
    void setSourceModel(QAbstractItemModel *sourceModel) override;

    QQmlComponent *component() const;
    void setComponent(QQmlComponent *newComponent);
    Q_SIGNAL void componentChanged();

    Q_INVOKABLE void clear();

    static ComponentCacheAttached *qmlAttachedProperties(QObject *object)
    {
        return new ComponentCacheAttached(object);
    }

private:
    void onRowsRemoved(const QModelIndex &parent, int start, int end);
    void onColumnsRemoved(const QModelIndex &parent, int start, int end);
    void createPendingInstance();

    QQmlComponent *m_component = nullptr;
    QHash<QPersistentModelIndex, QObject *> m_instances;

    mutable QList<QPersistentModelIndex> m_pendingInstances;
};
