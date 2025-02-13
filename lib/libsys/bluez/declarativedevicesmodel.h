/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef DECLARATIVEDEVICESMODEL_H
#define DECLARATIVEDEVICESMODEL_H

#include <QSortFilterProxyModel>

#include "declarativemanager.h"

#include <BluezQt/DevicesModel>

class DeclarativeDevicesModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(DeclarativeManager *manager READ manager WRITE setManager)

public:
    enum DeclarativeDeviceRoles {
        DeviceRole = BluezQt::DevicesModel::LastRole + 1,
        AdapterRole = BluezQt::DevicesModel::LastRole + 2,
        MediaPlayerRole = BluezQt::DevicesModel::LastRole + 3,
        // BatteryRole = BluezQt::DevicesModel::LastRole + 4,
    };

    explicit DeclarativeDevicesModel(QObject *parent = nullptr);

    DeclarativeManager *manager() const;
    void setManager(DeclarativeManager *manager);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    DeclarativeManager *m_manager;
    BluezQt::DevicesModel *m_model;
};

#endif // DECLARATIVEMANAGER_H
