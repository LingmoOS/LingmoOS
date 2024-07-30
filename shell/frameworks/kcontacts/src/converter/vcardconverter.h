/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2002 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_VCARDCONVERTER_H
#define KCONTACTS_VCARDCONVERTER_H

#include "kcontacts/addressee.h"
#include "kcontacts_export.h"
#include <QString>

namespace KContacts
{
/**
  @short Class to converting contact objects into vCard format and vice versa.

  This class implements reading and writing of contact using from/to the
  vCard format. Currently vCard version 2.1 and 3.0 is supported.

  Example:

  \code

  QFile file( "myfile.vcf" );
  file.open( QIODevice::ReadOnly );

  QByteArray data = file.readAll();

  VCardConverter converter;
  Addressee::List list = converter.parseVCards( data );

  // print formatted name of first contact
  qDebug( "name=%s", list[ 0 ].formattedName().toLatin1() );

  \endcode
*/
class KCONTACTS_EXPORT VCardConverter
{
public:
    /**
      @li v2_1 - VCard format version 2.1
      @li v3_0 - VCard format version 3.0
      @li v4_0 - VCard format version 4.0
     */
    enum Version {
        v2_1,
        v3_0,
        v4_0,
    };

    /**
      Constructor.
     */
    VCardConverter();

    /**
      Destructor.
     */
    ~VCardConverter();

    /**
      Creates a string in vCard format which contains the given
      contact.

      @param addr The contact object
      @param version The version of the generated vCard format
     */
    Q_REQUIRED_RESULT QByteArray createVCard(const Addressee &addr, Version version = v3_0) const;

    /**
      Creates a string in vCard format which contains the given
      list of contact.

      @param list The list of contact objects
      @param version The version of the generated vCard format
     */
    // FIXME: Add error handling
    Q_REQUIRED_RESULT QByteArray createVCards(const Addressee::List &list, Version version = v3_0) const;

    /**
     * @since 4.9.1
     */
    Q_REQUIRED_RESULT QByteArray exportVCard(const Addressee &addr, Version version) const;

    /**
     * @since 4.9.1
     */
    Q_REQUIRED_RESULT QByteArray exportVCards(const Addressee::List &list, Version version) const;

    /**
      Parses a string in vCard format and returns the first contact.
     */
    Q_REQUIRED_RESULT Addressee parseVCard(const QByteArray &vcard) const;

    /**
      Parses a string in vCard format and returns a list of contact objects.
     */
    // FIXME: Add error handling
    Q_REQUIRED_RESULT Addressee::List parseVCards(const QByteArray &vcard) const;

private:
    Q_DISABLE_COPY(VCardConverter)
    class VCardConverterPrivate;
    VCardConverterPrivate *const d;
};

/**
    Helper functions
  */

/**
 * Converts a QDateTime to a date string as it is used in VCard and LDIF files.
 * The return value is in the form "yyyyMMddThhmmssZ" (e.g. "20031201T120000Z")
 * @param dateTime date and time to be converted
 */
Q_REQUIRED_RESULT KCONTACTS_EXPORT QString dateToVCardString(const QDateTime &dateTime);

/**
 * Converts a QDate to a short date string as it is used in VCard and LDIF files.
 * The return value is in the form "yyyyMMdd" (e.g. "20031201")
 * @param date date to be converted
 */
Q_REQUIRED_RESULT KCONTACTS_EXPORT QString dateToVCardString(QDate date);

/**
 * Converts a date string as it is used in VCard and LDIF files to a QDateTime value.
 * If the date string does not contain a time value, it will be returned as 00:00:00.
 * (e.g. "20031201T120000" will return a QDateTime for 2003-12-01 at 12:00)
 * @param dateString string representing the date and time.
 */
Q_REQUIRED_RESULT KCONTACTS_EXPORT QDateTime VCardStringToDate(const QString &dateString);

/**
 * @brief adaptIMAttributes. Convert KAddressBook attribute to VCard IM Attribute
 * @param data
 */
KCONTACTS_EXPORT void adaptIMAttributes(QByteArray &data);
}
#endif
