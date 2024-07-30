/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_ADDRESS_H
#define KCONTACTS_ADDRESS_H

#include "geo.h"
#include "kcontacts_export.h"
#include "namespace.h"

#include <QList>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>
#include <QUrl>

namespace KContacts
{
class Geo;
/**
  @brief
  Postal address information.

  This class represents information about a postal address.
*/
class KCONTACTS_EXPORT Address
{
    friend KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &s, const Address &addr);
    friend KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &s, Address &addr);

    Q_GADGET
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(bool isEmpty READ isEmpty)
    Q_PROPERTY(Type type READ type WRITE setType)
    Q_PROPERTY(QString typeLabel READ typeLabel)
    Q_PROPERTY(QString postOfficeBox READ postOfficeBox WRITE setPostOfficeBox)
    Q_PROPERTY(QString extended READ extended WRITE setExtended)
    Q_PROPERTY(QString street READ street WRITE setStreet)
    Q_PROPERTY(QString locality READ locality WRITE setLocality)
    Q_PROPERTY(QString region READ region WRITE setRegion)
    Q_PROPERTY(QString postalCode READ postalCode WRITE setPostalCode)
    Q_PROPERTY(QString country READ country WRITE setCountry)
    Q_PROPERTY(QString label READ label WRITE setLabel)
    Q_PROPERTY(KContacts::Geo geo READ geo WRITE setGeo)

    // TODO KF6: ideally this would be deprecated as it doesn't specify the formatting style
    // we cannot do that yet though due to Grantlee themes needing this (which cannot call
    // the invokable methods instead). The KF6::TextTemplate port might bring new options there,
    // otherwise we can at least switch this from postal to the multi-line style for KF6
    /**
     * Country-specific formatted address without an addressee using postal address style.
     * This is the same as calling formatted(AddressFormatStyle::Postal) with empty arguments.
     * @see formatted()
     * @since 5.12
     */
    Q_PROPERTY(QString formattedAddress READ formattedPostalAddress)

    /** geo: URI for this address.
     *  @see Address::geoUri()
     *  @since 5.106
     */
    Q_PROPERTY(QUrl geoUri READ geoUri)

