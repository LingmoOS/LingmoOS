/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iokitdeviceinterface.h"

using namespace Solid::Backends::IOKit;

DeviceInterface::DeviceInterface(IOKitDevice *device)
    : QObject(device)
    , m_device(device)
    , m_deviceCopy(nullptr)
{
}

DeviceInterface::DeviceInterface(const IOKitDevice *device)
    : QObject(device->parent())
    , m_deviceCopy(new IOKitDevice(*device))
{
    m_device = m_deviceCopy;
}

DeviceInterface::~DeviceInterface()
{
    if (m_deviceCopy) {
        delete m_deviceCopy;
        m_deviceCopy = nullptr;
    }
}

#include "moc_iokitdeviceinterface.cpp"
