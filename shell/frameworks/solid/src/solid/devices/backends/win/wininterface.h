/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WININTERFACE_H
#define WININTERFACE_H

#include <solid/devices/ifaces/deviceinterface.h>

#include "windevice.h"

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)

public:
    WinInterface(WinDevice *device);
    virtual ~WinInterface();

protected:
    WinDevice *m_device;
};

}
}
}

#endif // WININTERFACE_H
