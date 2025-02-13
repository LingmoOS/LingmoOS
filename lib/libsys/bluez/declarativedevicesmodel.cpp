/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "declarativedevicesmodel.h"
#include "declarativeadapter.h"
// #include "declarativebattery.h"
#include "declarativedevice.h"
#include "declarativemanager.h"
#include "declarativemediaplayer.h"

DeclarativeDevicesModel::DeclarativeDevicesModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_manager(nullptr)
    , m_model(nullptr)
{
}

DeclarativeManager *DeclarativeDevicesModel::manager() const
{
    return m_manager;
}

void DeclarativeDevicesModel::setManager(DeclarativeManager *manager)
{
    m_manager = manager;
    m_model = new BluezQt::DevicesModel(m_manager, this);
    setSourceModel(m_model);
}

QHash<int, QByteArray> DeclarativeDevicesModel::roleNames() const
{
    QHash<int, QByteArray> roles = QSortFilterProxyModel::roleNames();

    roles[DeviceRole] = QByteArrayLiteral("Device");
    roles[AdapterRole] = QByteArrayLiteral("Adapter");
    roles[MediaPlayerRole] = QByteArrayLiteral("MediaPlayer");
    // roles[BatteryRole] = QByteArrayLiteral("Battery");

    return roles;
}

QVariant DeclarativeDevicesModel::data(const QModelIndex &index, int role) const
{
    if (!m_model) {
        return QSortFilterProxyModel::data(index, role);
    }

    BluezQt::DevicePtr dev = m_model->device(mapToSource(index));
    if (!dev) {
        return QSortFilterProxyModel::data(index, role);
    }

    switch (role) {
    case DeviceRole:
        return QVariant::fromValue(m_manager->declarativeDeviceFromPtr(dev));
    case AdapterRole:
        return QVariant::fromValue(m_manager->declarativeAdapterFromPtr(dev->adapter()));
    case MediaPlayerRole:
        if (DeclarativeDevice *device = m_manager->declarativeDeviceFromPtr(dev)) {
            return QVariant::fromValue(device->mediaPlayer());
        }
        break;
//    case BatteryRole:
//        if (DeclarativeDevice *device = m_manager->declarativeDeviceFromPtr(dev)) {
//            return QVariant::fromValue(device->battery());
//        }
//        break;
    }

    return QSortFilterProxyModel::data(index, role);
}
