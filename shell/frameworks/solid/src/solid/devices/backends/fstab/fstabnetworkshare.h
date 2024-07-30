/*
    SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FSTAB_NETWORKSHARE_H
#define SOLID_BACKENDS_FSTAB_NETWORKSHARE_H

#include <solid/devices/ifaces/networkshare.h>

#include <QObject>

namespace Solid
{
namespace Backends
{
namespace Fstab
{
class FstabDevice;
class FstabNetworkShare : public QObject, public Solid::Ifaces::NetworkShare
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::NetworkShare)

public:
    explicit FstabNetworkShare(Solid::Backends::Fstab::FstabDevice *device);

    ~FstabNetworkShare() override;

    Solid::NetworkShare::ShareType type() const override;

    QUrl url() const override;

public:
    const Solid::Backends::Fstab::FstabDevice *fstabDevice() const;

private:
    Solid::Backends::Fstab::FstabDevice *m_fstabDevice;
    Solid::NetworkShare::ShareType m_type;
    QUrl m_url;
};

}
}
}

#endif // SOLID_BACKENDS_FSTAB_NETWORKSHARE_H
