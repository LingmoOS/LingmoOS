/*
    SPDX-FileCopyrightText: 2005-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_DEVICENOTIFIER_H
#define SOLID_DEVICENOTIFIER_H

#include <QObject>

#include <solid/solid_export.h>

namespace Solid
{
/**
 * @class Solid::DeviceNotifier devicenotifier.h <Solid/DeviceNotifier>
 *
 * This class allow to query the underlying system to obtain information
 * about the hardware available.
 *
 * It's the unique entry point for hardware discovery. Applications should use
 * it to find devices, or to be notified about hardware changes.
 *
 * Note that it's implemented as a singleton and encapsulates the backend logic.
 *
 * @author Kevin Ottens <ervin@kde.org>
 */
class SOLID_EXPORT DeviceNotifier : public QObject // krazy:exclude=dpointer (interface class)
{
    Q_OBJECT
public:
    static DeviceNotifier *instance();

Q_SIGNALS:
    /**
     * This signal is emitted when a new device appears in the underlying system.
     *
     * @param udi the new device UDI
     */
    void deviceAdded(const QString &udi);

    /**
     * This signal is emitted when a device disappears from the underlying system.
     *
     * @param udi the old device UDI
     */
    void deviceRemoved(const QString &udi);
};
}

#endif
