/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_PREDICATE_H
#define SOLID_PREDICATE_H

#include <QSet>
#include <QVariant>

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
class Device;

/**
 * @class Solid::Predicate predicate.h <Solid/Predicate>
 *
 * This class implements predicates for devices.
 *
 * A predicate is a logical condition that a given device can match or not.
 * It's a constraint about the value a property must have in a given device
 * interface, or any combination (conjunction, disjunction) of such
 * constraints.
 *
 * A predicate can be:
 * - a single comparison, or
 * - a conjunction ("AND") of exactly two predicates, or
 * - a disjunction ("OR") of exactly two predicates.
 *
 * Since these can be nested, it is possible to express "a StorageVolume
 * that is not ignored AND that StorageVolume contains a FileSystem
 * AND that StorageVolume is removable". Since conjunctions use exactly
 * two predicates (and this expression has three), square brackets are
 * used to group the nested predicates when writing them out in full:
 *
 * ```
 *   [ [ StorageVolume.ignored == false AND StorageVolume.usage == 'FileSystem' ]
 *     AND StorageVolume.removable == true ]
 * ```
 *
 * Predicates can be constructed programmatically by creating single
 * comparisons with the Predicate constructor, and then building
 * conjunctions with `operator&` and disjunctions with `operator|`.
 *
 * Predicates can be constructed from a string by calling `fromString()`
 * which parses the given string and returns a predicate. If there
 * are any errors in parsing the string, an empty predicate is returned;
 * use `isValid()` to detect whether that is the case.
 *
 * The string language is described exactly in `predicate_parser.y`,
 * but boils down to:
 *
 * - a single comparison is written as `<interface>.<property> == <value>`
 * - a single bitmask check is written as `<interface>.<property> & <value>`
 * - a conjunction is written as `[ <predicate> AND <predicate> ]`
 * - a disjunction is written as `[ <predicate> OR <predicate> ]`
 *
 * Note the mandatory use of `[` and `]` around conjunctions and disjunctions.
 */
class SOLID_EXPORT Predicate
{
public:
    /**
     * The comparison operator which can be used for matching within the predicate.
     *
     * - Equals, the property and the value will match for strict equality
     * - Mask, the property and the value will match if the bitmasking is not null
     */
    enum ComparisonOperator { Equals, Mask };

    /**
     * The predicate type which controls how the predicate is handled
     *
     * - PropertyCheck, the predicate contains a comparison that needs to be matched using a ComparisonOperator
     * - Conjunction, the two contained predicates need to be true for this predicate to be true
     * - Disjunction, either of the two contained predicates may be true for this predicate to be true
     * - InterfaceCheck, the device type is compared
     */
    enum Type { PropertyCheck, Conjunction, Disjunction, InterfaceCheck };

    /**
     * Constructs an invalid predicate.
     */
    Predicate();

    /**
     * Copy constructor.
     *
     * @param other the predicate to copy
     */
    Predicate(const Predicate &other);

    /**
     * Constructs a predicate matching the value of a property in
     * a given device interface.
     *
     * @param ifaceType the device interface type the device must have
     * @param property the property name of the device interface
     * @param value the value the property must have to make the device match
     * @param compOperator the operator to apply between the property and the value when matching
     */
    Predicate(const DeviceInterface::Type &ifaceType, const QString &property, const QVariant &value, ComparisonOperator compOperator = Equals);

    /**
     * Constructs a predicate matching the value of a property in
     * a given device interface.
     *
     * @param ifaceName the name of the device interface the device must have
     * @param property the property name of the device interface
     * @param value the value the property must have to make the device match
     * @param compOperator the operator to apply between the property and the value when matching
     */
    Predicate(const QString &ifaceName, const QString &property, const QVariant &value, ComparisonOperator compOperator = Equals);

    /**
     * Constructs a predicate matching devices being of a particular device interface
     *
     * @param ifaceType the device interface the device must have
     */
    explicit Predicate(const DeviceInterface::Type &ifaceType);

