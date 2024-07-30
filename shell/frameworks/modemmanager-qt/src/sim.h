/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010-2011 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_SIM_H
#define MODEMMANAGERQT_SIM_H

#include <modemmanagerqt_export.h>

#include <QDBusPendingReply>
#include <QObject>
#include <QSharedPointer>

#include "generictypes.h"

namespace ModemManager
{
class SimPrivate;

/**
 * @brief The Sim class
 *
 * The SIM class handles communication with SIM, USIM, and RUIM (CDMA SIM) cards.
 */
class MODEMMANAGERQT_EXPORT Sim : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Sim)

public:
    typedef QSharedPointer<Sim> Ptr;
    typedef QList<Ptr> List;

    explicit Sim(const QString &path, QObject *parent = nullptr);
    ~Sim() override;

    /**
     * @return Boolean indicating whether the SIM is currently active.
     *
     * On systems that support Multi SIM Single Standby, only one SIM may be
     * active at any given time, which will be the one considered primary.

     * On systems that support Multi SIM Multi Standby, more than one SIM may
     * be active at any given time, but only one of them is considered primary.
     */
    bool active() const;

    /**
     * @return The ICCID of the SIM card.
     *
     * This may be available before the PIN has been entered depending on the device itself.
     */
    QString simIdentifier() const;

    /**
     * @return The IMSI of the SIM card, if any.
     */
    QString imsi() const;

    /**
     * @return The EID of the SIM card, if any.
     */
    QString eid() const;

    /**
     * @return The ID of the network operator, as given by the SIM card, if known.
     */
    QString operatorIdentifier() const;

    /**
     * @return The name of the network operator, as given by the SIM card, if known.
     */
    QString operatorName() const;

    /**
     * @return List of emergency numbers programmed in the SIM card.
     *
     * These numbers should be treated as numbers for emergency calls in
     * addition to 112 and 911.
     */
    QStringList emergencyNumbers() const;

    /**
     * @return Map of preferred networks with access technologies configured in the SIM card.
     *
     * Each entry contains an operator id string key "MCCMNC"
     * consisting of 5 or 6 digits, to an MMModemAccessTechnology mask value.
     * If the SIM card does not support access technology storage, the mask will be
     * set to MM_MODEM_ACCESS_TECHNOLOGY_UNKNOWN.
     */
    QVariantMap preferredNetworks() const;

#if MM_CHECK_VERSION(1, 20, 0)
    /**
     * @return Group identifier 1evel 1.
     */
    QByteArray gid1() const;

    /**
     * @return Group identifier 1evel 2.
     */
    QByteArray gid2() const;

    /**
     * @return Indicates whether the current primary SIM is a ESIM or a physical SIM.
     */
    MMSimType simType() const;

    /**
     * @return If current SIM is ESIM then this indicates whether there is a profile or not.
     */
    MMSimEsimStatus esimStatus() const;

    /**
     * @return Indicates whether the current SIM is a removable SIM or not.
     */
    MMSimRemovability removability() const;
#endif

    /**
     * Send the PIN to unlock the SIM card.
     * @param pin A string containing the PIN code.
     */
    QDBusPendingReply<> sendPin(const QString &pin);

    /**
     * Send the PUK and a new PIN to unlock the SIM card.
     * @param puk A string containing the PUK code.
     * @param pin A string containing the PIN code.
     */
    QDBusPendingReply<> sendPuk(const QString &puk, const QString &pin);

    /**
     * Enable or disable the PIN checking.
     * @param pin A string containing the PIN code.
     * @param enabled TRUE to enable PIN checking, FALSE otherwise.
     */
    QDBusPendingReply<> enablePin(const QString &pin, bool enabled);

    /**
     * Change the PIN code.
     * @param oldPin A string containing the current PIN code.
     * @param newPin A string containing the new PIN code.
     */
    QDBusPendingReply<> changePin(const QString &oldPin, const QString &newPin);

    /**
     * @param preferred_plmns List of preferred networks.
     *
     * Stores the provided preferred network list to the SIM card. Each entry contains
     * an operator id string ("MCCMNC") consisting of 5 or 6 digits,
     * and an MMModemAccessTechnology mask to store to SIM card if supported.
     *
     * This method removes any pre-existing entries of the preferred network list. Note
     * that even if this operation fails, the preferred network list on the SIM card may
     * have changed.
     */
    QDBusPendingReply<> setPreferredNetworks(QVariantMap preferredNetworks);

    QString uni() const;

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
    void activeChanged(bool active);
    void simIdentifierChanged(const QString &identifier);
    void imsiChanged(const QString &imsi);
    void eidChanged(const QString &eid);
    void operatorIdentifierChanged(const QString &identifier);
    void operatorNameChanged(const QString &name);
    void emergencyNumbersChanged(const QStringList &emergencyNumbers);
    void preferredNetworksChanged(const QVariantMap &preferredNetworks);
#if MM_CHECK_VERSION(1, 20, 0)
    void gid1Changed(const QByteArray &gid1);
    void gid2Changed(const QByteArray &gid2);
    void simTypeChanged(MMSimType simType);
    void esimStatusChanged(MMSimEsimStatus esimStatus);
    void removabilityChanged(MMSimRemovability removability);
#endif

private:
    SimPrivate *const d_ptr;
};
} // namespace ModemManager

#endif
