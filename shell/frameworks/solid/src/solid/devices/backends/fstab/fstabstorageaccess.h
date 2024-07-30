/*
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FSTAB_STORAGEACCESS_H
#define SOLID_BACKENDS_FSTAB_STORAGEACCESS_H

#include <solid/devices/ifaces/storageaccess.h>

#include <QObject>

namespace Solid
{
namespace Backends
{
namespace Fstab
{
class FstabDevice;
class FstabStorageAccess : public QObject, public Solid::Ifaces::StorageAccess
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageAccess)

public:
    explicit FstabStorageAccess(Solid::Backends::Fstab::FstabDevice *device);

    ~FstabStorageAccess() override;

    bool isAccessible() const override;

    QString filePath() const override;

    bool isIgnored() const override;

    bool isEncrypted() const override;

    bool setup() override;

    bool teardown() override;

public:
    const Solid::Backends::Fstab::FstabDevice *fstabDevice() const;

Q_SIGNALS:
    void accessibilityChanged(bool accessible, const QString &udi) override;

    void setupDone(Solid::ErrorType error, QVariant data, const QString &udi) override;

    void teardownDone(Solid::ErrorType error, QVariant data, const QString &udi) override;

    void setupRequested(const QString &udi) override;

    void teardownRequested(const QString &udi) override;

private Q_SLOTS:
    void onMtabChanged(const QString &device);
    void connectDBusSignals();

    void slotSetupRequested();
    void slotSetupDone(int error, const QString &errorString);
    void slotTeardownRequested();
    void slotTeardownDone(int error, const QString &errorString);

private:
    Solid::Backends::Fstab::FstabDevice *m_fstabDevice;
    QString m_filePath;
    bool m_isAccessible;
    bool m_isIgnored;
};

}
}
}

#endif // SOLID_BACKENDS_FSTAB_DEVICE_INTERFACE_H
