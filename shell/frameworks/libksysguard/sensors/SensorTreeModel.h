/*
    SPDX-FileCopyrightText: 2019 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "sensors_export.h"
#include <QAbstractItemModel>
#include <memory>
#include <qqmlregistration.h>

namespace KSysGuard
{
class SensorInfo;

/**
 * A model representing a tree of sensors that are available from the daemon.
 *
 * This model exposes the daemon's sensors as a tree, based on their path. Each
 * sensor is assumed to be structured in a format similar to
 * `category/object/sensor`. This model will then expose a tree, with `category`
 * as top level, `object` below it and finally `sensor` itself.
 */
class SENSORS_EXPORT SensorTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum AdditionalRoles {
        SensorId = Qt::UserRole + 1,
    };
    Q_ENUM(AdditionalRoles)

    explicit SensorTreeModel(QObject *parent = nullptr);
    ~SensorTreeModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant headerData(int section, Qt::Orientation, int role) const override;
    QStringList mimeTypes() const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

private:
    void init();
    void onSensorAdded(const QString &sensor);
    void onSensorRemoved(const QString &sensor);
    void onMetaDataChanged(const QString &sensorId, const SensorInfo &info);

    class Private;
    const std::unique_ptr<Private> d;
};

}
