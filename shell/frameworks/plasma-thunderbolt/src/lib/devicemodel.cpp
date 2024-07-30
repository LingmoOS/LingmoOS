/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "devicemodel.h"
#include "device.h"

using namespace Bolt;

Q_DECLARE_METATYPE(QSharedPointer<Bolt::Device>)

void DeviceModel::setManager(Manager *manager)
{
    if (mManager == manager) {
        return;
    }

    if (mManager) {
        mManager->disconnect(this);
    }

    beginResetModel();
    mManager = manager;
    mDevices.clear();
    if (mManager) {
        connect(mManager, &Manager::deviceAdded, this, [this](const QSharedPointer<Device> &device) {
            if (mShowHosts || device->type() == Type::Peripheral) {
                beginInsertRows({}, mDevices.count(), mDevices.count());
                mDevices.push_back(device);
                endInsertRows();
            }
        });
        connect(mManager, &Manager::deviceRemoved, this, [this](const QSharedPointer<Device> &device) {
            const int idx = mDevices.indexOf(device);
            if (idx == -1) {
                return;
            }
            beginRemoveRows({}, idx, idx);
            mDevices.removeAt(idx);
            endRemoveRows();
        });

        populateWithoutReset();
    }
    endResetModel();

    Q_EMIT managerChanged(mManager);
}

Manager *DeviceModel::manager() const
{
    return mManager;
}

bool DeviceModel::showHosts() const
{
    return mShowHosts;
}

void DeviceModel::setShowHosts(bool showHosts)
{
    if (mShowHosts != showHosts) {
        mShowHosts = showHosts;
        Q_EMIT showHostsChanged(mShowHosts);
        if (mManager) {
            beginResetModel();
            populateWithoutReset();
            endResetModel();
        }
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    auto roles = QAbstractListModel::roleNames();
    roles[DeviceRole] = "device";
    return roles;
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mDevices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= mDevices.size()) {
        return {};
    }

    if (role == DeviceRole) {
        return QVariant::fromValue(mDevices.at(index.row()).data());
    }

    return {};
}

void DeviceModel::populateWithoutReset()
{
    Q_ASSERT(mManager);

    mDevices.clear();
    const auto all = mManager->devices();
    std::copy_if(all.cbegin(), all.cend(), std::back_inserter(mDevices), [this](const auto &device) {
        return mShowHosts || device->type() == Type::Peripheral;
    });
}

#include "moc_devicemodel.cpp"
