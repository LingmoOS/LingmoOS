/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_VCARDTOOL_H
#define KCONTACTS_VCARDTOOL_H

#include "addressee.h"
#include "kcontacts_export.h"
#include "vcardparser/vcardparser_p.h"

class QDateTime;

namespace KContacts
{
class Key;
class Picture;
class Secrecy;
class Sound;

class KCONTACTS_EXPORT VCardTool
{
public:
    VCardTool();
    ~VCardTool();

    /**
      Creates a string that contains the addressees from the list in
      the vCard format.
     */
    Q_REQUIRED_RESULT QByteArray createVCards(const Addressee::List &list, VCard::Version version = VCard::v3_0) const;

    /**
     * @since 4.9.1
     */
    Q_REQUIRED_RESULT QByteArray exportVCards(const Addressee::List &list, VCard::Version version = VCard::v3_0) const;
    /**
      Parses the string and returns a list of addressee objects.
     */
    Q_REQUIRED_RESULT Addressee::List parseVCards(const QByteArray &vcard) const;

    static QDateTime parseDateTime(const QString &str, bool *timeValid = nullptr);
    static QString createDateTime(const QDateTime &dateTime, VCard::Version version, bool withTime = true);
    static QString createDate(const QDate &date, VCard::Version version);
    static QString createTime(const QTime &time, VCard::Version version);

private:
    KCONTACTS_NO_EXPORT
    QByteArray createVCards(const Addressee::List &list, VCard::Version version, bool exportVcard) const;

    /**
      Split a string and replaces escaped separators on the fly with
      unescaped ones.
     */
    KCONTACTS_NO_EXPORT
    QStringList splitString(QChar sep, const QString &value) const;

    KCONTACTS_NO_EXPORT
    Picture parsePicture(const VCardLine &line) const;
    KCONTACTS_NO_EXPORT
    VCardLine createPicture(const QString &identifier, const Picture &pic, VCard::Version version) const;

    KCONTACTS_NO_EXPORT
    Sound parseSound(const VCardLine &line) const;
    KCONTACTS_NO_EXPORT
    VCardLine createSound(const Sound &snd, VCard::Version version) const;

    KCONTACTS_NO_EXPORT
    Key parseKey(const VCardLine &line) const;
    KCONTACTS_NO_EXPORT
    VCardLine createKey(const Key &key, VCard::Version version) const;

    KCONTACTS_NO_EXPORT
    Secrecy parseSecrecy(const VCardLine &line) const;
    KCONTACTS_NO_EXPORT
    VCardLine createSecrecy(const Secrecy &secrecy) const;

    KCONTACTS_NO_EXPORT
    void addParameter(VCardLine *line, VCard::Version version, const QString &key, const QStringList &valueStringList) const;

    /** Translate alternative or legacy IMPP service types. */
    KCONTACTS_NO_EXPORT
    QString normalizeImppServiceType(const QString &serviceType) const;

    KCONTACTS_NO_EXPORT
    void processAddresses(const Address::List &addresses, VCard::Version version, VCard *card) const;
    KCONTACTS_NO_EXPORT
    void processEmailList(const Email::List &emailList, VCard::Version version, VCard *card) const;
    KCONTACTS_NO_EXPORT
    void processOrganizations(const Addressee &addressee, VCard::Version version, VCard *card) const;
    KCONTACTS_NO_EXPORT
    void processPhoneNumbers(const PhoneNumber::List &phoneNumbers, VCard::Version version, VCard *card) const;
    KCONTACTS_NO_EXPORT
    void processCustoms(const QStringList &customs, VCard::Version version, VCard *card, bool exportVcard) const;

    Q_DISABLE_COPY(VCardTool)
};
}

#endif
