/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "opticaldisc.h"
#include "opticaldisc_p.h"

#include "soliddefs_p.h"
#include <solid/devices/ifaces/opticaldisc.h>

Solid::OpticalDisc::OpticalDisc(QObject *backendObject)
    : StorageVolume(*new OpticalDiscPrivate(), backendObject)
{
}

Solid::OpticalDisc::~OpticalDisc()
{
}

Solid::OpticalDisc::ContentTypes Solid::OpticalDisc::availableContent() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), ContentTypes(), availableContent());
}

Solid::OpticalDisc::DiscType Solid::OpticalDisc::discType() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), UnknownDiscType, discType());
}

bool Solid::OpticalDisc::isAppendable() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), false, isAppendable());
}

bool Solid::OpticalDisc::isBlank() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), false, isBlank());
}

bool Solid::OpticalDisc::isRewritable() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), false, isRewritable());
}

qulonglong Solid::OpticalDisc::capacity() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), 0, capacity());
}

#include "moc_opticaldisc.cpp"
