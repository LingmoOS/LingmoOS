/*
    SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_NETWORKSHARE_P_H
#define SOLID_NETWORKSHARE_P_H

#include "deviceinterface_p.h"

namespace Solid
{
class NetworkSharePrivate : public DeviceInterfacePrivate
{
public:
    NetworkSharePrivate()
        : DeviceInterfacePrivate()
    {
    }
};
}

#endif
