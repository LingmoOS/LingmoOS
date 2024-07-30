/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_SERVICE_H
#define PLASMAVAULT_KDED_SERVICE_H

#include <KDEDModule>

#include <QVariant>

#include <common/vaultinfo.h>
#include <engine/vault.h>

class Q_DECL_EXPORT PlasmaVaultService : public KDEDModule
{
    Q_CLASSINFO("D-Bus Interface", "org.kde.plasmavault")
    Q_OBJECT

public:
    PlasmaVaultService(QObject *parent, const QVariantList &);
    ~PlasmaVaultService() override;

public Q_SLOTS:
    Q_SCRIPTABLE void requestNewVault();
    Q_SCRIPTABLE void requestImportVault();
    Q_SCRIPTABLE void openVault(const QString &device);
    Q_SCRIPTABLE void closeVault(const QString &device);
    Q_SCRIPTABLE void forceCloseVault(const QString &device);

    Q_SCRIPTABLE void configureVault(const QString &device);
    Q_SCRIPTABLE void openVaultInFileManager(const QString &device);

    Q_SCRIPTABLE PlasmaVault::VaultInfoList availableDevices() const;

    Q_SCRIPTABLE bool hasOpenVaults() const;
    Q_SCRIPTABLE void closeAllVaults();
    Q_SCRIPTABLE void forceCloseAllVaults();

    Q_SCRIPTABLE void deleteVault(const QString &device, const QString &name);

    Q_SCRIPTABLE void updateStatus();

Q_SIGNALS:
    void registered();

    Q_SCRIPTABLE void vaultAdded(const PlasmaVault::VaultInfo &vaultData);
    Q_SCRIPTABLE void vaultRemoved(const QString &device);
    Q_SCRIPTABLE void vaultChanged(const PlasmaVault::VaultInfo &vaultData);

    Q_SCRIPTABLE void hasOpenVaultsChanged(bool hasOpenVaults);

private Q_SLOTS:
    void slotRegistered(const QDBusObjectPath &path);

    void registerVault(PlasmaVault::Vault *vault);
    void forgetVault(PlasmaVault::Vault *vault);

    void onVaultStatusChanged(PlasmaVault::VaultInfo::Status status);
    void onVaultMessageChanged(const QString &message);
    void onVaultInfoChanged();

    void onActivitiesChanged(const QStringList &knownActivities);
    void onCurrentActivityChanged(const QString &currentActivity);
    void onActivityRemoved(const QString &removedActivity);

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // include guard
