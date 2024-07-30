/*
 *   SPDX-FileCopyrightText: 2007-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KUNITCONVERSION_UNIT_H
#define KUNITCONVERSION_UNIT_H

#include "kunitconversion/kunitconversion_export.h"

#include <QExplicitlySharedDataPointer>
#include <QString>

namespace KUnitConversion
{
enum CategoryId {
    InvalidCategory = -1,
    LengthCategory,
    AreaCategory,
    VolumeCategory,
    TemperatureCategory,
    VelocityCategory,
    MassCategory,
    PressureCategory,
    EnergyCategory,
    CurrencyCategory,
    PowerCategory,
    TimeCategory,
    FuelEfficiencyCategory,
    DensityCategory,
    WeightPerAreaCategory,
    AccelerationCategory,
    AngleCategory,
    FrequencyCategory,
    ForceCategory,
    /** @since 5.27 */
    ThermalConductivityCategory,
    /** @since 5.27 */
    ThermalFluxCategory,
    /** @since 5.27 */
    ThermalGenerationCategory,
    /** @since 5.27 */
    VoltageCategory,
    /** @since 5.27 */
    ElectricalCurrentCategory,
    /** @since 5.27 */
    ElectricalResistanceCategory,
    /** @since 5.53 */
    PermeabilityCategory,
    /** @since 5.61 **/
    BinaryDataCategory
};

enum UnitId {
    InvalidUnit = -1,
    NoUnit = 0,
    Percent = 1,

    // Area
    SquareYottameter = 1000,
    SquareZettameter,
    SquareExameter,
    SquarePetameter,
    SquareTerameter,
    SquareGigameter,
    SquareMegameter,
    SquareKilometer,
    SquareHectometer,
    SquareDecameter,
    SquareMeter,
    SquareDecimeter,
    SquareCentimeter,
    SquareMillimeter,
    SquareMicrometer,
    SquareNanometer,
    SquarePicometer,
    SquareFemtometer,
    SquareAttometer,
    SquareZeptometer,
    SquareYoctometer,
    Acre,
    SquareFoot,
    SquareInch,
    SquareMile,

    // Length
    Yottameter = 2000,
    Zettameter,
    Exameter,
    Petameter,
    Terameter,
    Gigameter,
    Megameter,
    Kilometer,
    Hectometer,
    Decameter,
    Meter,
    Decimeter,
    Centimeter,
    Millimeter,
    Micrometer,
    Nanometer,
    Picometer,
    Femtometer,
    Attometer,
    Zeptometer,
    Yoctometer,
    Inch,
    Foot,
    Yard,
    Mile,
    NauticalMile,
    LightYear,
    Parsec,
    AstronomicalUnit,
    Thou,
    Angstrom,

    // Volume
    CubicYottameter = 3000,
    CubicZettameter,
    CubicExameter,
    CubicPetameter,
    CubicTerameter,
    CubicGigameter,
    CubicMegameter,
    CubicKilometer,
    CubicHectometer,
    CubicDecameter,
    CubicMeter,
    CubicDecimeter,
    CubicCentimeter,
    CubicMillimeter,
    CubicMicrometer,
    CubicNanometer,
    CubicPicometer,
    CubicFemtometer,
    CubicAttometer,
    CubicZeptometer,
    CubicYoctometer,
    Yottaliter,
    Zettaliter,
    Exaliter,
    Petaliter,
    Teraliter,
    Gigaliter,
    Megaliter,
    Kiloliter,
    Hectoliter,
    Decaliter,
    Liter,
    Deciliter,
    Centiliter,
    Milliliter,
    Microliter,
    Nanoliter,
    Picoliter,
    Femtoliter,
    Attoliter,
    Zeptoliter,
    Yoctoliter,
    CubicFoot,
    CubicInch,
    CubicMile,
    FluidOunce,
    Cup,
    Teaspoon,
    Tablespoon,
    GallonUS,
    PintImperial,
    /** @since 5.53 */
    OilBarrel,
    /** @since 5.70 */
    GallonImperial,
    PintUS,

