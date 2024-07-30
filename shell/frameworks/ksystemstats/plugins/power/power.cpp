/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "power.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/Battery>

#include <systemstats/AggregateSensor.h>
#include <systemstats/SensorContainer.h>
#include <systemstats/SensorObject.h>
#include <systemstats/SensorProperty.h>

K_PLUGIN_CLASS_WITH_JSON(PowerPlugin, "metadata.json")

QString idHelper(const Solid::Device battery)
{
    const QString serial = battery.as<Solid::Battery>()->serial();
    if (!serial.isEmpty()) {
        return serial;
    }
    return battery.udi().mid(battery.udi().lastIndexOf(QLatin1Char('/')) + 1);
}

class Battery : public KSysGuard::SensorObject {
public:
    Battery(const Solid::Device &device, const QString &name, KSysGuard::SensorContainer *parent);
};

Battery::Battery(const Solid::Device &device, const QString &name, KSysGuard::SensorContainer *parent)
    : SensorObject(idHelper(device), name, parent)
{
    auto n = new KSysGuard::SensorProperty("name", i18nc("@title", "Name"), name, this);
    n->setVariantType(QVariant::String);

    const auto * const battery = device.as<Solid::Battery>();

    auto designCapacity = new KSysGuard::SensorProperty("design", i18nc("@title", "Design Capacity"), battery->energyFullDesign(), this);
    designCapacity->setShortName(i18nc("@title", "Design Capacity"));
    designCapacity->setPrefix(name);
    designCapacity->setDescription(i18n("Amount of energy that the Battery was designed to hold"));
    designCapacity->setUnit(KSysGuard::UnitWattHour);
    designCapacity->setVariantType(QVariant::Double);;
    designCapacity->setMin(battery->energyFullDesign());
    designCapacity->setMax(battery->energyFullDesign());

    auto currentCapacity = new KSysGuard::SensorProperty("capacity", i18nc("@title", "Current Capacity"), battery->energyFull(), this);
    currentCapacity->setShortName(i18nc("@title", "Current Capacity"));
    currentCapacity->setPrefix(name);
    currentCapacity->setDescription(i18n("Amount of energy that the battery can currently hold"));
    currentCapacity->setUnit(KSysGuard::UnitWattHour);
    currentCapacity->setVariantType(QVariant::Double);
    currentCapacity->setMax(designCapacity);
    connect(battery, &Solid::Battery::energyFullChanged, currentCapacity, &KSysGuard::SensorProperty::setValue);

    auto health = new KSysGuard::SensorProperty("health", i18nc("@title", "Health"), battery->capacity(), this);
    health->setShortName(i18nc("@title", "Health"));
    health->setPrefix(name);
    health->setDescription(i18n("Percentage of the design capacity that the battery can hold"));
    health->setUnit(KSysGuard::UnitPercent);
    health->setVariantType(QVariant::Int);
    health->setMax(100);
    connect(battery, &Solid::Battery::capacityChanged, health, &KSysGuard::SensorProperty::setValue);

    auto charge = new KSysGuard::SensorProperty("charge", i18nc("@title", "Charge"), battery->energy(), this);
    charge->setShortName(i18nc("@title", "Current Capacity"));
    charge->setPrefix(name);
    charge->setDescription(i18n("Amount of energy that the battery is currently holding"));
    charge->setUnit(KSysGuard::UnitWattHour);
    charge->setVariantType(QVariant::Double);
    charge->setMax(currentCapacity);
    connect(battery, &Solid::Battery::energyChanged, charge, &KSysGuard::SensorProperty::setValue);

    auto chargePercent = new KSysGuard::SensorProperty("chargePercentage", i18nc("@title", "Charge Percentage"), battery->chargePercent(), this);
    chargePercent->setShortName(i18nc("@title", "Charge Percentage"));
    chargePercent->setPrefix(name);
    chargePercent->setDescription(i18n("Percentage of the current capacity that the battery is currently holding"));
    chargePercent->setUnit(KSysGuard::UnitPercent);
    chargePercent->setVariantType(QVariant::Int);
    chargePercent->setMax(100);
    connect(battery, &Solid::Battery::chargePercentChanged, chargePercent, &KSysGuard::SensorProperty::setValue);

    // Solid reports negative of charging and positive for discharging
    auto chargeRate = new KSysGuard::SensorProperty("chargeRate", i18nc("@title", "Charging Rate"), 0, this);
    chargeRate->setShortName(i18nc("@title", "Charging  Rate"));
    chargeRate->setPrefix(name);
    chargeRate->setDescription(i18n("Power that the battery is being charged with (positive) or discharged (negative)"));
    chargeRate->setUnit(KSysGuard::UnitWatt);
    chargeRate->setVariantType(QVariant::Double);
    chargeRate->setValue(-battery->energyRate());
    connect(battery, &Solid::Battery::energyRateChanged, chargeRate, [chargeRate] (double rate) {
        chargeRate->setValue(-rate);
    });

}

PowerPlugin::PowerPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    m_container = new KSysGuard::SensorContainer("power", i18nc("@title", "Power"), this);
    const auto batteries = Solid::Device::listFromType(Solid::DeviceInterface::Battery);

    for (const auto &device : batteries) {
        auto battery = new Battery(device, device.displayName(), m_container);
        m_batteriesByUdi.insert(device.udi(), battery);
    }

    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, [this] (const QString &udi) {
        const Solid::Device device(udi);
        if (device.isDeviceInterface(Solid::DeviceInterface::Battery)) {
            auto battery = new Battery(device, device.displayName(), m_container);
            m_batteriesByUdi.insert(udi, battery);
        }
    });
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, [this] (const QString &udi) {
        if (m_batteriesByUdi.contains(udi)) {
            m_container->removeObject(m_batteriesByUdi[udi]);
            m_batteriesByUdi.remove(udi);
        }
    });
}

#include "power.moc"

#include "moc_power.cpp"
