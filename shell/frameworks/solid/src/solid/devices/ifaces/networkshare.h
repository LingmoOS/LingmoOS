/*
    SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_IFACES_NETWORKSHARE_H
#define SOLID_IFACES_NETWORKSHARE_H

#include <solid/devices/ifaces/deviceinterface.h>
#include <solid/networkshare.h>

namespace Solid
{
namespace Ifaces
{
/**
 * NetworkShare interface.
 *
 * a NetworkShare interface is used to determine the type of
 * network access.
 */
class NetworkShare : virtual public DeviceInterface
{
public:
    /**
     * Destroys a NetworkShare object.
     */
    ~NetworkShare() override;

    /**
     * Retrieves the type of network
     *
     * @return the type of network
     */
    virtual Solid::NetworkShare::ShareType type() const = 0;

    /**
     * Retrieves the url of network share
     *
     * @return the url of network share
     */
    virtual QUrl url() const = 0;
};
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::NetworkShare, "org.kde.Solid.Ifaces.NetworkShare/0.1")

#endif
