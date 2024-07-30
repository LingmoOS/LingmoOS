/*
    SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevdeviceinterface.h"

using namespace Solid::Backends::UDev;

DeviceInterface::DeviceInterface(UDevDevice *device)
    : QObject(device)
    , m_device(device)
{
}

DeviceInterface::~DeviceInterface()
{
}

#include "moc_udevdeviceinterface.cpp"
