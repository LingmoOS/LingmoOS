/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_ADDRESSFORMAT_H
#define KCONTACTS_ADDRESSFORMAT_H

#include "kcontacts_export.h"
#include "namespace.h"

#include <QExplicitlySharedDataPointer>

namespace KContacts
{

class Address;
class AddressFormatElementPrivate;

/** A single element in an address format.
 *
 *  A format element can be one of three types:
 *  - a field from the address data
 *  - a literal string
 *  - a separator
 *
 *  @since 5.92
 *  @see KContacts::AddressFormat
 */
class KCONTACTS_EXPORT AddressFormatElement
{
    Q_GADGET
    Q_PROPERTY(bool isField READ isField)
    Q_PROPERTY(KContacts::AddressFormatField field READ field)
    Q_PROPERTY(bool isLiteral READ isLiteral)
    Q_PROPERTY(QString literal READ literal)
    Q_PROPERTY(bool isSeparator READ isSeparator)

public:
    explicit AddressFormatElement();
    AddressFormatElement(const AddressFormatElement &);
    ~AddressFormatElement();
    AddressFormatElement &operator=(const AddressFormatElement &);

    bool isField() const;
    AddressFormatField field() const;

    bool isLiteral() const;
    QString literal() const;

    bool isSeparator() const;

private:
    friend class AddressFormatElementPrivate;
    QExplicitlySharedDataPointer<AddressFormatElementPrivate> d;
};

class AddressFormatPrivate;

/** Information on how addresses are formatted in a specific country/language.
 *  This is primarily used for displaying or printing addresses, but is also
 *  useful for country specific adjustment in address edit forms, or for parsing
 *  textual addresses.
 *
 *  @since 5.92
 *  @see AddressFormatRepository
 */
class KCONTACTS_EXPORT AddressFormat
{
    Q_GADGET
    Q_PROPERTY(QString country READ country)
    Q_PROPERTY(QList<KContacts::AddressFormatElement> elements READ elementsForQml)
    Q_PROPERTY(KContacts::AddressFormatFields requiredFields READ requiredFields)
    Q_PROPERTY(KContacts::AddressFormatFields usedFields READ usedFields)
    Q_PROPERTY(KContacts::AddressFormatFields upperCaseFields READ upperCaseFields)
    Q_PROPERTY(QString postalCodeRegularExpression READ postalCodeRegularExpression)

public:
    AddressFormat();
    AddressFormat(const AddressFormat &);
    ~AddressFormat();
    AddressFormat &operator=(const AddressFormat &);

    /** ISO 3166-1 alpha2 code of the country this format is for. */
    QString country() const;

    /** A sequence of field/literal/separator elements for this address format. */
    const std::vector<AddressFormatElement> &elements() const;

    /** The address fields that are required by this format for a valid address.
     *  @note This information is not available for all formats.
     */
    AddressFormatFields requiredFields() const;

    /** The address fields that are used by this format.
     *  This is a superset of requiredFields(), and this information is
     *  available for all formats.
     */
    AddressFormatFields usedFields() const;

    /** Fields that should be printed in upper case regardless
     *  of the input casing.
     */
    AddressFormatFields upperCaseFields() const;

    /** Regular expression matching the postal codes of this format. */
    QString postalCodeRegularExpression() const;

private:
    KCONTACTS_NO_EXPORT QList<AddressFormatElement> elementsForQml() const;

    friend class AddressFormatPrivate;
    QExplicitlySharedDataPointer<AddressFormatPrivate> d;
};

/** Provides address format information for a given country.
 *
 *  @since 5.92
 */
class KCONTACTS_EXPORT AddressFormatRepository
{
    Q_GADGET
public:
    /** Look up format data for a country.
     *  @param countryCode ISO 3166-1 alpha 2 country code.
     */
    static Q_INVOKABLE KContacts::AddressFormat formatForCountry(const QString &countryCode,
                                                                 KContacts::AddressFormatScriptPreference scriptPref,
                                                                 KContacts::AddressFormatPreference formatPref = AddressFormatPreference::Generic);

    /** Look up format data for a given address.
     *  The preferred script is determined from the script used in the address object.
     *  If the address object has no country information set, the local country is assumed.
     */
    static KContacts::AddressFormat formatForAddress(const Address &address, AddressFormatPreference formatPref = AddressFormatPreference::Generic);
};

}

#endif // KCONTACTS_ADDRESSFORMAT_H
