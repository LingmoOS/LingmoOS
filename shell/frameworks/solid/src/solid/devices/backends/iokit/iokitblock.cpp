/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iokitblock.h"

#include "iokitdevice.h"

using namespace Solid::Backends::IOKit;

Block::Block(IOKitDevice *device)
    : DeviceInterface(device)
{
}

Block::Block(const IOKitDevice *device)
    : DeviceInterface(device)
{
}

Block::~Block()
{
}

int Block::deviceMajor() const
{
    return m_device->property(QLatin1String("BSD Major")).toInt();
}

int Block::deviceMinor() const
{
    return m_device->property(QLatin1String("BSD Minor")).toInt();
}

QString Block::device() const
{
    if (m_device->iOKitPropertyExists(QStringLiteral("BSD Name"))) {
        return QStringLiteral("/dev/") + m_device->property(QLatin1String("BSD Name")).toString();
    }
    return QString();
}

#include "moc_iokitblock.cpp"
