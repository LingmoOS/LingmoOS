/*
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "smbmountmodel.h"

#include <QDebug>
#include <QIcon>
#include <QMetaEnum>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/NetworkShare>
#include <Solid/StorageAccess>

SmbMountModel::SmbMountModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &SmbMountModel::addDevice);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, &SmbMountModel::removeDevice);
    metaObject()->invokeMethod(this, &SmbMountModel::reloadData);
}

SmbMountModel::~SmbMountModel() = default;

int SmbMountModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_devices.size();
}

QVariant SmbMountModel::data(const QModelIndex &index, int intRole) const
{
    if (!index.isValid()) {
        return {};
    }

    Q_ASSERT(index.row() < m_devices.count());

    static QMetaEnum roleEnum = QMetaEnum::fromType<Role>();
    if (roleEnum.valueToKey(intRole) == nullptr) {
        return {};
    }
    const auto role = static_cast<Role>(intRole);

    const Solid::Device &device = m_devices.at(index.row());
    switch (role) {
    case Role::Share:
        return device.as<Solid::NetworkShare>()->url();
    case Role::Path:
        return device.as<Solid::StorageAccess>()->filePath();
    case Role::Accessible:
        return device.as<Solid::StorageAccess>()->isAccessible();
    }

    return {};
}

void SmbMountModel::addDevice(const QString &udi)
{
    auto it = deviceForUdi(udi);
    if (it != m_devices.constEnd()) {
        return; // already in the list
    }

    Solid::Device device(udi);
    if (!device.is<Solid::NetworkShare>()) {
        return;
    }

    beginInsertRows(QModelIndex(), m_devices.count(), m_devices.count());
    m_devices.append(device);
    endInsertRows();
}

void SmbMountModel::removeDevice(const QString &udi)
{
    auto it = deviceForUdi(udi);
    if (it == m_devices.constEnd()) {
        return; // untracked udi
    }

    const int index = static_cast<int>(std::distance(m_devices.constBegin(), it));
    beginRemoveRows(QModelIndex(), index, index);
    m_devices.removeAt(index);
    endRemoveRows();
}

void SmbMountModel::reloadData()
{
    beginResetModel();
    m_devices.clear();
    const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::NetworkShare);
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if (!it->is<Solid::NetworkShare>()) {
            // Workaround in case listFromType still gives incorrect types.
            // https://bugs.kde.org/show_bug.cgi?id=419220
            continue;
        }
        switch (it->as<Solid::NetworkShare>()->type()) {
        case Solid::NetworkShare::Cifs:
        case Solid::NetworkShare::Smb3:
            m_devices.append(*it);
            continue;
        case Solid::NetworkShare::Nfs:
        case Solid::NetworkShare::Unknown:
            continue;
        }
    }
    endResetModel();
}

bool SmbMountModel::hasChildren(const QModelIndex &parent) const
{
    return !parent.isValid() ? false : (rowCount(parent) > 0);
}

QHash<int, QByteArray> SmbMountModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (!roles.isEmpty()) {
        return roles;
    }

    const QMetaEnum roleEnum = QMetaEnum::fromType<Role>();
    for (int i = 0; i < roleEnum.keyCount(); ++i) {
        const int value = roleEnum.value(i);
        Q_ASSERT(value != -1);
        roles[static_cast<int>(value)] = QByteArray("ROLE_") + roleEnum.valueToKey(value);
    }
    return roles;
}

#include "moc_smbmountmodel.cpp"
