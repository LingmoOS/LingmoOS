/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vaultsmodel.h"
#include "vaultsmodel_p.h"
#include <QFileInfo>

using namespace PlasmaVault;

VaultsModel::Private::Private(VaultsModel *parent)
    : service("org.kde.kded6", "/modules/plasmavault", QDBusConnection::sessionBus())
    , serviceWatcher("org.kde.kded6", QDBusConnection::sessionBus())
    , q(parent)
{
    connect(&service, &org::kde::plasmavault::vaultAdded, this, &Private::onVaultAdded);
    connect(&service, &org::kde::plasmavault::vaultChanged, this, &Private::onVaultChanged);
    connect(&service, &org::kde::plasmavault::vaultRemoved, this, &Private::onVaultRemoved);

    connect(&serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, [this](const QString &service, const QString &oldOwner, const QString &newOwner) {
        Q_UNUSED(oldOwner);

        if (service != "org.kde.kded6") {
            return;
        }

        // If kded is not running, just clear all vault info,
        // otherwise load the data
        if (newOwner.isEmpty()) {
            clearData();
        } else {
            loadData();
        }
    });

    // Try to load the data. This should start kded if it is not running
    // for some reason
    loadData();
}

void VaultsModel::Private::loadData()
{
    // Before loading the new data, lets forget everything
    clearData();

    // Asynchronously load the devices
    QDBusPendingReply<VaultInfoList> reply = service.availableDevices();
    auto watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, &QDBusPendingCallWatcher::finished, q, [this, reply, watcher] {
        watcher->deleteLater();
        if (reply.isError()) {
            return;
        }
        const VaultInfoList &vaultList = reply.value();
        const int oldSize = vaultKeys.size();
        q->beginResetModel();

        vaults.clear();
        vaultKeys.clear();
        busyVaults.clear();
        errorVaults.clear();
        openVaults.clear();

        for (const auto &vault : vaultList) {
            vaults[vault.device] = vault;
            vaultKeys << vault.device;

            if (vault.isBusy()) {
                busyVaults << vault.device;
            }

            if (vault.isOpened()) {
                openVaults << vault.device;
            }

            if (!vault.message.isEmpty()) {
                errorVaults << vault.device;
            }
        }

        q->endResetModel();

        if (vaultKeys.size() != oldSize) {
            Q_EMIT q->rowCountChanged(vaultKeys.size());
        }

        Q_EMIT q->isBusyChanged(busyVaults.count() != 0);
        Q_EMIT q->hasOpenVaultsChanged(openVaults.count() != 0);
        Q_EMIT q->hasErrorChanged(errorVaults.count() != 0);
    });
}

void VaultsModel::Private::clearData()
{
    q->beginResetModel();
    vaultKeys.clear();
    vaults.clear();
    q->endResetModel();
}

void VaultsModel::Private::onVaultAdded(const PlasmaVault::VaultInfo &vaultInfo)
{
    const auto device = vaultInfo.device;

    if (vaults.contains(device))
        return;

    q->beginInsertRows(QModelIndex(), vaultKeys.size(), vaultKeys.size());
    vaults[device] = vaultInfo;
    vaultKeys << device;
    q->endInsertRows();
    Q_EMIT q->rowCountChanged(vaultKeys.size());
}

void VaultsModel::Private::onVaultRemoved(const QString &device)
{
    if (!vaults.contains(device))
        return;

    const auto row = vaultKeys.indexOf(device);

    q->beginRemoveRows(QModelIndex(), row, row);
    vaultKeys.removeAt(row);
    vaults.remove(device);
    q->endRemoveRows();
    Q_EMIT q->rowCountChanged(vaultKeys.size());
}

void VaultsModel::Private::onVaultChanged(const PlasmaVault::VaultInfo &vaultInfo)
{
    const auto device = vaultInfo.device;

    if (!vaultKeys.contains(device))
        return;

    const auto row = vaultKeys.indexOf(device);

    // Lets see whether this warrants updates to the busy flag
    if (vaultInfo.isBusy() && !busyVaults.contains(device)) {
        busyVaults << device;
        if (busyVaults.count() == 1) {
            Q_EMIT q->isBusyChanged(true);
        }
    }

    if (!vaultInfo.isBusy() && busyVaults.contains(device)) {
        busyVaults.remove(device);
        if (busyVaults.count() == 0) {
            Q_EMIT q->isBusyChanged(false);
        }
    }

    // Lets see whether this warrants updates to the error flag
    if (!vaultInfo.message.isEmpty() && !errorVaults.contains(device)) {
        errorVaults << device;
        if (errorVaults.count() == 1) {
            Q_EMIT q->hasErrorChanged(true);
        }
    }

    if (vaultInfo.message.isEmpty() && errorVaults.contains(device)) {
        errorVaults.remove(device);
        if (errorVaults.count() == 0) {
            Q_EMIT q->hasErrorChanged(false);
        }
    }

    // Let's see whether this warrants updates to the opened flag
    if (vaultInfo.isOpened() && !openVaults.contains(device)) {
        openVaults << device;
        Q_EMIT q->hasOpenVaultsChanged(true);
    }

    if (!vaultInfo.isOpened() && openVaults.contains(device)) {
        openVaults.remove(device);
        if (openVaults.count() == 0) {
            Q_EMIT q->hasOpenVaultsChanged(false);
        }
    }

    vaults[device] = vaultInfo;
    q->dataChanged(q->index(row), q->index(row));
}

