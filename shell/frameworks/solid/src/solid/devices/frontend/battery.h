/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2012 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BATTERY_H
#define SOLID_BATTERY_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
class BatteryPrivate;
class Device;

/**
 * This device interface is available on batteries.
 */
class SOLID_EXPORT Battery : public DeviceInterface
{
    Q_OBJECT
    Q_PROPERTY(bool present READ isPresent NOTIFY presentStateChanged)
    Q_PROPERTY(BatteryType type READ type CONSTANT)
    Q_PROPERTY(int chargePercent READ chargePercent NOTIFY chargePercentChanged)
    Q_PROPERTY(int capacity READ capacity NOTIFY capacityChanged)
    Q_PROPERTY(bool rechargeable READ isRechargeable CONSTANT)
    Q_PROPERTY(bool powerSupply READ isPowerSupply NOTIFY powerSupplyStateChanged)
    Q_PROPERTY(ChargeState chargeState READ chargeState NOTIFY chargeStateChanged)
    Q_PROPERTY(qlonglong timeToEmpty READ timeToEmpty NOTIFY timeToEmptyChanged)
    Q_PROPERTY(qlonglong timeToFull READ timeToFull NOTIFY timeToFullChanged)
    Q_PROPERTY(double energy READ energy NOTIFY energyChanged)
    Q_PROPERTY(double energyFull READ energyFull NOTIFY energyFullChanged)
    Q_PROPERTY(double energyFullDesign READ energyFullDesign NOTIFY energyFullDesignChanged)
    Q_PROPERTY(double energyRate READ energyRate NOTIFY energyRateChanged)
    Q_PROPERTY(double voltage READ voltage NOTIFY voltageChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(Technology technology READ technology CONSTANT)
    Q_PROPERTY(QString serial READ serial CONSTANT)
    Q_PROPERTY(qlonglong remainingTime READ remainingTime NOTIFY remainingTimeChanged)
    Q_DECLARE_PRIVATE(Battery)
    friend class Device;

public:
    /**
     * This enum type defines the type of the device holding the battery
     *
     * - PdaBattery : A battery in a Personal Digital Assistant
     * - UpsBattery : A battery in an Uninterruptible Power Supply
     * - PrimaryBattery : A primary battery for the system (for example laptop battery)
     * - MouseBattery : A battery in a mouse
     * - KeyboardBattery : A battery in a keyboard
     * - KeyboardMouseBattery : A battery in a combined keyboard and mouse
     * - CameraBattery : A battery in a camera
     * - PhoneBattery : A battery in a phone
     * - MonitorBattery : A battery in a monitor
     * - GamingInputBattery : A battery in a gaming input device (for example a wireless game pad)
     * - BluetoothBattery: A generic Bluetooth device battery (if its type isn't known, a Bluetooth
     *                     mouse would normally show up as a MouseBattery), @since 5.54
     * - TabletBattery : A battery in a graphics tablet or pen, @since 5.88
     * - HeadphoneBattery : A battery in a headphone, @since 6.0
     * - HeadsetBattery : A battery in a headset, @since 6.0
     * - TouchpadBattery : A battery in a touchpad. This is how the Dualsense Wireless Controller is categorized @since 6.0
     * - UnknownBattery : A battery in an unknown device
     */
    enum BatteryType {
        UnknownBattery,
        PdaBattery,
        UpsBattery,
        PrimaryBattery,
        MouseBattery,
        KeyboardBattery,
        KeyboardMouseBattery,
        CameraBattery,
        PhoneBattery,
        MonitorBattery,
        GamingInputBattery,
        BluetoothBattery,
        TabletBattery,
        HeadphoneBattery,
        HeadsetBattery,
        TouchpadBattery,
    };
    Q_ENUM(BatteryType)

    /**
     * This enum type defines charge state of a battery
     *
     * - NoCharge : Battery charge is stable, not charging or discharging or
     *              the state is Unknown
     * - Charging : Battery is charging
     * - Discharging : Battery is discharging
     * - FullyCharged: The battery is fully charged; a battery not necessarily
     *                 charges up to 100%
     */
    enum ChargeState { NoCharge, Charging, Discharging, FullyCharged };
    Q_ENUM(ChargeState)

    /**
     * Technology used in the battery
     *
     * 0: Unknown
     * 1: Lithium ion
     * 2: Lithium polymer
     * 3: Lithium iron phosphate
     * 4: Lead acid
     * 5: Nickel cadmium
     * 6: Nickel metal hydride
     */
    enum Technology {
        UnknownTechnology = 0,
        LithiumIon,
        LithiumPolymer,
        LithiumIronPhosphate,
        LeadAcid,
        NickelCadmium,
        NickelMetalHydride,
    };
    Q_ENUM(Technology)

private:
    /**
     * Creates a new Battery object.
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * @param backendObject the device interface object provided by the backend
     * @see Solid::Device::as()
     */
    SOLID_NO_EXPORT explicit Battery(QObject *backendObject);

public:
    /**
     * Destroys a Battery object.
     */
    ~Battery() override;

    /**
     * Get the Solid::DeviceInterface::Type of the Battery device interface.
     *
     * @return the Battery device interface type
     * @see Solid::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::Battery;
    }

    /**
     * Indicates if this battery is currently present in its bay.
     *
     * @return true if the battery is present, false otherwise
     */
    bool isPresent() const;

    /**
     * Retrieves the type of device holding this battery.
     *
     * @return the type of device holding this battery
     * @see Solid::Battery::BatteryType
     */
    Solid::Battery::BatteryType type() const;

    /**
     * Retrieves the current charge level of the battery normalised
     * to percent.
     *
     * @return the current charge level normalised to percent
     */
    int chargePercent() const;

    /**
     * Retrieves the battery capacity normalised to percent,
     * meaning how much energy can it hold compared to what it is designed to.
     * The capacity of the battery will reduce with age.
     * A capacity value less than 75% is usually a sign that you should renew your battery.
     *
     * @since 4.11
     * @return the battery capacity normalised to percent
     */
    int capacity() const;

    /**
     * Indicates if the battery is rechargeable.
     *
     * @return true if the battery is rechargeable, false otherwise (one time usage)
     */
    bool isRechargeable() const;

    /**
     * Indicates if the battery is powering the machine.
     *
     * @return true if the battery is powersupply, false otherwise
     */
    bool isPowerSupply() const;

    /**
     * Retrieves the current charge state of the battery. It can be in a stable
     * state (no charge), charging or discharging.
     *
     * @return the current battery charge state
     * @see Solid::Battery::ChargeState
     */
    Solid::Battery::ChargeState chargeState() const;

    /**
     * Time (in seconds) until the battery is empty.
     *
     * @return time until the battery is empty
     * @since 5.0
     */
    qlonglong timeToEmpty() const;

    /**
     * Time (in seconds) until the battery is full.
     *
     * @return time until the battery is full
     * @since 5.0
     */
    qlonglong timeToFull() const;

    /**
     * Retrieves the technology used to manufacture the battery.
     *
     * @return the battery technology
     * @see Solid::Battery::Technology
     */
    Solid::Battery::Technology technology() const;

    /**
     * Amount of energy (measured in Wh) currently available in the power source.
     *
     * @return amount of battery energy in Wh
     */
    double energy() const;

    /**
     * Amount of energy (measured in Wh) the battery has when it is full.
     *
     * @return amount of battery energy when full in Wh
     * @since 5.7
     */
    double energyFull() const;

    /**
     * Amount of energy (measured in Wh) the battery should have by design hen it is full.
     *
     * @return amount of battery energy when full by design in Wh
     * @since 5.7
     */
    double energyFullDesign() const;

    /**
     * Amount of energy being drained from the source, measured in W.
     * If positive, the source is being discharged, if negative it's being charged.
     *
     * @return battery rate in Watts
     *
     */
    double energyRate() const;

    /**
     * Voltage in the Cell or being recorded by the meter.
     *
     * @return voltage in Volts
     */
    double voltage() const;

    /**
     * The temperature of the battery in degrees Celsius.
     *
     * @return the battery temperature in degrees Celsius
     * @since 5.0
     */
    double temperature() const;

    /**
     * The serial number of the battery
     *
     * @return the serial number of the battery
     * @since 5.0
     */
    QString serial() const;

    /**
     * Retrieves the current estimated remaining time of the system batteries
     *
     * @return the current global estimated remaining time in seconds
     * @since 5.8
     */
    qlonglong remainingTime() const;

Q_SIGNALS:
    /**
     * This signal is emitted if the battery gets plugged in/out of the
     * battery bay.
     *
     * @param newState the new plugging state of the battery, type is boolean
     * @param udi the UDI of the battery with thew new plugging state
     */
    void presentStateChanged(bool newState, const QString &udi);

    /**
     * This signal is emitted when the charge percent value of this
     * battery has changed.
     *
     * @param value the new charge percent value of the battery
     * @param udi the UDI of the battery with the new charge percent
     */
    void chargePercentChanged(int value, const QString &udi);

    /**
     * This signal is emitted when the capacity of this battery has changed.
     *
     * @param value the new capacity of the battery
     * @param udi the UDI of the battery with the new capacity
     * @since 4.11
     */
    void capacityChanged(int value, const QString &udi);

    /**
     * This signal is emitted when the power supply state of the battery
     * changes.
     *
     * @param newState the new power supply state, type is boolean
     * @param udi the UDI of the battery with the new power supply state
     * @since 4.11
     */
    void powerSupplyStateChanged(bool newState, const QString &udi);

    /**
     * This signal is emitted when the charge state of this battery
     * has changed.
     *
     * @param newState the new charge state of the battery, it's one of
     * the type Solid::Battery::ChargeState
     * @see Solid::Battery::ChargeState
     * @param udi the UDI of the battery with the new charge state
     */
    void chargeStateChanged(int newState, const QString &udi = QString());

    /**
     * This signal is emitted when the time until the battery is empty
     * has changed.
     *
     * @param time the new remaining time
     * @param udi the UDI of the battery with the new remaining time
     * @since 5.0
     */
    void timeToEmptyChanged(qlonglong time, const QString &udi);

    /**
     * This signal is emitted when the time until the battery is full
     * has changed.
     *
     * @param time the new remaining time
     * @param udi the UDI of the battery with the new remaining time
     * @since 5.0
     */
    void timeToFullChanged(qlonglong time, const QString &udi);

    /**
     * This signal is emitted when the energy value of this
     * battery has changed.
     *
     * @param energy the new energy value of the battery
     * @param udi the UDI of the battery with the new energy value
     */
    void energyChanged(double energy, const QString &udi);

    /**
     * This signal is emitted when the energy full value of this
     * battery has changed.
     *
     * @param energy the new energy full value of the battery
     * @param udi the UDI of the battery with the new energy full value
     */
    void energyFullChanged(double energy, const QString &udi);

    /**
     * This signal is emitted when the energy full design value of this
     * battery has changed.
     *
     * @param energy the new energy full design value of the battery
     * @param udi the UDI of the battery with the new energy full design value
     */
    void energyFullDesignChanged(double energy, const QString &udi);

    /**
     * This signal is emitted when the energy rate value of this
     * battery has changed.
     *
     * If positive, the source is being discharged, if negative it's being charged.
     *
     * @param energyRate the new energy rate value of the battery
     * @param udi the UDI of the battery with the new charge percent
     */
    void energyRateChanged(double energyRate, const QString &udi);

    /**
     * This signal is emitted when the voltage in the cell has changed.
     *
     * @param voltage the new voltage of the cell
     * @param udi the UDI of the battery with the new voltage
     * @since 5.0
     */
    void voltageChanged(double voltage, const QString &udi);

    /**
     * This signal is emitted when the battery temperature has changed.
     *
     * @param temperature the new temperature of the battery in degrees Celsius
     * @param udi the UDI of the battery with the new temperature
     * @since 5.0
     */
    void temperatureChanged(double temperature, const QString &udi);

    /**
     * This signal is emitted when the estimated battery remaining time changes.
     *
     * @param time the new remaining time
     * @param udi the UDI of the battery with the new remaining time
     * @since 5.8
     */
    void remainingTimeChanged(qlonglong time, const QString &udi);
};
}

#endif