    // Mass
    Yottagram = 4000,
    Zettagram,
    Exagram,
    Petagram,
    Teragram,
    Gigagram,
    Megagram,
    Kilogram,
    Hectogram,
    Decagram,
    Gram,
    Decigram,
    Centigram,
    Milligram,
    Microgram,
    Nanogram,
    Picogram,
    Femtogram,
    Attogram,
    Zeptogram,
    Yoctogram,
    Ton,
    Carat,
    Pound,
    Ounce,
    TroyOunce,
    MassNewton,
    Kilonewton,

    /** @since 5.26 */
    Stone,

    // Pressure
    Yottapascal = 5000,
    Zettapascal,
    Exapascal,
    Petapascal,
    Terapascal,
    Gigapascal,
    Megapascal,
    Kilopascal,
    Hectopascal,
    Decapascal,
    Pascal,
    Decipascal,
    Centipascal,
    Millipascal,
    Micropascal,
    Nanopascal,
    Picopascal,
    Femtopascal,
    Attopascal,
    Zeptopascal,
    Yoctopascal,
    Bar,
    Millibar,
    Decibar,
    Torr,
    TechnicalAtmosphere,
    Atmosphere,
    PoundForcePerSquareInch,
    InchesOfMercury,
    MillimetersOfMercury,

    // Temperature
    Kelvin = 6000,
    Celsius,
    Fahrenheit,
    Rankine,
    Delisle,
    TemperatureNewton,
    Reaumur,
    Romer,

    // Energy
    Yottajoule = 7000,
    Zettajoule,
    Exajoule,
    Petajoule,
    Terajoule,
    Gigajoule,
    Megajoule,
    Kilojoule,
    Hectojoule,
    Decajoule,
    Joule,
    Decijoule,
    Centijoule,
    Millijoule,
    Microjoule,
    Nanojoule,
    Picojoule,
    Femtojoule,
    Attojoule,
    Zeptojoule,
    Yoctojoule,
    GuidelineDailyAmount,
    Electronvolt,
    Rydberg,
    Kilocalorie,
    PhotonWavelength,
    KiloJoulePerMole,
    JoulePerMole,
    /** @since 5.27 */
    Btu,
    /** @since 5.27 */
    Erg,

    // Currency
    Eur = 8000,
    Ats,
    Bef,
    Nlg,
    Fim,
    Frf,
    Dem,
    Iep,
    Itl,
    Luf,
    Pte,
    Esp,
    Grd,
    Sit,
    Cyp,
    Mtl,
    Skk,
    Usd,
    Jpy,
    Bgn,
    Czk,
    Dkk,
    Eek,
    Gbp,
    Huf,
    Ltl,
    Lvl,
    Pln,
    Ron,
    Sek,
    Chf,
    Nok,
    Hrk,
    Rub,
    Try,
    Aud,
    Brl,
    Cad,
    Cny,
    Hkd,
    Idr,
    Inr,
    Krw,
    Mxn,
    Myr,
    Nzd,
    Php,
    Sgd,
    Thb,
    Zar,
    Ils,
    Isk,

    // Velocity
    MeterPerSecond = 9000,
    KilometerPerHour,
    MilePerHour,
    FootPerSecond,
    InchPerSecond,
    Knot,
    Mach,
    SpeedOfLight,
    Beaufort,

    // Power
    Yottawatt = 10000,
    Zettawatt,
    Exawatt,
    Petawatt,
    Terawatt,
    Gigawatt,
    Megawatt,
    Kilowatt,
    Hectowatt,
    Decawatt,
    Watt,
    Deciwatt,
    Centiwatt,
    Milliwatt,
    Microwatt,
    Nanowatt,
    Picowatt,
    Femtowatt,
    Attowatt,
    Zeptowatt,
    Yoctowatt,
    Horsepower,
    /** @since 5.62 */
    DecibelKilowatt,
    DecibelWatt,
    DecibelMilliwatt,
    DecibelMicrowatt,

