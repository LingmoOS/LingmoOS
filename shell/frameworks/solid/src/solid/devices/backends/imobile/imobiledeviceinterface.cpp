/*
    SPDX-FileCopyrightText: 2020 MBition GmbH
    SPDX-FileContributor: Kai Uwe Broulik <kai_uwe.broulik@mbition.io>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "imobiledeviceinterface.h"

#include "imobiledevice.h"

using namespace Solid::Backends::IMobile;

DeviceInterface::DeviceInterface(IMobileDevice *device)
    : QObject(device)
    , m_device(device)
{
}

DeviceInterface::~DeviceInterface() = default;

#include "moc_imobiledeviceinterface.cpp"
