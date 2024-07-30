/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_ENGINE_VAULT_H
#define PLASMAVAULT_KDED_ENGINE_VAULT_H

#include <QDBusArgument>
#include <QObject>
#include <QString>
#include <QVariant>

#include <common/vaultinfo.h>

#include "types.h"

#include "commandresult.h"

namespace PlasmaVault
{
// Main keys
#define KEY_NAME "vault-name"
#define KEY_BACKEND "vault-backend"
#define KEY_PASSWORD "vault-password"
#define KEY_DEVICE "vault-device"
#define KEY_MOUNT_POINT "vault-mount-point"

// Additional options
#define KEY_ACTIVITIES "vault-activities"
#define KEY_OFFLINEONLY "vault-offline-only"

class Vault : public QObject
{
    Q_OBJECT

    Q_PROPERTY(PlasmaVault::Device device READ device)
    Q_PROPERTY(PlasmaVault::MountPoint mountPoint READ mountPoint NOTIFY mountPointChanged)

    Q_PROPERTY(VaultInfo::Status status READ status NOTIFY statusChanged)

    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY isInitializedChanged)
    Q_PROPERTY(bool isOpened READ isOpened NOTIFY isOpenedChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)

    Q_PROPERTY(QStringList activities READ activities NOTIFY activitiesChanged)
    Q_PROPERTY(bool isOfflineOnly READ isOfflineOnly NOTIFY isOfflineOnlyChanged)

public:
    Vault(const Device &device, QObject *parent = nullptr);
    ~Vault() override;

    typedef QHash<QByteArray, QVariant> Payload;

    bool isValid() const;

    FutureResult<> create(const QString &name, const MountPoint &mountPoint, const Payload &payload);
    FutureResult<> import(const QString &name, const MountPoint &mountPoint, const Payload &payload);

    FutureResult<> open(const Payload &payload);
    FutureResult<> close();

    // FutureResult<> configure();
    FutureResult<> forceClose();

    FutureResult<> dismantle(const Payload &payload);

    VaultInfo info() const;

    // TODO: Replace this with shared_ptr and enable_shared_from_this?
    void scheduleDeletion();

    void updateStatus();

public Q_SLOTS:
    QString message() const;
    VaultInfo::Status status() const;

    bool isInitialized() const;
    bool isOpened() const;
    bool isBusy() const;

    Device device() const;
    QString backend() const;

    MountPoint mountPoint() const;
    void setMountPoint(const MountPoint &mountPoint);

    QString name() const;
    void setName(const QString &name);

    QStringList activities() const;
    void setActivities(const QStringList &activities);

    bool isOfflineOnly() const;
    void setIsOfflineOnly(bool isOfflineOnly);

    void saveConfiguration();

Q_SIGNALS:
    void mountPointChanged(const QString &mountPoint);
    void statusChanged(VaultInfo::Status status);
    void isInitializedChanged(bool isInitialized);
    void isOpenedChanged(bool isOpened);
    void isBusyChanged(bool isBusy);
    void activitiesChanged(const QStringList &activities);
    void messageChanged(const QString &message);
    void nameChanged(const QString &name);
    void isOfflineOnlyChanged(bool isOfflineOnly);

    void infoChanged();

public:
    static QList<Device> availableDevices();
    static QStringList statusMessage();

private:
    class Private;
    QScopedPointer<Private> d;
};

} // namespace PlasmaVault

#endif // include guard
