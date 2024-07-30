/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_PORTABLEMEDIAPLAYER_P_H
#define SOLID_PORTABLEMEDIAPLAYER_P_H

#include "deviceinterface_p.h"

namespace Solid
{
class PortableMediaPlayerPrivate : public DeviceInterfacePrivate
{
public:
    PortableMediaPlayerPrivate()
        : DeviceInterfacePrivate()
    {
    }
};
}

#endif