    // Time
    Yottasecond = 11000,
    Zettasecond,
    Exasecond,
    Petasecond,
    Terasecond,
    Gigasecond,
    Megasecond,
    Kilosecond,
    Hectosecond,
    Decasecond,
    Second,
    Decisecond,
    Centisecond,
    Millisecond,
    Microsecond,
    Nanosecond,
    Picosecond,
    Femtosecond,
    Attosecond,
    Zeptosecond,
    Yoctosecond,
    Minute,
    Hour,
    Day,
    Week,
    JulianYear,
    LeapYear,
    Year,

    // FuelEfficiency
    LitersPer100Kilometers = 12000,
    MilePerUsGallon,
    MilePerImperialGallon,
    KilometrePerLitre,

    // Density
    YottakilogramsPerCubicMeter = 13000,
    ZettakilogramPerCubicMeter,
    ExakilogramPerCubicMeter,
    PetakilogramPerCubicMeter,
    TerakilogramPerCubicMeter,
    GigakilogramPerCubicMeter,
    MegakilogramPerCubicMeter,
    KilokilogramPerCubicMeter,
    HectokilogramsPerCubicMeter,
    DecakilogramsPerCubicMeter,
    KilogramsPerCubicMeter,
    DecikilogramsPerCubicMeter,
    CentikilogramsPerCubicMeter,
    MillikilogramsPerCubicMeter,
    MicrokilogramsPerCubicMeter,
    NanokilogramsPerCubicMeter,
    PicokilogramsPerCubicMeter,
    FemtokilogramsPerCubicMeter,
    AttokilogramsPerCubicMeter,
    ZeptokilogramsPerCubicMeter,
    YoctokilogramsPerCubicMeter,
    KilogramPerLiter,
    GramPerLiter,
    GramPerMilliliter,
    OuncePerCubicInch,
    OuncePerCubicFoot,
    OuncePerCubicYard,
    PoundPerCubicInch,
    PoundPerCubicFoot,
    PoundPerCubicYard,

    // Weight per area
    GramsPerSquareMeter,
    OuncesPerSquareYard,

    // Acceleration
    MetresPerSecondSquared = 14000,
    FeetPerSecondSquared,
    StandardGravity,

    // Force
    Yottanewton = 15000,
    Zettanewton,
    Exanewton,
    Petanewton,
    Teranewton,
    Giganewton,
    Meganewton,
    KilonewtonForce,
    Hectonewton,
    Decanewton,
    Newton,
    Decinewton,
    Centinewton,
    Millinewton,
    Micronewton,
    Nanonewton,
    Piconewton,
    Femtonewton,
    Attonewton,
    Zeptonewton,
    Yoctonewton,
    Dyne,
    Kilopond,
    PoundForce,
    Poundal,

    // Angle
    Degree = 16000,
    Radian,
    Gradian,
    ArcMinute,
    ArcSecond,

    // Frequency
    Yottahertz = 17000,
    Zettahertz,
    Exahertz,
    Petahertz,
    Terahertz,
    Gigahertz,
    Megahertz,
    Kilohertz,
    Hectohertz,
    Decahertz,
    Hertz,
    Decihertz,
    Centihertz,
    Millihertz,
    Microhertz,
    Nanohertz,
    Picohertz,
    Femtohertz,
    Attohertz,
    Zeptohertz,
    Yoctohertz,
    RPM,

    // Thermal Conductivity
    /** @since 5.27 */
    WattPerMeterKelvin = 18000,
    /** @since 5.27 */
    BtuPerFootHourFahrenheit,
    /** @since 5.27 */
    BtuPerSquareFootHourFahrenheitPerInch,

    // Thermal Flux Density
    /** @since 5.27 */
    WattPerSquareMeter = 19000,
    /** @since 5.27 */
    BtuPerHourPerSquareFoot,

    // Thermal Generation per volume
    /** @since 5.27 */
    WattPerCubicMeter = 20000,
    /** @since 5.27 */
    BtuPerHourPerCubicFoot,

