/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEM_H
#define MODEMMANAGERQT_MODEM_H

#include <ModemManager/ModemManager.h>

#include <modemmanagerqt_export.h>

#include <QDBusObjectPath>
#include <QObject>
#include <QSharedPointer>

#include "bearer.h"
#include "generictypes.h"
#include "interface.h"

namespace ModemManager
{
class ModemPrivate;

/**
 * This class represents bearer properties used for creating of new bearers
 */
class MODEMMANAGERQT_EXPORT BearerProperties
{
public:
    /**
     * Constructs an empty BearerProperties object
     */
    BearerProperties();

    /**
     * Destroys this BearerProperties object.
     */
    ~BearerProperties();

    /**
     * Constructs a BearerProperties object that is a copy of the object @p other.
     */
    BearerProperties(const BearerProperties &other);

    /**
     * Returns Access Point Name
     */
    QString apn() const;

    /**
     * Sets Access Point Name
     */
    void setApn(const QString &apn);

    /**
     * Returns addressing type
     */
    MMBearerIpFamily ipType() const;

    /**
     * Sets addressing type
     */
    void setIpType(MMBearerIpFamily ipType);

    /**
     * Returns used authentication method
     */
    MMBearerAllowedAuth allowedAuthentication() const;

    /**
     * Sets the authentication method to use
     */
    void setAllowedAuthentication(MMBearerAllowedAuth allowedAuth);

    /**
     * Returns user name
     */
    QString user() const;

    /**
     * Sets user name
     */
    void setUser(const QString &user);

    /**
     * Returns password
     */
    QString password() const;

    /**
     * Sets password
     */
    void setPassword(const QString &password);

    /**
     * Returns whether connection is allowed during roaming
     */
    bool allowRoaming() const;

    /**
     * Sets whether connection is allowed during roaming
     */
    void setAllowRoaming(bool allow);

    /**
     * Returns protocol of the Rm interface
     */
    MMModemCdmaRmProtocol rmProtocol() const;

    /**
     * Sets protocol of the Rm interface
     */
    void setRmProtocol(MMModemCdmaRmProtocol rmProtocol);

    /**
     * Returns telephone number to dial
     */
    QString number() const;

    /**
     * Sets telephone number to dial
     */
    void setNumber(const QString &number);

    /**
     * Makes a copy of the IpConfig object @p other.
     */
    BearerProperties &operator=(const BearerProperties &other);

private:
    class Private;
    Private *const d;
};

/**
 * @brief The Modem class
 *
 * The Modem interface controls the status and actions in a given modem object.
 */
class MODEMMANAGERQT_EXPORT Modem : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Modem)

    Q_FLAGS(MMModemCapability)
    Q_FLAGS(MMModemAccessTechnology)
    Q_FLAGS(MMModemMode)
    Q_FLAGS(MMBearerIpFamily)

