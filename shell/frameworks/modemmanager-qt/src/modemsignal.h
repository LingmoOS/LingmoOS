/*
    SPDX-FileCopyrightText: 2014 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMSIGNAL_H
#define MODEMMANAGERQT_MODEMSIGNAL_H

#include "interface.h"
#include <modemmanagerqt_export.h>

#include <QDBusPendingReply>
#include <QObject>
#include <QSharedPointer>

namespace ModemManager
{
class ModemSignalPrivate;

/**
 * @brief The ModemSignal class
 *
 * This class provides access to extended signal quality information.
 *
 * @since 1.1.94
 */
class MODEMMANAGERQT_EXPORT ModemSignal : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ModemSignal)

public:
    typedef QSharedPointer<ModemSignal> Ptr;
    typedef QList<Ptr> List;

    explicit ModemSignal(const QString &path, QObject *parent = nullptr);
    ~ModemSignal() override;

    /**
     * @return refresh rate for the extended signal quality information updates, in seconds.
     * A value of 0 disables the retrieval of the values.
     */
    uint rate() const;

    /**
     * @return Dictionary of available signal information for the CDMA1x access technology.
     *
     * This dictionary is composed of a string key, with an associated data which contains type-specific information.
     * @param rssi The CDMA1x RSSI (Received Signal Strength Indication), in dBm, given as a floating point value (signature "d").
     * @param ecio The CDMA1x Ec/Io, in dBm, given as a floating point value (signature "d").
     */
    QVariantMap cdma() const;

    /**
     * @return Dictionary of available signal information for the CDMA EV-DO access technology.
     *
     * This dictionary is composed of a string key, with an associated data which contains type-specific information.
     * @param rssi The CDMA EV-DO RSSI (Received Signal Strength Indication), in dBm, given as a floating point value (signature "d").
     * @param ecio The CDMA EV-DO Ec/Io, in dBm, given as a floating point value (signature "d").
     * @param sinr CDMA EV-DO SINR level, in dB, given as a floating point value (signature "d").
     * @param io The CDMA EV-DO Io, in dBm, given as a floating point value (signature "d").
     */
    QVariantMap evdo() const;

    /**
     * @return Dictionary of available signal information for the GSM/GPRS access technology.
     *
     * This dictionary is composed of a string key, with an associated data which contains type-specific information.
     * @param rssi The GSM RSSI (Received Signal Strength Indication), in dBm, given as a floating point value (signature "d").
     */
    QVariantMap gsm() const;

    /**
     * @return Dictionary of available signal information for the UMTS (WCDMA) access technology.
     *
     * This dictionary is composed of a string key, with an associated data which contains type-specific information.
     * @param rssi The LTE RSSI (Received Signal Strength Indication), in dBm, given as a floating point value (signature "d").
     * @param rsrq The LTE RSRQ (Reference Signal Received Quality), in dB, given as a floating point value (signature "d").
     * @param rsrp The LTE RSRP (Reference Signal Received Power), in dBm, given as a floating point value (signature "d").
     * @param snr The LTE S/R ratio, in dB, given as a floating point value (signature "d").
     */
    QVariantMap lte() const;

    /**
     * @return Dictionary of available signal information for the UMTS (WCDMA) access technology.
     *
     * This dictionary is composed of a string key, with an associated data which contains type-specific information.
     * @param rssi The UMTS RSSI (Received Signal Strength Indication), in dBm, given as a floating point value (signature "d").
     * @param ecio The UMTS Ec/Io, in dBm, given as a floating point value (signature "d").
     */
    QVariantMap umts() const;

    /**
     * Setup extended signal quality information retrieval.
     * @param rate refresh rate to set, in seconds. 0 to disable retrieval.
     */
    QDBusPendingReply<void> setup(uint rate);

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
    void rateChanged(uint rate);
    void cdmaChanged(const QVariantMap &cdma);
    void evdoChanged(const QVariantMap &evdo);
    void gsmChanged(const QVariantMap &gsm);
    void umtsChanged(const QVariantMap &umts);
    void lteChanged(const QString &lte);
};

} // namespace ModemManager

#endif