    /**
     * Constructs a predicate matching devices being of a particular device interface
     *
     * @param ifaceName the name of the device interface the device must have
     */
    explicit Predicate(const QString &ifaceName);

    /**
     * Destroys a Predicate object.
     */
    ~Predicate();

    /**
     * Assignment operator.
     *
     * @param other the predicate to assign
     * @return this predicate after having assigned 'other' to it
     */
    Predicate &operator=(const Predicate &other);

    /**
     * 'And' operator.
     *
     * @param other the second operand
     * @return a new 'and' predicate having 'this' and 'other' as operands
     */
    Predicate operator&(const Predicate &other);

    /**
     * 'AndEquals' operator.
     *
     * @param other the second operand
     * @return assigns to 'this' a new 'and' predicate having 'this' and 'other' as operands
     */
    Predicate &operator&=(const Predicate &other);

    /**
     * 'Or' operator.
     *
     * @param other the second operand
     * @return a new 'or' predicate having 'this' and 'other' as operands
     */
    Predicate operator|(const Predicate &other);

    /**
     * 'OrEquals' operator.
     *
     * @param other the second operand
     * @return assigns to 'this' a new 'or' predicate having 'this' and 'other' as operands
     */
    Predicate &operator|=(const Predicate &other);

    /**
     * Indicates if the predicate is valid.
     *
     * Predicate() is the only invalid predicate.
     *
     * @return true if the predicate is valid, false otherwise
     */
    bool isValid() const;

    /**
     * Checks if a device matches the predicate.
     *
     * @param device the device to match against the predicate
     * @return true if the given device matches the predicate, false otherwise
     */
    bool matches(const Device &device) const;

    /**
     * Retrieves the device interface types used in this predicate.
     *
     * @return all the device interface types used in this predicate
     */
    QSet<DeviceInterface::Type> usedTypes() const;

    /**
     * Converts the predicate to its string form.
     *
     * @return a string representation of the predicate
     */
    QString toString() const;

    /**
     * Converts a string to a predicate.
     *
     * @param predicate the string to convert
     * @return a new valid predicate if the given string is syntactically
     * correct, Predicate() otherwise
     */
    static Predicate fromString(const QString &predicate);

    /**
     * Retrieves the predicate type, used to determine how to handle the predicate
     *
     * @since 4.4
     * @return the predicate type
     */
    Type type() const;

    /**
     * Retrieves the interface type.
     *
     * @note This is only valid for InterfaceCheck and PropertyCheck types
     * @since 4.4
     * @return a device interface type used by the predicate
     */
    DeviceInterface::Type interfaceType() const;

    /**
     * Retrieves the property name used when retrieving the value to compare against
     *
     * @note This is only valid for the PropertyCheck type
     * @since 4.4
     * @return a property name
     */
    QString propertyName() const;

    /**
     * Retrieves the value used when comparing a devices property to see if it matches the predicate
     *
     * @note This is only valid for the PropertyCheck type
     * @since 4.4
     * @return the value used
     */
    QVariant matchingValue() const;

    /**
     * Retrieves the comparison operator used to compare a property's value
     *
     * @since 4.4
     * @note This is only valid for Conjunction and Disjunction types
     * @return the comparison operator used
     */
    ComparisonOperator comparisonOperator() const;

    /**
     * A smaller, inner predicate which is the first to appear and is compared with the second one
     *
     * @since 4.4
     * @note This is only valid for Conjunction and Disjunction types
     * @return The predicate used for the first operand
     */
    Predicate firstOperand() const;

    /**
     * A smaller, inner predicate which is the second to appear and is compared with the first one
     *
     * @since 4.4
     * @note This is only valid for Conjunction and Disjunction types
     * @return The predicate used for the second operand
     */
    Predicate secondOperand() const;

private:
    class Private;
    Private *const d;
};
}

#endif
