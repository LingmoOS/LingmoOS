/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WINOPTICALDISC_H
#define WINOPTICALDISC_H

#include <solid/devices/ifaces/opticaldisc.h>

#include "winstoragevolume.h"

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinOpticalDisc : public WinStorageVolume, virtual public Solid::Ifaces::OpticalDisc
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDisc)

public:
    WinOpticalDisc(WinDevice *device);
    virtual ~WinOpticalDisc();

    virtual Solid::OpticalDisc::ContentTypes availableContent() const;

    virtual Solid::OpticalDisc::DiscType discType() const;

    virtual bool isAppendable() const;

    virtual bool isBlank() const;

    virtual bool isRewritable() const;

    virtual qulonglong capacity() const;

private:
    Solid::OpticalDisc::DiscType m_discType;
    bool m_isRewritable;
    bool m_isBlank;
    bool m_isAppendable;
};
}
}
}

#endif // WINOPTICALDISC_H
