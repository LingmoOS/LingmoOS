/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "devicesmodel.h"
#include "adapter.h"
#include "device.h"
#include "manager.h"

namespace BluezQt
{
class DevicesModelPrivate : public QObject
{
public:
    explicit DevicesModelPrivate(DevicesModel *qq);

    void init();

    void deviceAdded(DevicePtr device);
    void deviceRemoved(DevicePtr device);
    void deviceChanged(DevicePtr device);
    void adapterChanged(AdapterPtr adapter);

    DevicesModel *q;
    Manager *m_manager;
    QList<DevicePtr> m_devices;
};

DevicesModelPrivate::DevicesModelPrivate(DevicesModel *qq)
    : QObject(qq)
    , q(qq)
    , m_manager(nullptr)
{
}

void DevicesModelPrivate::init()
{
    m_devices = m_manager->devices();

    connect(m_manager, &Manager::deviceAdded, this, &DevicesModelPrivate::deviceAdded);
    connect(m_manager, &Manager::deviceRemoved, this, &DevicesModelPrivate::deviceRemoved);
    connect(m_manager, &Manager::deviceChanged, this, &DevicesModelPrivate::deviceChanged);
    connect(m_manager, &Manager::adapterChanged, this, &DevicesModelPrivate::adapterChanged);
}

void DevicesModelPrivate::deviceAdded(DevicePtr device)
{
    q->beginInsertRows(QModelIndex(), m_devices.size(), m_devices.size());
    m_devices.append(device);
    q->endInsertRows();
}

void DevicesModelPrivate::deviceRemoved(DevicePtr device)
{
    int offset = m_devices.indexOf(device);
    Q_ASSERT(offset >= 0);

    q->beginRemoveRows(QModelIndex(), offset, offset);
    m_devices.removeAt(offset);
    q->endRemoveRows();
}

void DevicesModelPrivate::deviceChanged(DevicePtr device)
{
    int offset = m_devices.indexOf(device);
    Q_ASSERT(offset >= 0);

    QModelIndex idx = q->createIndex(offset, 0);
    Q_EMIT q->dataChanged(idx, idx);
}

void DevicesModelPrivate::adapterChanged(AdapterPtr adapter)
{
    const auto devices = adapter->devices();
    for (const DevicePtr &device : devices) {
        deviceChanged(device);
    }
}

DevicesModel::DevicesModel(Manager *manager, QObject *parent)
    : QAbstractListModel(parent)
    , d(new DevicesModelPrivate(this))
{
    d->m_manager = manager;
    d->init();
}

DevicesModel::~DevicesModel() = default;

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();

    roles[UbiRole] = QByteArrayLiteral("Ubi");
    roles[AddressRole] = QByteArrayLiteral("Address");
    roles[NameRole] = QByteArrayLiteral("Name");
    roles[FriendlyNameRole] = QByteArrayLiteral("FriendlyName");
    roles[RemoteNameRole] = QByteArrayLiteral("RemoteName");
    roles[ClassRole] = QByteArrayLiteral("Class");
    roles[TypeRole] = QByteArrayLiteral("Type");
    roles[AppearanceRole] = QByteArrayLiteral("Appearance");
    roles[IconRole] = QByteArrayLiteral("Icon");
    roles[PairedRole] = QByteArrayLiteral("Paired");
    roles[TrustedRole] = QByteArrayLiteral("Trusted");
    roles[BlockedRole] = QByteArrayLiteral("Blocked");
    roles[LegacyPairingRole] = QByteArrayLiteral("LegacyPairing");
    roles[RssiRole] = QByteArrayLiteral("Rssi");
    roles[ConnectedRole] = QByteArrayLiteral("Connected");
    roles[UuidsRole] = QByteArrayLiteral("Uuids");
    roles[ModaliasRole] = QByteArrayLiteral("Modalias");
    roles[AdapterNameRole] = QByteArrayLiteral("AdapterName");
    roles[AdapterAddressRole] = QByteArrayLiteral("AdapterAddress");
    roles[AdapterPoweredRole] = QByteArrayLiteral("AdapterPowered");
    roles[AdapterDiscoverableRole] = QByteArrayLiteral("AdapterDiscoverable");
    roles[AdapterPairableRole] = QByteArrayLiteral("AdapterPairable");
    roles[AdapterDiscoveringRole] = QByteArrayLiteral("AdapterDiscovering");
    roles[AdapterUuidsRole] = QByteArrayLiteral("AdapterUuids");

    return roles;
}

int DevicesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->m_devices.size();
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
    DevicePtr dev = device(index);
    if (!dev) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return dev->name();
    case UbiRole:
        return dev->ubi();
    case AddressRole:
        return dev->address();
    case NameRole:
        return dev->name();
    case FriendlyNameRole:
        return dev->friendlyName();
    case RemoteNameRole:
        return dev->remoteName();
    case ClassRole:
        return dev->deviceClass();
    case TypeRole:
        return dev->type();
    case AppearanceRole:
        return dev->appearance();
    case IconRole:
        return dev->icon();
    case PairedRole:
        return dev->isPaired();
    case TrustedRole:
        return dev->isTrusted();
    case BlockedRole:
        return dev->isBlocked();
    case LegacyPairingRole:
        return dev->hasLegacyPairing();
    case RssiRole:
        return dev->rssi();
    case ConnectedRole:
        return dev->isConnected();
    case UuidsRole:
        return dev->uuids();
    case ModaliasRole:
        return dev->modalias();
    case AdapterNameRole:
        return dev->adapter()->name();
    case AdapterAddressRole:
        return dev->adapter()->address();
    case AdapterPoweredRole:
        return dev->adapter()->isPowered();
    case AdapterDiscoverableRole:
        return dev->adapter()->isDiscoverable();
    case AdapterPairableRole:
        return dev->adapter()->isPairable();
    case AdapterDiscoveringRole:
        return dev->adapter()->isDiscovering();
    case AdapterUuidsRole:
        return dev->adapter()->uuids();
    default:
        return QVariant();
    }
}

QModelIndex DevicesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    return createIndex(row, 0);
}

DevicePtr DevicesModel::device(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return DevicePtr();
    }
    return d->m_devices.at(index.row());
}

} // namespace BluezQt

#include "moc_devicesmodel.cpp"
