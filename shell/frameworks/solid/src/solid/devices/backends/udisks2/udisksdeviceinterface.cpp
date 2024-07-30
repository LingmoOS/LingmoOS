/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksdeviceinterface.h"

using namespace Solid::Backends::UDisks2;

DeviceInterface::DeviceInterface(Device *device)
    : QObject(device)
    , m_device(device)
{
}

DeviceInterface::~DeviceInterface()
{
}

#include "moc_udisksdeviceinterface.cpp"