    // Voltage
    /** @since 5.27 */
    Yottavolts = 30000,
    /** @since 5.27 */
    Zettavolts,
    /** @since 5.27 */
    Exavolts,
    /** @since 5.27 */
    Petavolts,
    /** @since 5.27 */
    Teravolts,
    /** @since 5.27 */
    Gigavolts,
    /** @since 5.27 */
    Megavolts,
    /** @since 5.27 */
    Kilovolts,
    /** @since 5.27 */
    Hectovolts,
    /** @since 5.27 */
    Decavolts,
    /** @since 5.27 */
    Volts,
    /** @since 5.27 */
    Decivolts,
    /** @since 5.27 */
    Centivolts,
    /** @since 5.27 */
    Millivolts,
    /** @since 5.27 */
    Microvolts,
    /** @since 5.27 */
    Nanovolts,
    /** @since 5.27 */
    Picovolts,
    /** @since 5.27 */
    Femtovolts,
    /** @since 5.27 */
    Attovolts,
    /** @since 5.27 */
    Zeptovolts,
    /** @since 5.27 */
    Yoctovolts,
    /** @since 5.27 */
    Statvolts,

    // Electrical Current
    /** @since 5.27 */
    Yottaampere = 31000,
    /** @since 5.27 */
    Zettaampere,
    /** @since 5.27 */
    Exaampere,
    /** @since 5.27 */
    Petaampere,
    /** @since 5.27 */
    Teraampere,
    /** @since 5.27 */
    Gigaampere,
    /** @since 5.27 */
    Megaampere,
    /** @since 5.27 */
    Kiloampere,
    /** @since 5.27 */
    Hectoampere,
    /** @since 5.27 */
    Decaampere,
    /** @since 5.27 */
    Ampere,
    /** @since 5.27 */
    Deciampere,
    /** @since 5.27 */
    Centiampere,
    /** @since 5.27 */
    Milliampere,
    /** @since 5.27 */
    Microampere,
    /** @since 5.27 */
    Nanoampere,
    /** @since 5.27 */
    Picoampere,
    /** @since 5.27 */
    Femtoampere,
    /** @since 5.27 */
    Attoampere,
    /** @since 5.27 */
    Zeptoampere,
    /** @since 5.27 */
    Yoctoampere,

    // Electrical Resistance
    /** @since 5.27 */
    Yottaohms = 32000,
    /** @since 5.27 */
    Zettaohms,
    /** @since 5.27 */
    Exaohms,
    /** @since 5.27 */
    Petaohms,
    /** @since 5.27 */
    Teraohms,
    /** @since 5.27 */
    Gigaohms,
    /** @since 5.27 */
    Megaohms,
    /** @since 5.27 */
    Kiloohms,
    /** @since 5.27 */
    Hectoohms,
    /** @since 5.27 */
    Decaohms,
    /** @since 5.27 */
    Ohms,
    /** @since 5.27 */
    Deciohms,
    /** @since 5.27 */
    Centiohms,
    /** @since 5.27 */
    Milliohms,
    /** @since 5.27 */
    Microohms,
    /** @since 5.27 */
    Nanoohms,
    /** @since 5.27 */
    Picoohms,
    /** @since 5.27 */
    Femtoohms,
    /** @since 5.27 */
    Attoohms,
    /** @since 5.27 */
    Zeptoohms,
    /** @since 5.27 */
    Yoctoohms,

    /** @since 5.53 */
    Darcy = 33000,
    /** @since 5.53 */
    MiliDarcy,
    /** @since 5.53 */
    PermeabilitySquareMicrometer,

