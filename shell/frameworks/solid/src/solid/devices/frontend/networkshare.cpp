/*
    SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "networkshare.h"
#include "networkshare_p.h"

#include "soliddefs_p.h"
#include <solid/devices/ifaces/networkshare.h>

Solid::NetworkShare::NetworkShare(QObject *backendObject)
    : DeviceInterface(*new NetworkSharePrivate(), backendObject)
{
}

Solid::NetworkShare::~NetworkShare()
{
}

Solid::NetworkShare::ShareType Solid::NetworkShare::type() const
{
    Q_D(const NetworkShare);
    return_SOLID_CALL(Ifaces::NetworkShare *, d->backendObject(), Solid::NetworkShare::Unknown, type());
}

QUrl Solid::NetworkShare::url() const
{
    Q_D(const NetworkShare);
    return_SOLID_CALL(Ifaces::NetworkShare *, d->backendObject(), QUrl(), url());
}

#include "moc_networkshare.cpp"
