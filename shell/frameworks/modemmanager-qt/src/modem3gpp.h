/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEM3GPP_H
#define MODEMMANAGERQT_MODEM3GPP_H

#include <modemmanagerqt_export.h>

#include <QDBusPendingReply>
#include <QObject>
#include <QSharedPointer>

#include "generictypes.h"
#include "interface.h"

namespace ModemManager
{
class Modem3gppPrivate;

/**
 * @brief The Modem3gpp class
 *
 * This class provides access to specific actions that may be performed in modems with 3GPP capabilities.
 */
class MODEMMANAGERQT_EXPORT Modem3gpp : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Modem3gpp)
    Q_FLAGS(MMModem3gppFacility)

public:
    typedef QSharedPointer<Modem3gpp> Ptr;
    typedef QList<Ptr> List;

    Q_DECLARE_FLAGS(FacilityLocks, MMModem3gppFacility)

    explicit Modem3gpp(const QString &path, QObject *parent = nullptr);
    ~Modem3gpp() override;

    /**
     * @return the IMEI of the device
     */
    QString imei() const;

    /**
     * @return mobile registration status as defined in 3GPP TS 27.007 section 10.1.19
     */
    MMModem3gppRegistrationState registrationState() const;

    /**
     * @return code of the operator to which the mobile is currently registered.
     *
     * Returned in the format "MCCMNC", where MCC is the three-digit ITU E.212
     * Mobile Country Code and MNC is the two- or three-digit GSM Mobile Network
     * Code. e.g. e"31026" or "310260".
     *
     * If the MCC and MNC are not known or the mobile is not registered to a
     * mobile network, this property will be an empty string.
     */
    QString operatorCode() const;

    /**
     * @return name of the operator to which the mobile is currently registered.
     *
     * If the operator name is not known or the mobile is not registered to a
     * mobile network, this property will be an empty string.
     */
    QString operatorName() const;

    /**
     * @return country code of the operator to which the mobile is currently registered.
     *
     * Returned in the format "ISO 3166-1 alpha-2" according to the MMC mapping from Wikipedia.
     * Country Code is the two-letter country codes defined in ISO 3166-1, part of the ISO 3166 standard,
     * e.g. "RU" or "FI".
     *
     * If the MCC is not known or the mobile is not registered to a
     * mobile network, this property will be an empty string.
     */
    QString countryCode() const;

    /**
     * @return QFlags of MMModem3gppFacility values for which PIN locking is enabled
     */
    FacilityLocks enabledFacilityLocks() const;

#if MM_CHECK_VERSION(1, 2, 0)
    /**
     * @return Value representing the subscription status of the account and whether there is any data remaining.
     */
    MMModem3gppSubscriptionState subscriptionState() const;
#endif
    /**
     * Register the device to network.
     *
     * @param networkId The operator ID (ie, "MCCMNC", like "310260") to register. An empty string can be used to register to the home network.
     */
    void registerToNetwork(const QString &networkId = QString());

    /**
     * Scan for available networks.
     *
     * @return a QList<QVariantMap> with the results, where each map may contain these values:
     *
     * "status": A MMModem3gppNetworkAvailability value representing network
     * availability status, given as an unsigned integer (signature "u"). This
     * key will always be present.
     *
     * "operator-long": Long-format name of operator, given as a string value
     * (signature "s"). If the name is unknown, this field should not be present.
     *
     * "operator-short": Short-format name of operator, given as a string value
     * (signature "s"). If the name is unknown, this field should not be present.
     *
     * "operator-code": Mobile code of the operator, given as a string value
     * (signature "s"). Returned in the format "MCCMNC", where MCC is the
     * three-digit ITU E.212 Mobile Country Code and MNC is the two- or
     * three-digit GSM Mobile Network Code. e.g. "31026" or "310260".
     *
     * "access-technology": A MMModemAccessTechnology value representing the
     * generic access technology used by this mobile network, given as an
     * unsigned integer (signature "u").
     */
    QDBusPendingReply<QVariantMapList> scan();

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
    void imeiChanged(const QString &imei);
    void registrationStateChanged(MMModem3gppRegistrationState registrationState);
    void operatorCodeChanged(const QString &operatorCode);
    void operatorNameChanged(const QString &operatorName);
    void countryCodeChanged(const QString &countryCode);
    void enabledFacilityLocksChanged(QFlags<MMModem3gppFacility> locks);
    void subscriptionStateChanged(MMModem3gppSubscriptionState subscriptionState);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Modem3gpp::FacilityLocks)

} // namespace ModemManager

#endif
