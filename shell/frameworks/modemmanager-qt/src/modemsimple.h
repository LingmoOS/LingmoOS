/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEM_SIMPLE_H
#define MODEMMANAGERQT_MODEM_SIMPLE_H

#include <ModemManager/ModemManager.h>

#include <modemmanagerqt_export.h>

#include "generictypes.h"
#include "interface.h"

#include <QDBusPendingReply>

namespace ModemManager
{
class ModemSimplePrivate;

/**
 * @brief The ModemSimple class
 *
 * The Simple interface allows controlling and querying the status of Modems.
 */
class MODEMMANAGERQT_EXPORT ModemSimple : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ModemSimple)

public:
    typedef QSharedPointer<ModemSimple> Ptr;
    typedef QList<Ptr> List;

    explicit ModemSimple(const QString &path, QObject *parent = nullptr);
    ~ModemSimple() override;

    QString uni() const;

    /**
     * Do everything needed to connect the modem using the given properties.
     *
     * This method will attempt to find a matching packet data bearer and activate it if necessary,
     * returning the bearer's IP details. If no matching bearer is found, a new bearer will be created
     * and activated, but this operation may fail if no resources are available to complete this connection
     * attempt (ie, if a conflicting bearer is already active).
     *
     * This call may make a large number of changes to modem configuration based on properties passed in. For
     * example, given a PIN-locked, disabled GSM/UMTS modem, this call may unlock the SIM PIN, alter the access
     * technology preference, wait for network registration (or force registration to a specific provider), create
     * a new packet data bearer using the given "apn", and connect that bearer.
     *
     * Dictionary of properties needed to get the modem connected.
     * Each implementation is free to add its own specific key-value pairs. The predefined
     * common ones are:
     *
     * @param pin SIM-PIN unlock code, given as a string value (signature "s").
     * @param operator-id ETSI MCC-MNC of a network to force registration with, given as a string value (signature "s").
     * @param apn For GSM/UMTS and LTE devices the APN to use, given as a string value (signature "s").
     * @param ip-type For GSM/UMTS and LTE devices the IP addressing type to use, given as a MMBearerIpFamily value (signature "u").
     * @param allowed-auth The authentication method to use, given as a MMBearerAllowedAuth value (signature "u"). Optional in 3GPP.
     * @param user User name (if any) required by the network, given as a string value (signature "s"). Optional in 3GPP.
     * @param password Password (if any) required by the network, given as a string value (signature "s"). Optional in 3GPP.
     * @param number For POTS devices the number to dial,, given as a string value (signature "s").
     * @param allow-roaming FALSE to allow only connections to home networks, given as a boolean value (signature "b").
     * @param rm-protocol For CDMA devices, the protocol of the Rm interface, given as a MMModemCdmaRmProtocol value (signature "u").
     *
     * @return On successful connect, returns the object path of the connected packet data bearer used for the connection attempt.
     */
    QDBusPendingReply<QDBusObjectPath> connectModem(const QVariantMap &properties);

    /**
     *  Dictionary of properties.
     *  Each implementation is free to add it's own specific key-value pairs. The predefined
     *  common ones are:
     *
     * @param state A MMModemState value specifying the overall state of the modem, given as an unsigned integer value (signature "u").
     * @param signal-quality Signal quality value, given only when registered, as an unsigned integer value (signature "u").
     * @param current-bands List of MMModemBand values, given only when registered, as a list of unsigned integer values (signature "au").
     * @param access-technology A MMModemAccessTechnology value, given only when registered, as an unsigned integer value (signature "u").
     * @param m3gpp-registration-state A MMModem3gppRegistrationState value specifying the state of the registration,
     *   given only when registered in a 3GPP network, as an unsigned integer value (signature "u").
     * @param m3gpp-operator-code Operator MCC-MNC, given only when registered in a 3GPP network, as a string value (signature "s").
     * @param m3gpp-operator-name Operator name, given only when registered in a 3GPP network, as a string value (signature "s").
     * @param cdma-cdma1x-registration-state A MMModemCdmaRegistrationState value specifying the state of the registration,
     *   given only when registered in a CDMA1x network, as an unsigned integer value (signature "u").
     * @param cdma-evdo-registration-state A MMModemCdmaRegistrationState value specifying the state of the registration,
     *   given only when registered in a EV-DO network, as an unsigned integer value (signature "u").
     * @param cdma-sid The System Identifier of the serving network, if registered in a CDMA1x network and if known.
     *   Given as an unsigned integer value (signature "u").
     * @param cdma-nid The Network Identifier of the serving network, if registered in a CDMA1x network and if known.
     *   Given as an unsigned integer value (signature "u").
     */
    QDBusPendingReply<QVariantMap> getStatus();

    /**
     * Disconnect an active packet data connection.
     */
    QDBusPendingReply<void> disconnectModem(const QString &bearer);

    /**
     * Convenient method calling disconnectModem with "/" to make ModemManager disconnect all modems
     */
    QDBusPendingReply<void> disconnectAllModems();

    /**
     * Sets the timeout in milliseconds for all async method DBus calls.
     * -1 means the default DBus timeout (usually 25 seconds).
     */
    void setTimeout(int timeout);

    /**
     * Returns the current value of the DBus timeout in milliseconds.
     * -1 means the default DBus timeout (usually 25 seconds).
     */
    int timeout() const;
};

} // namespace ModemManager

#endif
