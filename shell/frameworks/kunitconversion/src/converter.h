/*
 *   SPDX-FileCopyrightText: 2008-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KUNITCONVERSION_CONVERTER_H
#define KUNITCONVERSION_CONVERTER_H

#include <kunitconversion/kunitconversion_export.h>

#include "unitcategory.h"

#include <QExplicitlySharedDataPointer>

namespace KUnitConversion
{
class Value;
class UnitCategory;
class ConverterPrivate;

/**
 * @short Class for converting values between units of measurement
 *
 * This is a class to convert values between different units of measurement.
 *
 * @see Unit, UnitCategory, Value
 *
 * @author Petri Damstén <damu@iki.fi>
 * @author John Layt <jlayt@kde.org>
 */

class KUNITCONVERSION_EXPORT Converter
{
public:
    /**
     * Creates a Converter instance.
     */
    Converter();
    /**
     * Destroys this Converter instance.
     */
    ~Converter();
    /**
     * Copy constructor.
     * @param other existing Converter instance.
     */
    Converter(const Converter &other);

    /**
     * Assignment operator, assign @p other to this.
     **/
    Converter &operator=(const Converter &other);

    /**
     * Move-assigns @p other to this Converter instance, transferring the
     * ownership of the managed pointer to this instance.
     **/
    Converter &operator=(Converter &&other);

    /**
     * Convert value to another unit.
     *
     * @param value value to convert
     * @param toUnit unit to convert to. If empty default unit is used.
     * @return converted value
     **/
    Value convert(const Value &value, const QString &toUnit = QString()) const;
    Value convert(const Value &value, UnitId toUnit) const;
    Value convert(const Value &value, const Unit &toUnit) const;

    /**
     * Find unit category for unit.
     *
     * @param unit unit to find category for.
     * @return unit category for unit
     **/
    UnitCategory categoryForUnit(const QString &unit) const;

    /**
     * Find unit for string unit.
     *
     * @param unitString unit string to find unit for.
     * @return unit for string unit
     **/
    Unit unit(const QString &unitString) const;

    /**
     * Find unit for unit enum.
     *
     * @param unitId unit enum to find unit for.
     * @return unit for string unit
     **/
    Unit unit(UnitId unitId) const;

    /**
     * Find unit category.
     *
     * @param category name of the category to find (length, area, mass, etc.).
     * @return unit category named category or invalid category.
     **/
    UnitCategory category(const QString &category) const;

    /**
     * Find unit category.
     *
     * @param categoryId id of the category to find (LengthCategory, AreaCategory, etc.).
     * @return unit category which id is categoryId or invalid category.
     **/
    UnitCategory category(CategoryId categoryId) const;

    /**
     * Returns a list of all unit categories.
     *
     * @return list of unit categories.
     **/
    QList<UnitCategory> categories() const;

private:
    QExplicitlySharedDataPointer<ConverterPrivate> d;
};

} // KUnitConversion namespace

#endif
