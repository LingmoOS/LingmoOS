/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDEV_PORTABLEMEDIAPLAYER_H
#define SOLID_BACKENDS_UDEV_PORTABLEMEDIAPLAYER_H

#include "udevdeviceinterface.h"
#include <solid/devices/ifaces/portablemediaplayer.h>

#include <QStringList>

namespace Solid
{
namespace Backends
{
namespace UDev
{
class UDevDevice;

class PortableMediaPlayer : public DeviceInterface, virtual public Solid::Ifaces::PortableMediaPlayer
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::PortableMediaPlayer)

public:
    PortableMediaPlayer(UDevDevice *device);
    ~PortableMediaPlayer() override;

    QStringList supportedProtocols() const override;
    QStringList supportedDrivers(QString protocol = QString()) const override;
    QVariant driverHandle(const QString &driver) const override;

private:
    /**
     * Return full absolute path to media-player-info .mpi file, based on ID_MEDIA_PLAYER
     * udev property. Does not check for existence. Returns empty string in case no reasonable
     * file path could be determined.
     */
    QString mediaPlayerInfoFilePath() const;
};
}
}
}

#endif // SOLID_BACKENDS_UDEV_PORTABLEMEDIAPLAYER_H