public:
    /**
      List of addresses.
    */
    typedef QList<Address> List;

    /**
      Address types:
      @see Type
    */
    enum TypeFlag {
        Dom = 1, /**< domestic */
        Intl = 2, /**< international */
        Postal = 4, /**< postal */
        Parcel = 8, /**< parcel */
        Home = 16, /**< home address */
        Work = 32, /**< address at work */
        Pref = 64, /**< preferred address */
    };

    /**
     * Stores a combination of #TypeFlag values.
     */
    Q_DECLARE_FLAGS(Type, TypeFlag)
    Q_FLAG(Type)

    /**
      List of address types.
    */
    typedef QList<TypeFlag> TypeList;

    /**
      Creates an empty address.
    */
    Address();

    /**
      Creates an address of the given @p type.
    */
    Address(Type type);

    /**
      Copy constructor.
    */
    Address(const Address &address);

    /**
      Destroys the address.
    */
    ~Address();

    /**
      Equality operator.

      @param addr the address to compare to
      @return @c true if @c this and @p addr are equal, otherwise @c false
    */
    Q_REQUIRED_RESULT bool operator==(const Address &other) const;

    /**
      Not-equal operator.

      @param addr the address to compare to
      @return @c true if @c this and @p addr are not equal, otherwise @c false
    */
    Q_REQUIRED_RESULT bool operator!=(const Address &other) const;

    /**
      Assignment operator.

      @param addr the address data to assign to @c this
      @return a reference to @c this
    */
    Address &operator=(const Address &other);

    /**
      Returns true, if the address is empty.
    */
    Q_REQUIRED_RESULT bool isEmpty() const;

    /**
      Clears all entries of the address.
    */
    void clear();

    /**
      Sets the unique @p identifier.
    */
    void setId(const QString &identifier);

    /**
      Returns the unique identifier.
    */
    Q_REQUIRED_RESULT QString id() const;

    /**
      Sets the type of address. See enum for definition of types.

      @param type type, can be a bitwise or of multiple types.
    */
    void setType(Type type);

    /**
      Returns the type of address. Can be a bitwise or of multiple types.
    */
    Q_REQUIRED_RESULT Type type() const;

    /**
      Returns a translated string of all types the address has.
    */
    Q_REQUIRED_RESULT QString typeLabel() const;

    /**
      Sets the post office box.
    */
    void setPostOfficeBox(const QString &postOfficeBox);

    /**
      Returns the post office box.
    */
    Q_REQUIRED_RESULT QString postOfficeBox() const;

    /**
      Returns the translated label for post office box field.
    */
    static QString postOfficeBoxLabel();

    /**
      Sets the @p extended address information.
    */
    void setExtended(const QString &extended);

    /**
      Returns the extended address information.
    */
    Q_REQUIRED_RESULT QString extended() const;

    /**
      Returns the translated label for extended field.
    */
    static QString extendedLabel();

    /**
      Sets the @p street (including house number).
    */
    void setStreet(const QString &street);

    /**
      Returns the street.
    */
    Q_REQUIRED_RESULT QString street() const;

    /**
      Returns the translated label for street field.
    */
    static QString streetLabel();

    /**
      Sets the @p locality, e.g. city.

      @param locality the locality of the address, e.g. city
    */
    void setLocality(const QString &locality);

    /**
      Returns the locality.
    */
    Q_REQUIRED_RESULT QString locality() const;

    /**
      Returns the translated label for locality field.
    */
    static QString localityLabel();

    /**
      Sets the @p region, e.g. state.

      @param region the region the address falls into, e.g. state
    */
    void setRegion(const QString &region);

    /**
      Returns the region.
    */
    Q_REQUIRED_RESULT QString region() const;

    /**
      Returns the translated label for region field.
    */
    static QString regionLabel();

    /**
      Sets the postal @p code.
    */
    void setPostalCode(const QString &code);

    /**
      Returns the postal code.
    */
    Q_REQUIRED_RESULT QString postalCode() const;

    /**
      Returns the translated label for postal code field.
    */
    static QString postalCodeLabel();

    /**
      Sets the @p country.
    */
    void setCountry(const QString &country);

    /**
      Returns the country.
    */
    Q_REQUIRED_RESULT QString country() const;

    /**
      Returns the translated label for country field.
    */
    static QString countryLabel();

    /**
      Sets the delivery @p label. This is the literal text to be used as label.

      @param label the string to use for delivery labels
    */
    void setLabel(const QString &label);

    /**
      Returns the delivery label.
    */
    Q_REQUIRED_RESULT QString label() const;

    /**
      Returns the translated label for delivery label field.
    */
    static QString labelLabel();

    /**
      Returns the list of available types.
    */
    static TypeList typeList();

    /**
      Returns the translated label for the given @p type.
    */
    static QString typeLabel(Type type);

    /**
      Returns a string representation of the address.
    */
    Q_REQUIRED_RESULT QString toString() const;

    // note: cannot be called "formattedAddress" due to a collision
    // with the property of that name in QML
    /**
      Returns this address formatted according to the country-specific
      address formatting rules. The formatting rules applied depend on
      either the addresses {@link #country country} field, or (if the
      latter is empty) on the system country setting.

      @param style      the formatting style variant to use
      @param realName   the formatted name of the contact
      @param orgaName   the name of the organization or company
      @return           the formatted address
      @since 5.92
    */
    Q_REQUIRED_RESULT Q_INVOKABLE QString formatted(KContacts::AddressFormatStyle style,
                                                    const QString &realName = QString(),
                                                    const QString &orgaName = QString()) const;

    static QString typeFlagLabel(TypeFlag type);

    /**
      Set geographic position.
     */
    void setGeo(const Geo &geo);

    /**
      Return geographic position.
     */
    Q_REQUIRED_RESULT Geo geo() const;

    /**
     * Returns a geo: URI representing this address.
     * This contains either the geographic coordinate if set, or the address as query term.
     * This can be used to show the address in the default map view.
     * @since 5.106
     */
    Q_REQUIRED_RESULT QUrl geoUri() const;

private:
    KCONTACTS_NO_EXPORT QString formattedPostalAddress() const;

    class Private;
    QSharedDataPointer<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Address::Type)

/**
  Serializes the @p address object into the @p stream.
*/
KCONTACTS_EXPORT QDataStream &operator<<(QDataStream &stream, const Address &address);

/**
  Initializes the @p address object from the @p stream.
*/
KCONTACTS_EXPORT QDataStream &operator>>(QDataStream &stream, Address &address);
}

#endif
