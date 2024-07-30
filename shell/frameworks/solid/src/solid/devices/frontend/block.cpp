/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "block.h"
#include "block_p.h"

#include "soliddefs_p.h"
#include <solid/devices/ifaces/block.h>

Solid::Block::Block(QObject *backendObject)
    : DeviceInterface(*new BlockPrivate(), backendObject)
{
}

Solid::Block::~Block()
{
}

int Solid::Block::deviceMajor() const
{
    Q_D(const Block);
    return_SOLID_CALL(Ifaces::Block *, d->backendObject(), 0, deviceMajor());
}

int Solid::Block::deviceMinor() const
{
    Q_D(const Block);
    return_SOLID_CALL(Ifaces::Block *, d->backendObject(), 0, deviceMinor());
}

QString Solid::Block::device() const
{
    Q_D(const Block);
    return_SOLID_CALL(Ifaces::Block *, d->backendObject(), QString(), device());
}

#include "moc_block.cpp"
