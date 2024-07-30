/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "managerbase_p.h"

#include <stdlib.h>

#include <config-backends.h>

// do *not* use other defines than BUILD_DEVICE_BACKEND_$backend to include
// the managers, and keep an alphabetical order
#ifdef BUILD_DEVICE_BACKEND_fakehw
#include "backends/fakehw/fakemanager.h"
#endif
#ifdef BUILD_DEVICE_BACKEND_fstab
#include "backends/fstab/fstabmanager.h"
#endif
#ifdef BUILD_DEVICE_BACKEND_imobile
#include "backends/imobile/imobilemanager.h"
#endif
#ifdef BUILD_DEVICE_BACKEND_iokit
#include "backends/iokit/iokitmanager.h"
#endif
#ifdef BUILD_DEVICE_BACKEND_udev
#include "backends/udev/udevmanager.h"
#endif
#ifdef BUILD_DEVICE_BACKEND_udisks2
#include "backends/udisks2/udisksmanager.h"
#endif
#ifdef BUILD_DEVICE_BACKEND_upower
#include "backends/upower/upowermanager.h"
#endif
#ifdef BUILD_DEVICE_BACKEND_win
#include "backends/win/windevicemanager.h"
#endif

Solid::ManagerBasePrivate::ManagerBasePrivate()
{
}

Solid::ManagerBasePrivate::~ManagerBasePrivate()
{
    qDeleteAll(m_backends);
}

// do *not* use other defines than BUILD_DEVICE_BACKEND_$backend to add
// the managers, and keep an alphabetical order
void Solid::ManagerBasePrivate::loadBackends()
{
    QString solidFakeXml(QString::fromLocal8Bit(qgetenv("SOLID_FAKEHW")));

    if (!solidFakeXml.isEmpty()) {
#ifdef BUILD_DEVICE_BACKEND_fakehw
        m_backends << new Solid::Backends::Fake::FakeManager(nullptr, solidFakeXml);
#endif
    } else {
#ifdef BUILD_DEVICE_BACKEND_fstab
        m_backends << new Solid::Backends::Fstab::FstabManager(nullptr);
#endif
#ifdef BUILD_DEVICE_BACKEND_imobile
        m_backends << new Solid::Backends::IMobile::Manager(nullptr);
#endif
#ifdef BUILD_DEVICE_BACKEND_iokit
        m_backends << new Solid::Backends::IOKit::IOKitManager(nullptr);
#endif
#ifdef BUILD_DEVICE_BACKEND_udev
        m_backends << new Solid::Backends::UDev::UDevManager(nullptr);
#endif
#ifdef BUILD_DEVICE_BACKEND_udisks2
        if (!qEnvironmentVariableIsSet("SOLID_DISABLE_UDISKS2")) {
            m_backends << new Solid::Backends::UDisks2::Manager(nullptr);
        }
#endif
#ifdef BUILD_DEVICE_BACKEND_upower
        if (!qEnvironmentVariableIsSet("SOLID_DISABLE_UPOWER")) {
            m_backends << new Solid::Backends::UPower::UPowerManager(nullptr);
        }
#endif
#ifdef BUILD_DEVICE_BACKEND_win
        m_backends << new Solid::Backends::Win::WinDeviceManager(nullptr);
#endif
    }
}

QList<Solid::Ifaces::DeviceManager *> Solid::ManagerBasePrivate::managerBackends() const
{
    return m_backends;
}
