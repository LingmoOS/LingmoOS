/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MANAGER_H
#define MODEMMANAGERQT_MANAGER_H

#include <modemmanagerqt_export.h>

#include <QObject>
#include <QSharedPointer>

#include "modemdevice.h"

/**
 * This namespace allows to query the underlying system to discover the available
 * modem interfaces
 * responsibility to notify when a modem interface or a modem appears or disappears.
 *
 * Note that it's implemented as a singleton and encapsulates the backend logic.
 */
namespace ModemManager
{
class Modem;

class MODEMMANAGERQT_EXPORT Notifier : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    /**
     * This signal is emitted when a new modem interface is available.
     *
     * @param udi the network interface identifier
     */
    void modemAdded(const QString &udi);
    /**
     * This signal is emitted when a network interface is not available anymore.
     *
     * @param udi the network interface identifier
     */
    void modemRemoved(const QString &udi);

    /**
     * This signal is emitted when the ModemManager DBus service goes away
     */
    void serviceDisappeared();
    /**
     * This signal is emitted when the ModemManager DBus service appears
     */
    void serviceAppeared();
};

/**
 * Retrieves the list of all modem interfaces Unique Device Identifiers (UDIs)
 * in the system. This method is the equivalent of enumerateDevices described
 * in Modem Manager specification.
 *
 * Note: only HW modems are returned (Gsm or Cdma)
 *
 * @return the list of modem interfaces available in this system
 */
MODEMMANAGERQT_EXPORT ModemDevice::List modemDevices();

/**
 * Find a new ModemManagerInterface object given its UDI.
 *
 * Note: only Modem-inherited objects are returned (not SMS, SIM or Bearer objects)
 *
 * @param uni the identifier of the modem interface to find
 * @returns a valid Modem object if there's a device having the given UDI, an invalid one otherwise
 */
MODEMMANAGERQT_EXPORT ModemDevice::Ptr findModemDevice(const QString &uni);

/**
 * Start a new scan for connected modem devices.
 */
MODEMMANAGERQT_EXPORT void scanDevices();

MODEMMANAGERQT_EXPORT Notifier *notifier();
}

#endif
