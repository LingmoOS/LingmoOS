/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_PLASMA_VAULTSMODEL_P_H
#define PLASMAVAULT_PLASMA_VAULTSMODEL_P_H

#include <QDBusInterface>
#include <QDBusServiceWatcher>

#include "plasmavault_interface.h"
#include "vaultsmodel.h"
#include <common/vaultinfo.h>

class VaultsModel::Private : public QObject
{
    Q_OBJECT

public:
    explicit Private(VaultsModel *parent);
    QStringList vaultKeys;
    QMap<QString, PlasmaVault::VaultInfo> vaults;

public Q_SLOTS:
    void onVaultAdded(const PlasmaVault::VaultInfo &vaultData);
    void onVaultRemoved(const QString &device);
    void onVaultChanged(const PlasmaVault::VaultInfo &vaultData);

    void loadData();
    void clearData();

public:
    org::kde::plasmavault service;
    QDBusServiceWatcher serviceWatcher;

    QSet<QString> busyVaults;
    QSet<QString> errorVaults;
    QSet<QString> openVaults;

    VaultsModel *const q;
};

#endif // include guard
