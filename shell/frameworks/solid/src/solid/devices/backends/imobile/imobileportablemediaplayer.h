/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IMOBILE_IMOBILEPORTABLEMEDIAPLAYER_H
#define SOLID_BACKENDS_IMOBILE_IMOBILEPORTABLEMEDIAPLAYER_H

#include <solid/devices/ifaces/portablemediaplayer.h>

#include <QStringList>

#include "imobiledeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace IMobile
{
class IMobileDevice;

class PortableMediaPlayer : public DeviceInterface, virtual public Solid::Ifaces::PortableMediaPlayer
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::PortableMediaPlayer)

public:
    PortableMediaPlayer(IMobileDevice *device);
    ~PortableMediaPlayer() override;

    QStringList supportedProtocols() const override;
    QStringList supportedDrivers(QString protocol = QString()) const override;
    QVariant driverHandle(const QString &driver) const override;
};

} // namespace IMobile
} // namespace Backends
} // namespace Solid

#endif // SOLID_BACKENDS_IMOBILE_IMOBILEPORTABLEMEDIAPLAYER_H
