/*
    SPDX-FileCopyrightText: 2010 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevblock.h"

using namespace Solid::Backends::UDev;

Block::Block(UDevDevice *device)
    : DeviceInterface(device)
{
}

Block::~Block()
{
}

int Block::deviceMajor() const
{
    return m_device->property("MAJOR").toInt();
}

int Block::deviceMinor() const
{
    return m_device->property("MINOR").toInt();
}

QString Block::device() const
{
    return m_device->property("DEVNAME").toString();
}

#include "moc_udevblock.cpp"
