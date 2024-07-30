/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "fakedeviceinterface.h"

using namespace Solid::Backends::Fake;

FakeDeviceInterface::FakeDeviceInterface(FakeDevice *device)
    : QObject(device)
    , m_device(device)
{
}

FakeDeviceInterface::~FakeDeviceInterface()
{
}

#include "moc_fakedeviceinterface.cpp"
