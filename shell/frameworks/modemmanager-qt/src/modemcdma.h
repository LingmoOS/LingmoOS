/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMCDMA_H
#define MODEMMANAGERQT_MODEMCDMA_H

#include <modemmanagerqt_export.h>

#include <QDBusPendingReply>
#include <QObject>
#include <QSharedPointer>
#include <QVariant>

#include "generictypes.h"
#include "interface.h"

namespace ModemManager
{
class ModemCdmaPrivate;

/**
 * @brief The ModemCdma class
 *
 * This class provides access to specific actions that may be performed in modems with CDMA capabilities.
 */
class MODEMMANAGERQT_EXPORT ModemCdma : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ModemCdma)
public:
    typedef QSharedPointer<ModemCdma> Ptr;
    typedef QList<Ptr> List;

    explicit ModemCdma(const QString &path, QObject *parent = nullptr);
    ~ModemCdma() override;

    /**
     * Provisions the modem for use with a given carrier using the modem's
     * Over-The-Air (OTA) activation functionality, if any.
     *
     * Some modems will reboot after this call is made.
     *
     * @param carrierCode name of carrier, or carrier-specific code
     */
    QDBusPendingReply<void> activate(const QString &carrierCode);

    /**
     * Sets the modem provisioning data directly, without contacting the carrier over the air.
     *
     * Some modems will reboot after this call is made.
     *
     * @param properties QVariantMap consisting of:
     *
     * "spc": The Service Programming Code, given as a string of exactly 6 digit characters. Mandatory parameter.
     * "sid": The System Identification Number, given as a 16-bit unsigned integer (signature "q"). Mandatory parameter.
     * "mdn": The Mobile Directory Number, given as a string of maximum 15 characters. Mandatory parameter.
     * "min": The Mobile Identification Number, given as a string of maximum 15 characters. Mandatory parameter.
     * "mn-ha-key": The MN-HA key, given as a string of maximum 16 characters.
     * "mn-aaa-key": The MN-AAA key, given as a string of maximum 16 characters.
     * "prl": The Preferred Roaming List, given as an array of maximum 16384 bytes.
     */
    QDBusPendingReply<void> activateManual(const QVariantMap &properties);

    /**
     * @return a MMModemCdmaActivationState value specifying the state of the activation in the 3GPP2 network.
     */
    MMModemCdmaActivationState activationState() const;

    /**
     * @return the modem's Mobile Equipment Identifier.
     */
    QString meid() const;

    /**
     * @return the modem's Electronic Serial Number (superseded by MEID but still used by older devices).
     */
    QString esn() const;

    /**
     * @return the System Identifier of the serving CDMA 1x network, if known, and if the modem is registered with a CDMA 1x network.
     * @see http://ifast.org or the mobile broadband provider database for mappings of SIDs to network providers.
     */
    uint sid() const;

    /**
     * @return the Network Identifier of the serving CDMA 1x network, if known, and if the modem is registered with a CDMA 1x network.
     */
    uint nid() const;

    /**
     * @return a MMModemCdmaRegistrationState value specifying the CDMA 1x registration state.
     */
    MMModemCdmaRegistrationState cdma1xRegistrationState() const;

    /**
     * @return a MMModemCdmaRegistrationState value specifying the EVDO registration state.
     */
    MMModemCdmaRegistrationState evdoRegistrationState() const;

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

Q_SIGNALS:
    /**
     * This signal is emitted when the activation info this network changes
     *
     * @param state current activation state, given as a MMModemCdmaActivationState.
     * @param error carrier-specific error code, given as a MMCdmaActivationError.
     * @param status_changes properties that have changed as a result of this activation state change, including "mdn" and "min".
     *                       The map may be empty if the changed properties are unknown.
     */
    void activationStateChanged(MMModemCdmaActivationState state, MMCdmaActivationError error, const QVariantMap &status_changes);
    void meidChanged(const QString &meid);
    void esnChanged(const QString &esn);
    void sidChanged(uint sid);
    void nidChanged(uint nid);
    void cdma1xRegistrationStateChanged(MMModemCdmaRegistrationState cdma1xRegistrationState);
    void evdoRegistrationStateChanged(MMModemCdmaRegistrationState evdoRegistrationState);
};
} // namespace ModemManager

#endif
