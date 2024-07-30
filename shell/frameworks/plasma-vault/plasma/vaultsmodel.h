/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_PLASMA_VAULTSMODEL_H
#define PLASMAVAULT_PLASMA_VAULTSMODEL_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include <PlasmaActivities/Consumer>

#include <common/vaultinfo.h>

class VaultsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)
    Q_PROPERTY(bool hasError READ hasError NOTIFY hasErrorChanged)
    Q_PROPERTY(bool hasOpenVaults READ hasOpenVaults NOTIFY hasOpenVaultsChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)

public:
    explicit VaultsModel(QObject *parent = nullptr);
    ~VaultsModel() override;

    // This forces detection of removable drives
    void reloadDevices();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    enum Roles {
        VaultName = Qt::UserRole + 1, ///< The user defined name of the vault
        VaultDevice, ///< Path to the encrypted data
        VaultMountPoint, ///< Where the data should be visible from
        VaultIcon, ///< Icon for the fault
        VaultIsBusy, ///< Is the vault currently being opened or closed
        VaultIsOpened, ///< Is the vault open
        VaultStatus, ///< Status of the vault, @see VaultInfo::Status
        VaultActivities, ///< To which activities does this vault belong to
        VaultIsOfflineOnly, ///< Should the networking be off when this vault is mounted
        VaultMessage, ///< Message to be shown for the vault
        VaultIsEnabled, ///< Can the vault be mounted
    };

public Q_SLOTS:
    void refresh();

    // Open (unlock) a vault
    void open(const QString &device);

    // Close the vault
    void close(const QString &device);

    // Open the vault if it is closed, or close it otherwise
    void toggle(const QString &device);

    // Opens the new-vault wizard
    void requestNewVault();

    // Kills all applications using the vault and closes it
    void forceClose(const QString &device);

    // Opens the configuration dialogue for the vault
    void configure(const QString &device);

    // Open in file manager
    void openInFileManager(const QString &device);

    bool isBusy() const;
    bool hasError() const;
    bool hasOpenVaults() const;

Q_SIGNALS:
    void isBusyChanged(bool isBusy);
    void hasErrorChanged(bool hasError);
    void hasOpenVaultsChanged(bool hasOpenVaults);
    void rowCountChanged(int count);

private:
    class Private;
    friend class Private;
    QScopedPointer<Private> d;
};

class SortedVaultsModelProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit SortedVaultsModelProxy(QObject *parent);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

public Q_SLOTS:
    QObject *actionsModel() const;

    // This forces detection of removable drives
    void reloadDevices();

private:
    VaultsModel *m_source;
    KActivities::Consumer *m_kamd;
};

#endif // include guard
