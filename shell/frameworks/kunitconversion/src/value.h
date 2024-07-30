/*
 *   SPDX-FileCopyrightText: 2007-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KUNITCONVERSION_VALUE_H
#define KUNITCONVERSION_VALUE_H

#include "kunitconversion/kunitconversion_export.h"

#include "unit.h"

#include <QSharedDataPointer>
#include <QString>

class QVariant;

namespace KUnitConversion
{
class ValuePrivate;

/**
 * @short Class to hold a value in a unit of measurement
 *
 * This is a class to hold a value in a unit of measurement.
 *
 * @see Converter, Unit, UnitCategory
 *
 * @author Petri Damstén <damu@iki.fi>
 * @author John Layt <jlayt@kde.org>
 */

class KUNITCONVERSION_EXPORT Value
{
public:
    /**
     * Creates a null value.
     */
    Value();
    /**
     * Creates a value with a unit instance
     */
    Value(qreal number, const Unit &unit);
    /**
     * Creates a value with a unit (as a string).
     */
    Value(qreal number, const QString &unitString);
    /**
     * Creates a value with a unit (as a enum value).
     */
    Value(qreal number, UnitId unitId);
    /**
     * Creates a value based on a QVariant (calling toReal() on it) with a unit (as a string).
     */
    Value(const QVariant &number, const QString &unitString);

    /**
     * Copy constructor, copy @p other to this.
     **/
    Value(const Value &other);

    /**
     * Destroys this Value instance
     */
    ~Value();

    /**
     * Assignment operator, assign @p other to this.
     **/
    Value &operator=(const Value &other);

#ifdef Q_COMPILER_RVALUE_REFS
    /**
     * Move-assigns @p other to this Value instance, transferring the
     * ownership of the managed pointer to this instance.
     **/
    Value &operator=(Value &&other)
    {
        swap(other);
        return *this;
    }
#endif

    /**
     * Swaps this Value with @p other. This function is very fast and never fails.
     **/
    void swap(Value &other)
    {
        d.swap(other.d);
    }

    /**
     * @return @c true if this Value is equal to the @p other Value.
     **/
    bool operator==(const Value &other) const;

    /**
     * @return @c true if this Value is not equal to the @p other Value.
     **/
    bool operator!=(const Value &other) const;

    /**
     * @return returns true if this Value is null
     **/
    bool isNull() const;

    /**
     * Check if value is valid.
     *
     * @return True if value is valid
     **/
    bool isValid() const;

    /**
     * Number part of the value
     **/
    qreal number() const;

    /**
     * Unit part of the value
     **/
    Unit unit() const;

    /**
     * Convert value to a string
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param format type of floating point formatting, like in QString::arg
     * @param precision number of digits after the decimal separator
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return value as a string
     **/
    QString toString(int fieldWidth = 0, char format = 'g', int precision = -1, const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * Convert value to a string with symbol
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param format type of floating point formatting, like in QString::arg
     * @param precision number of digits after the decimal separator
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return value as a string
     **/
    QString toSymbolString(int fieldWidth = 0, char format = 'g', int precision = -1, const QChar &fillChar = QLatin1Char(' ')) const;

    /**
     * rounds value to decimal count
     * @param decimals decimal count.
     **/
    Value &round(uint decimals);

    /**
     * convert to another unit
     **/
    Value convertTo(const Unit &unit) const;

    /**
     * convert to another unit
     **/
    Value convertTo(UnitId unit) const;

    /**
     * convert to another unit
     **/
    Value convertTo(const QString &unit) const;

private:
    QSharedDataPointer<ValuePrivate> d;
};

} // KUnitConversion namespace

#endif