    /** @since 5.61 */
    Yobibyte = 34000,
    /** @since 5.61 */
    Yobibit,
    /** @since 5.61 */
    Yottabyte,
    /** @since 5.61 */
    Yottabit,
    /** @since 5.61 */
    Zebibyte,
    /** @since 5.61 */
    Zebibit,
    /** @since 5.61 */
    Zettabyte,
    /** @since 5.61 */
    Zettabit,
    /** @since 5.61 */
    Exbibyte,
    /** @since 5.61 */
    Exbibit,
    /** @since 5.61 */
    Exabyte,
    /** @since 5.61 */
    Exabit,
    /** @since 5.61 */
    Pebibyte,
    /** @since 5.61 */
    Pebibit,
    /** @since 5.61 */
    Petabyte,
    /** @since 5.61 */
    Petabit,
    /** @since 5.61 */
    Tebibyte,
    /** @since 5.61 */
    Tebibit,
    /** @since 5.61 */
    Terabyte,
    /** @since 5.61 */
    Terabit,
    /** @since 5.61 */
    Gibibyte,
    /** @since 5.61 */
    Gibibit,
    /** @since 5.61 */
    Gigabyte,
    /** @since 5.61 */
    Gigabit,
    /** @since 5.61 */
    Mebibyte,
    /** @since 5.61 */
    Mebibit,
    /** @since 5.61 */
    Megabyte,
    /** @since 5.61 */
    Megabit,
    /** @since 5.61 */
    Kibibyte,
    /** @since 5.61 */
    Kibibit,
    /** @since 5.61 */
    Kilobyte,
    /** @since 5.61 */
    Kilobit,
    /** @since 5.61 */
    Byte,
    /** @since 5.61 */
    Bit
};

class UnitCategory;
class UnitPrivate;

/**
 * @short Class to define a unit of measurement
 *
 * This is a class to define a unit of measurement.
 *
 * @see Converter, UnitCategory, Value
 *
 * @author Petri Damstén <damu@iki.fi>
 * @author John Layt <jlayt@kde.org>
 */

class KUNITCONVERSION_EXPORT Unit
{
public:
    /**
     * Null constructor
     **/
    Unit();

    /**
     * Copy constructor, copy @p other to this.
     **/
    Unit(const Unit &other);

    ~Unit();

    /**
     * Assignment operator, assign @p other to this.
     **/
    Unit &operator=(const Unit &other);

    /**
     * Move-assigns @p other to this Unit instance, transferring the
     * ownership of the managed pointer to this instance.
     **/
    Unit &operator=(Unit &&other);

    /**
     * @return @c true if this Unit is equal to the @p other Unit.
     **/
    bool operator==(const Unit &other) const;

    /**
     * @return @c true if this Unit is not equal to the @p other Unit.
     **/
    bool operator!=(const Unit &other) const;

    /**
     * @return returns true if this Unit is null
     **/
    bool isNull() const;

    /**
     * @return if unit is valid.
     **/
    bool isValid() const;

    /**
     * @return unit id.
     **/
    UnitId id() const;

    /**
     * @return category id.
     **/
    CategoryId categoryId() const;

    /**
     * @return unit category.
     **/
    UnitCategory category() const;

    /**
     * @return translated name for unit.
     **/
    QString description() const;

    /**
     * @return symbol for the unit.
     **/
    QString symbol() const;

    /**
     * @param value number value
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param format type of floating point formatting, like in QString::arg
     * @param precision number of digits after the decimal separator
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return value + unit string
     **/
    QString toString(qreal value, int fieldWidth = 0, char format = 'g', int precision = -1, const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * @param value number value
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param format type of floating point formatting, like in QString::arg
     * @param precision number of digits after the decimal separator
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return value + unit string
     **/
    QString toSymbolString(qreal value, int fieldWidth = 0, char format = 'g', int precision = -1, const QChar &fillChar = QLatin1Char(' ')) const;

protected:
    qreal toDefault(qreal value) const;
    qreal fromDefault(qreal value) const;

private:
    friend class UnitPrivate;
    friend class UnitCategory;
    friend class UnitCategoryPrivate;
    friend class CurrencyCategoryPrivate;

    KUNITCONVERSION_NO_EXPORT explicit Unit(UnitPrivate *dd);

    KUNITCONVERSION_NO_EXPORT void setUnitMultiplier(qreal multiplier);

    QExplicitlySharedDataPointer<UnitPrivate> d;
};

} // KUnitConversion namespace

Q_DECLARE_TYPEINFO(KUnitConversion::Unit, Q_RELOCATABLE_TYPE);

#endif
