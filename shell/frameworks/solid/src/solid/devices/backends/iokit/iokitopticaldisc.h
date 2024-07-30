/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_OPTICALDISC_H
#define SOLID_BACKENDS_IOKIT_OPTICALDISC_H

#include "iokitvolume.h"
#include <solid/devices/ifaces/opticaldisc.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitOpticalDisc : public IOKitVolume, virtual public Solid::Ifaces::OpticalDisc
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDisc)

public:
    IOKitOpticalDisc(IOKitDevice *device);
    IOKitOpticalDisc(const IOKitDevice *device);
    virtual ~IOKitOpticalDisc();

    // overridden from IOKit::Block because optical discs must
    // be accessed through the raw device.
    QString device() const override;

    Solid::OpticalDisc::ContentTypes availableContent() const override;
    Solid::OpticalDisc::DiscType discType() const override;
    bool isAppendable() const override;
    bool isBlank() const override;
    bool isRewritable() const override;
    qulonglong capacity() const override;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_OPTICALDISC_H
