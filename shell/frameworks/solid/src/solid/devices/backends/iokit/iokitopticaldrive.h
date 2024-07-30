/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_OPTICALDRIVE_H
#define SOLID_BACKENDS_IOKIT_OPTICALDRIVE_H

#include "iokitstorage.h"
#include <solid/devices/ifaces/opticaldrive.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitOpticalDrive : public IOKitStorage, virtual public Solid::Ifaces::OpticalDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDrive)

public:
    explicit IOKitOpticalDrive(IOKitDevice *device);
    virtual ~IOKitOpticalDrive();

public Q_SLOTS:
    Solid::OpticalDrive::MediumTypes supportedMedia() const override;
    int readSpeed() const override;
    int writeSpeed() const override;
    QList<int> writeSpeeds() const override;
    bool eject() override;

Q_SIGNALS:
    void ejectPressed(const QString &udi) override;
    void ejectDone(Solid::ErrorType error, QVariant errorData, const QString &udi) override;
    void ejectRequested(const QString &udi);

private:
    class Private;
    Private *d;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_OPTICALDRIVE_H
