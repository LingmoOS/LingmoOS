/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSOPTICALDISC_H
#define UDISKSOPTICALDISC_H

#include <solid/devices/ifaces/opticaldisc.h>

#include "../shared/udevqt.h"
#include <config-solid.h>

#include "udisksdevice.h"
#include "udisksstoragevolume.h"

namespace Solid
{
namespace Backends
{
namespace UDisks2
{
class OpticalDisc : public StorageVolume, virtual public Solid::Ifaces::OpticalDisc
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDisc)

public:
    OpticalDisc(Device *dev);
    ~OpticalDisc() override;

    qulonglong capacity() const override;
    bool isRewritable() const override;
    bool isBlank() const override;
    bool isAppendable() const override;
    Solid::OpticalDisc::DiscType discType() const override;
    Solid::OpticalDisc::ContentTypes availableContent() const override;

    class Identity
    {
    public:
        Identity();
        Identity(const Device &device, const Device &drive);
        bool operator==(const Identity &) const;

    private:
        long long m_detectTime;
        long long m_size;
        uint m_labelHash;
    };

private:
    mutable Identity m_identity;
    QString media() const;
    mutable Solid::OpticalDisc::ContentTypes m_cachedContent;
    Device *m_drive;
#if UDEV_FOUND
    UdevQt::Device m_udevDevice;
#endif
};

}
}
}
#endif // UDISKSOPTICALDISC_H
