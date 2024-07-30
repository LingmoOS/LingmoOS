/*
 *   SPDX-FileCopyrightText: 2008-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "converter.h"

#include "acceleration_p.h"
#include "angle_p.h"
#include "area_p.h"
#include "binary_data_p.h"
#include "currency_p.h"
#include "density_p.h"
#include "electrical_current_p.h"
#include "electrical_resistance_p.h"
#include "energy_p.h"
#include "weight_per_area_p.h"
#include "force_p.h"
#include "frequency_p.h"
#include "fuel_efficiency_p.h"
#include "length_p.h"
#include "mass_p.h"
#include "permeability_p.h"
#include "power_p.h"
#include "pressure_p.h"
#include "temperature_p.h"
#include "thermal_conductivity_p.h"
#include "thermal_flux_p.h"
#include "thermal_generation_p.h"
#include "timeunit_p.h"
#include "unit.h"
#include "velocity_p.h"
#include "voltage_p.h"
#include "volume_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
class ConverterPrivate : public QSharedData
{
public:
    ConverterPrivate()
    {
        m_categories[LengthCategory] = Length::makeCategory();
        m_categories[AreaCategory] = Area::makeCategory();
        m_categories[VolumeCategory] = Volume::makeCategory();
        m_categories[TemperatureCategory] = Temperature::makeCategory();
        m_categories[VelocityCategory] = Velocity::makeCategory();
        m_categories[MassCategory] = Mass::makeCategory();
        m_categories[PressureCategory] = Pressure::makeCategory();
        m_categories[EnergyCategory] = Energy::makeCategory();
        m_categories[CurrencyCategory] = Currency::makeCategory();
        m_categories[PowerCategory] = Power::makeCategory();
        m_categories[TimeCategory] = Time::makeCategory();
        m_categories[FuelEfficiencyCategory] = FuelEfficiency::makeCategory();
        m_categories[DensityCategory] = Density::makeCategory();
        m_categories[WeightPerAreaCategory] = WeightPerArea::makeCategory();
        m_categories[AccelerationCategory] = Acceleration::makeCategory();
        m_categories[ForceCategory] = Force::makeCategory();
        m_categories[AngleCategory] = Angle::makeCategory();
        m_categories[FrequencyCategory] = Frequency::makeCategory();
        m_categories[ThermalConductivityCategory] = ThermalConductivity::makeCategory();
        m_categories[ThermalFluxCategory] = ThermalFlux::makeCategory();
        m_categories[ThermalGenerationCategory] = ThermalGeneration::makeCategory();
        m_categories[VoltageCategory] = Voltage::makeCategory();
        m_categories[ElectricalCurrentCategory] = ElectricalCurrent::makeCategory();
        m_categories[ElectricalResistanceCategory] = ElectricalResistance::makeCategory();
        m_categories[PermeabilityCategory] = Permeability::makeCategory();
        m_categories[BinaryDataCategory] = BinaryData::makeCategory();
    }

    QMap<CategoryId, UnitCategory> m_categories;
};

class QConverterSingleton
{
public:
    QConverterSingleton()
        : d(new ConverterPrivate())
    {
    }
    QExplicitlySharedDataPointer<ConverterPrivate> d;
};

Q_GLOBAL_STATIC(QConverterSingleton, global_converter)

Converter::Converter()
    : d(global_converter->d)
{
}

Converter::~Converter()
{
}

Converter::Converter(const Converter &other)
    : d(other.d)
{
}

Converter &Converter::operator=(const Converter &other)
{
    d = other.d;
    return *this;
}

Converter &Converter::operator=(Converter &&other)
{
    d.swap(other.d);
    return *this;
}

Value Converter::convert(const Value &value, const QString &toUnit) const
{
    if (d && value.unit().isValid()) {
        return value.unit().category().convert(value, toUnit);
    }
    return Value();
}

Value Converter::convert(const Value &value, UnitId toUnit) const
{
    if (d && value.unit().isValid()) {
        return value.unit().category().convert(value, toUnit);
    }
    return Value();
}

Value Converter::convert(const Value &value, const Unit &toUnit) const
{
    if (d && toUnit.isValid() && value.unit().isValid()) {
        return value.unit().category().convert(value, toUnit);
    }
    return Value();
}

UnitCategory Converter::categoryForUnit(const QString &unit) const
{
    const auto lstCategories = categories();
    for (const UnitCategory &u : lstCategories) {
        if (u.hasUnit(unit)) {
            return u;
        }
    }
    return UnitCategory();
}

Unit Converter::unit(const QString &unitString) const
{
    const auto lstCategories = categories();
    for (const UnitCategory &u : lstCategories) {
        Unit unitClass = u.unit(unitString);
        if (unitClass.isValid()) {
            return unitClass;
        }
    }
    return Unit();
}

Unit Converter::unit(UnitId unitId) const
{
    const auto lstCategories = categories();
    for (const UnitCategory &u : lstCategories) {
        Unit unitClass = u.unit(unitId);
        if (unitClass.isValid()) {
            return unitClass;
        }
    }
    return Unit();
}

UnitCategory Converter::category(const QString &category) const
{
    const auto lstCategories = categories();
    for (const UnitCategory &u : lstCategories) {
        if (u.name() == category) {
            return u;
        }
    }
    // not found
    return UnitCategory();
}

UnitCategory Converter::category(CategoryId categoryId) const
{
    if (d && d->m_categories.contains(categoryId)) {
        return d->m_categories[categoryId];
    }
    // not found
    return UnitCategory();
}

QList<UnitCategory> Converter::categories() const
{
    if (d) {
        return d->m_categories.values();
    }
    return QList<UnitCategory>();
}

}
