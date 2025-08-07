/*
    SPDX-FileCopyrightText: 2006 Jimmy Gilles <jimmygilles@gmail.com>
    SPDX-FileCopyrightText: 2010, 2013, 2014, 2016 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONVERT_H
#define CONVERT_H

#include "kgpgkey.h"

#include <gpgme.h>

class QString;

namespace KgpgCore
{

namespace Convert
{
    QString toString(const KgpgCore::KgpgKeyAlgo algorithm);
    QString toString(const gpgme_validity_t ownertrust);
    QString toString(const KgpgCore::KgpgKeyTrust trust);
    QString toString(const KgpgCore::KgpgSubKeyType type);
    KgpgKeyAlgo toAlgo(const uint v);
    KgpgKeyAlgo toAlgo(const QString &s);
    KgpgKeyTrust toTrust(const QChar &c);
    KgpgKeyTrust toTrust(const QString &s);
    gpgme_validity_t toOwnerTrust(const QChar &c);
    /**
     * @brief parse the GnuPG capabilities field
     * @param capString the capability string as returned by GnuPG
     * @param upper if the uppercase or lowercase version should be parsed
     */
    KgpgSubKeyType toSubType(const QString &capString, bool upper);
    /**
     * @brief parse GnuPG date fields
     * @param s the input string
     * @return the parsed date
     * @retval QDateTime() if s was empty or the string was invalid
     *
     * Both the seconds since epoch UTC as well as ISO 8601 formats are
     * supported.
     */
    QDateTime toDateTime(const QString &s);
}

} // namespace KgpgCore

#endif // CONVERT_H