public:
    Q_DECLARE_FLAGS(Capabilities, MMModemCapability)
    Q_DECLARE_FLAGS(AccessTechnologies, MMModemAccessTechnology)
    Q_DECLARE_FLAGS(ModemModes, MMModemMode)
    Q_DECLARE_FLAGS(IpBearerFamilies, MMBearerIpFamily)

    typedef QSharedPointer<Modem> Ptr;
    typedef QList<Ptr> List;

    explicit Modem(const QString &path, QObject *parent = nullptr);
    ~Modem() override;

    QString uni() const;
    /**
     * @return @p true if the modem is fully functional, @p false when in low power mode or disabled
     * @see setEnabled()
     */
    bool isEnabled() const;
    bool isValid() const;

    /**
     * Enable or disable the modem.
     *
     * When enabled, the modem's radio is powered on and data sessions, voice calls, location services, and Short Message Service may be available.
     *
     * When disabled, the modem enters low-power state and no network-related operations are available.
     */
    QDBusPendingReply<void> setEnabled(bool enable);

    /**
     * Create a new packet data bearer using the given characteristics.
     *
     * This request may fail if the modem does not support additional bearers, if too many bearers are already defined, or if properties are invalid.
     *
     */
    QDBusPendingReply<QDBusObjectPath> createBearer(const ModemManager::BearerProperties &bearerProperties);

    /**
     * Delete an existing packet data bearer.
     *
     * If the bearer is currently active and providing packet data server, it will be disconnected and that packet data service will terminate.
     * @param bearer path to the bearer to delete
     */
    QDBusPendingReply<void> deleteBearer(const QString &bearer);

    /**
     * @return the configured packet data bearers (EPS Bearers, PDP Contexts, or CDMA2000 Packet Data Sessions).
     */
    ModemManager::Bearer::List listBearers() const;

    /**
     * @return the configured packet data bearer on given path
     */
    ModemManager::Bearer::Ptr findBearer(const QString &bearer) const;

    /**
     * Clear non-persistent configuration and state, and return the device to a newly-powered-on state.
     *
     * This command may power-cycle the device.
     */
    QDBusPendingReply<void> reset();

    /**
     * Clear the modem's configuration (including persistent configuration and state), and return the device to a factory-default state.
     *
     * If not required by the modem, @p code may be ignored.
     *
     * This command may or may not power-cycle the device.
     * @param code Carrier-supplied code required to reset the modem.
     */
    QDBusPendingReply<void> factoryReset(const QString &code);

    /**
     * Set the power @p state of the modem. This action can only be run when the modem is in MM_MODEM_STATE_DISABLED state.
     */
    QDBusPendingReply<void> setPowerState(MMModemPowerState state);

    /**
     * Set the capabilities of the device. A restart of the modem may be required.
     * @param caps QFlags of MMModemCapability values, to specify the capabilities to use.
     */
    QDBusPendingReply<void> setCurrentCapabilities(Capabilities caps);

    /**
     * Set the access technologies (e.g. 2G/3G/4G preference) the device is currently allowed to use when connecting to a network.
     *
     * The given combination should be supported by the modem, as specified in supportedModes()
     * @param mode
     */
    QDBusPendingReply<void> setCurrentModes(const CurrentModesType &mode);

    /**
     * Set the radio frequency and technology bands the device is currently allowed to use when connecting to a network.
     * @param bands List of MMModemBand values, to specify the bands to be used.
     */
    QDBusPendingReply<void> setCurrentBands(const QList<MMModemBand> &bands);

    QDBusPendingReply<QString> command(const QString &cmd, uint timeout);

    /**
     * @return The path of the SIM object available in this device, if any.
     */
    QString simPath() const;

    /**
     * @return List of MMModemCapability values, specifying the combinations of generic family of access technologies the modem supports.
     *
     * If the modem doesn't allow changing the current capabilities, a single entry with MM_MODEM_CAPABILITY_ANY will be given.
     */
    QList<MMModemCapability> supportedCapabilities() const;

    /**
     * @return QFlags of MMModemCapability values, specifying the generic family of
     * access technologies the modem currently supports without a firmware
     * reload or reinitialization.
     */
    Capabilities currentCapabilities() const;

    /**
     * @return The maximum number of defined packet data bearers the modem supports.
     *
     * This is not the number of active/connected bearers the modem supports,
     * but simply the number of bearers that may be defined at any given time.
     * For example, POTS and CDMA2000-only devices support only one bearer,
     * while GSM/UMTS devices typically support three or more, and any
     * LTE-capable device (whether LTE-only, GSM/UMTS-capable, and/or
     * CDMA2000-capable) also typically support three or more.
     */
    uint maxBearers() const;

    /**
     * @return The maximum number of active packet data bearers the modem supports.
     *
     * POTS and CDMA2000-only devices support one active bearer, while GSM/UMTS
     * and LTE-capable devices (including LTE/CDMA devices) typically support at
     * least two active bearers.
     */
    uint maxActiveBearers() const;

    /**
     * @return The equipment manufacturer, as reported by the modem.
     */
    QString manufacturer() const;

    /**
     * @return The equipment model, as reported by the modem.
     */
    QString model() const;

    /**
     * @return The revision identification of the software, as reported by the modem.
     */
    QString revision() const;

    /**
     * @return A best-effort device identifier based on various device
     * information like model name, firmware revision, USB/PCI/PCMCIA IDs, and
     * other properties.
     *
     * This ID is not guaranteed to be unique and may be shared between
     * identical devices with the same firmware, but is intended to be "unique
     * enough" for use as a casual device identifier for various user experience
     * operations.
     *
     * This is not the device's IMEI or ESN since those may not be available
     * before unlocking the device via a PIN.
     */
    QString deviceIdentifier() const;

    /**
     * @return The physical modem device reference (ie, USB, PCI, PCMCIA device), which may be dependent upon the operating system.
     *
     * In Linux for example, this points to a sysfs path of the usb_device object.
     */
    QString device() const;

    /**
     * @return The Operating System device drivers handling communication with the modem hardware.
     */
    QStringList drivers() const;

    /**
     * @return The name of the plugin handling this modem.
     */
    QString plugin() const;

    /**
     * @return The name of the primary port using to control the modem.
     */
    QString primaryPort() const;

    /**
     * @return The list of ports in the modem, given as an array of string and unsigned integer pairs.
     * The string is the port name or path, and the integer is the port type given as a MMModemPortType value.
     *
     * @since 1.1.94
     */
    PortList ports() const;

    /**
     * @return The identity of the device.
     *
     * This will be the IMEI number for GSM devices and the hex-format ESN/MEID for CDMA devices.
     */
    QString equipmentIdentifier() const;

    /**
     * @return Current lock state of the device, given as a MMModemLock value.
     */
    MMModemLock unlockRequired() const;

    /**
     * @return A dictionary in which the keys are MMModemLock flags, and the
     * values are integers giving the number of PIN tries remaining before the
     * code becomes blocked (requiring a PUK) or permanently blocked. Dictionary
     * entries exist only for the codes for which the modem is able to report
     * retry counts.
     */
    UnlockRetriesMap unlockRetries() const;

    /**
     * @return Overall state of the modem, given as a MMModemState value.
     *
     * If the device's state cannot be determined, MM_MODEM_STATE_UNKNOWN will be reported.
     */
    MMModemState state() const;

    /**
     * @return Error specifying why the modem is in MM_MODEM_STATE_FAILED state, given as a MMModemStateFailedReason value.
     */
    MMModemStateFailedReason stateFailedReason() const;

    /**
     * @return QFlags of MMModemAccessTechnology values, specifying the current
     * network access technologies used by the device to communicate with the
     * network.
     *
     * If the device's access technology cannot be determined, MM_MODEM_ACCESS_TECHNOLOGY_UNKNOWN will be reported.
     */
    AccessTechnologies accessTechnologies() const;

    /**
     * @return Signal quality in percent (0 - 100) of the dominant access
     * technology the device is using to communicate with the network. Always 0
     * for POTS devices.
     *
     * The additional boolean value indicates if the quality value given was recently taken.
     */
    SignalQualityPair signalQuality() const;

    /**
     * @return List of numbers (e.g. MSISDN in 3GPP) being currently handled by this modem.
     */
    QStringList ownNumbers() const;

    /**
     * @return A MMModemPowerState value specifying the current power state of the modem.
     */
    MMModemPowerState powerState() const;

    /**
     * @return This property exposes the supported mode combinations, given as an list of unsigned integer pairs, where:
     * The first integer is a bitmask of MMModemMode values, specifying the allowed modes.
     * The second integer is a single MMModemMode, which specifies the preferred access technology, among the ones defined in the allowed modes.
     */
    SupportedModesType supportedModes() const;

    /**
     * @return A pair of MMModemMode values, where the first one is a bitmask
     * specifying the access technologies (eg 2G/3G/4G) the device is currently
     * allowed to use when connecting to a network, and the second one is the
     * preferred mode of those specified as allowed.
     *
     * The pair must be one of those specified in supportedModes()
     */
    CurrentModesType currentModes() const;

    /**
     * @return List of MMModemBand values, specifying the radio frequency and technology bands supported by the device.
     *
     * For POTS devices, only the MM_MODEM_BAND_ANY mode will be returned.
     */
    QList<MMModemBand> supportedBands() const;

    /**
     * @return List of MMModemBand values, specifying the radio frequency and
     * technology bands the device is currently using when connecting to a
     * network.
     *
     * It must be a subset of supportedBands()
     */
    QList<MMModemBand> currentBands() const;

    /**
     * @return QFlags of MMBearerIpFamily values, specifying the IP families supported by the device.
     */
    IpBearerFamilies supportedIpFamilies() const;

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
    void bearerAdded(const QString &bearer);
    void bearerRemoved(const QString &bearer);
    void bearersChanged();

    void simPathChanged(const QString &oldPath, const QString &newPath);
    void supportedCapabilitiesChanged(const QList<MMModemCapability> &supportedCapabilities);
    void currentCapabilitiesChanged(const QFlags<MMModemCapability> &currentCapabilities);
    void maxBearersChanged(uint maxBearers);
    void maxActiveBearersChanged(uint maxActiveBearers);
    void manufacturerChanged(const QString &manufacturer);
    void modelChanged(const QString &model);
    void revisionChanged(const QString &revision);
    void deviceIdentifierChanged(const QString &deviceIdentifier);
    void deviceChanged(const QString &device);
    void driversChanged(const QStringList &drivers);
    void pluginChanged(const QString &plugin);
    void primaryPortChanged(const QString &primaryPort);
    void portsChanged(const ModemManager::PortList &ports);
    void equipmentIdentifierChanged(const QString &equipmentIdentifier);
    void unlockRequiredChanged(MMModemLock unlockRequired);
    void unlockRetriesChanged(const ModemManager::UnlockRetriesMap &unlockRetries);
    void stateFailedReasonChanged(MMModemStateFailedReason stateFailedReason);
    void accessTechnologiesChanged(QFlags<MMModemAccessTechnology> accessTechnologies);
    void signalQualityChanged(ModemManager::SignalQualityPair signalQuality);
    void ownNumbersChanged(const QStringList &ownNumbers);
    void powerStateChanged(MMModemPowerState powerState);
    void supportedModesChanged(ModemManager::SupportedModesType supportedModes);
    void currentModesChanged(ModemManager::CurrentModesType currentModes);
    void supportedBandsChanged(const QList<MMModemBand> &supportedBands);
    void currentBandsChanged(const QList<MMModemBand> &supportedBands);
    void supportedIpFamiliesChanged(QFlags<MMBearerIpFamily> supportedIpFamilies);
    void stateChanged(MMModemState oldState, MMModemState newState, MMModemStateChangeReason reason);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Modem::Capabilities)
Q_DECLARE_OPERATORS_FOR_FLAGS(Modem::AccessTechnologies)
Q_DECLARE_OPERATORS_FOR_FLAGS(Modem::ModemModes)
Q_DECLARE_OPERATORS_FOR_FLAGS(Modem::IpBearerFamilies)

} // namespace ModemManager

#endif
