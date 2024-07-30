/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_STORAGEACCESS_H
#define SOLID_BACKENDS_IOKIT_STORAGEACCESS_H

#include "dadictionary_p.h"
#include "iokitdeviceinterface.h"
#include <solid/devices/ifaces/storageaccess.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitStorageAccess : public DeviceInterface, virtual public Solid::Ifaces::StorageAccess
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageAccess)

public:
    IOKitStorageAccess(IOKitDevice *device);
    IOKitStorageAccess(const IOKitDevice *device);
    virtual ~IOKitStorageAccess();

    bool isAccessible() const override;
    QString filePath() const override;
    bool isIgnored() const override;
    bool isEncrypted() const override;
public Q_SLOTS:
    bool setup() override;
    bool teardown() override;

Q_SIGNALS:
    void accessibilityChanged(bool accessible, const QString &udi) override;
    void setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi) override;
    void teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi) override;
    void setupRequested(const QString &udi) override;
    void teardownRequested(const QString &udi) override;

private Q_SLOTS:
    void onPropertyChanged(const QMap<QString, int> &changes);

private:
    DADictionary *daDict;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_STORAGEACCESS_H