VaultsModel::VaultsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
}

VaultsModel::~VaultsModel()
{
}

int VaultsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->vaultKeys.size();
}

QVariant VaultsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const int row = index.row();

    if (row >= d->vaultKeys.count()) {
        return {};
    }

    const auto device = d->vaultKeys[row];
    const auto &vault = d->vaults[device];

    switch (role) {
    case VaultDevice:
        return vault.device;

    case VaultMountPoint:
        return vault.mountPoint;

    case VaultName:
        return vault.name.isEmpty() ? vault.device : vault.name;

    case VaultIcon: {
        switch (vault.status) {
        case VaultInfo::Error:
            return "document-close";

        case VaultInfo::DeviceMissing:
            return "document-close";

        case VaultInfo::NotInitialized:
            return "folder-gray";

        case VaultInfo::Closed:
            return "folder-encrypted";

        case VaultInfo::Opened:
            return "folder-decrypted";

        default:
            return "";
        }
    }

    case VaultIsBusy:
        return vault.isBusy();

    case VaultIsOpened:
        return vault.isOpened();

    case VaultActivities:
        return vault.activities;

    case VaultIsOfflineOnly:
        return vault.isOfflineOnly;

    case VaultMessage:
        return vault.message;

    case VaultIsEnabled:
        return !(vault.status == VaultInfo::Error) //
            && !(vault.status == VaultInfo::DeviceMissing);
    }

    return {};
}

QHash<int, QByteArray> VaultsModel::roleNames() const
{
    return {
        {VaultName, "name"},
        {VaultIcon, "icon"},
        {VaultDevice, "device"},
        {VaultMountPoint, "mountPoint"},
        {VaultIsBusy, "isBusy"},
        {VaultIsOpened, "isOpened"},
        {VaultActivities, "activities"},
        {VaultIsOfflineOnly, "isOfflineOnly"},
        {VaultStatus, "status"},
        {VaultMessage, "message"},
        {VaultIsEnabled, "isEnabled"},
    };
}

void VaultsModel::refresh()
{
    d->loadData();
}

void VaultsModel::reloadDevices()
{
    d->service.updateStatus();
}

void VaultsModel::open(const QString &device)
{
    if (!d->vaults.contains(device))
        return;

    d->service.openVault(device);
}

void VaultsModel::close(const QString &device)
{
    if (!d->vaults.contains(device))
        return;

    d->service.closeVault(device);
}

void VaultsModel::toggle(const QString &device)
{
    if (!d->vaults.contains(device))
        return;

    const auto &vault = d->vaults[device];
    if (vault.status == VaultInfo::Opened) {
        close(device);
    } else if (vault.status == VaultInfo::Closed) {
        open(device);
    }
}

void VaultsModel::forceClose(const QString &device)
{
    if (!d->vaults.contains(device))
        return;

    d->service.forceCloseVault(device);
}

void VaultsModel::configure(const QString &device)
{
    if (!d->vaults.contains(device))
        return;

    d->service.configureVault(device);
}

void VaultsModel::openInFileManager(const QString &device)
{
    if (!d->vaults.contains(device))
        return;

    d->service.openVaultInFileManager(device);
}

void VaultsModel::requestNewVault()
{
    d->service.requestNewVault();
}

bool VaultsModel::isBusy() const
{
    return !d->busyVaults.isEmpty();
}

bool VaultsModel::hasError() const
{
    return !d->errorVaults.isEmpty();
}

bool VaultsModel::hasOpenVaults() const
{
    return !d->openVaults.isEmpty();
}

SortedVaultsModelProxy::SortedVaultsModelProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_source(new VaultsModel(this))
    , m_kamd(new KActivities::Consumer(this))
{
    setSourceModel(m_source);

    connect(m_kamd, &KActivities::Consumer::currentActivityChanged, this, &SortedVaultsModelProxy::invalidate);
    connect(m_kamd, &KActivities::Consumer::serviceStatusChanged, this, &SortedVaultsModelProxy::invalidate);
}

bool SortedVaultsModelProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftData = sourceModel()->data(left, VaultsModel::VaultName);
    const auto rightData = sourceModel()->data(right, VaultsModel::VaultName);
    return QPartialOrdering::Less == QVariant::compare(leftData, rightData);
}

bool SortedVaultsModelProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);

    const auto activities = m_source->index(sourceRow).data(VaultsModel::VaultActivities).toStringList();

    const auto isOpened = m_source->index(sourceRow).data(VaultsModel::VaultIsOpened).toBool();

    return activities.size() == 0 || isOpened || activities.contains(m_kamd->currentActivity());
}

QObject *SortedVaultsModelProxy::actionsModel() const
{
    return sourceModel();
}

void SortedVaultsModelProxy::reloadDevices()
{
    static_cast<VaultsModel *>(sourceModel())->reloadDevices();
}

#include "moc_vaultsmodel_p.cpp"

#include "moc_vaultsmodel.cpp"
