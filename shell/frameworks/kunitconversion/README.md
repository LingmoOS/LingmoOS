# KUnitConversion

Converting physical units

## Introduction

KUnitConversion provides functions to convert values in different physical
units. It supports converting different prefixes (e.g. kilo, mega, giga) as
well as converting between different unit systems (e.g. liters, gallons). The
following areas are supported:

* Acceleration
* Angle
* Area
* Binary Data
* Currency
* Density
* Electrical Current
* Electrical Resistance
* Energy
* Force
* Frequency
* Fuel Efficiency
* Length
* Mass
* Permeability
* Power
* Pressure
* Temperature
* Thermal Conductivity
* Thermal Flux
* Thermal Generation
* Time
* Velocity
* Volume
* Voltage
* Weight per area

## Usage

To convert 100 GBP into EUR, you can write:

    using namespace KUnitConversion;
    Value pounds(100, Gbp);
    Value eur = pounds.convertTo(Eur);

