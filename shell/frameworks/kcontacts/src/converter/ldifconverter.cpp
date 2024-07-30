/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2003 Helge Deller <deller@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
    Useful links:
        - http://tldp.org/HOWTO/LDAP-Implementation-HOWTO/schemas.html
        - http://www.faqs.org/rfcs/rfc2849.html

    Not yet handled items:
        - objectclass microsoftaddressbook
                - info,
                - initials,
                - otherfacsimiletelephonenumber,
                - otherpager,
                - physicaldeliveryofficename,
*/

#include "ldifconverter.h"
#include "address.h"
#include "kcontacts_debug.h"
#include "vcardconverter.h"

#include "ldif_p.h"

#include <KCountry>
#include <KLocalizedString>

#include <QIODevice>
#include <QStringList>
#include <QTextStream>

using namespace KContacts;

/* internal functions - do not use !! */

namespace KContacts
{
/**
  @internal

  Evaluates @p fieldname and sets the @p value at the addressee or the address
  objects when appropriate.

  @param a The addressee to store information into
  @param homeAddr The home address to store respective information into
  @param workAddr The work address to store respective information into
  @param fieldname LDIF field name to evaluate
  @param value The value of the field addressed by @p fieldname
*/
void evaluatePair(Addressee &a,
                  Address &homeAddr,
                  Address &workAddr,
                  QString &fieldname,
                  QString &value,
                  int &birthday,
                  int &birthmonth,
                  int &birthyear,
                  ContactGroup &contactGroup);
}

/* generate LDIF stream */

static void ldif_out(QTextStream &t, const QString &formatStr, const QString &value)
{
    if (value.isEmpty()) {
        return;
    }

    const QByteArray txt = Ldif::assembleLine(formatStr, value, 72);

    // write the string
    t << QString::fromUtf8(txt) << "\n";
}

bool LDIFConverter::addresseeAndContactGroupToLDIF(const AddresseeList &addrList, const ContactGroup::List &contactGroupList, QString &str)
{
    bool result = addresseeToLDIF(addrList, str);
    if (!contactGroupList.isEmpty()) {
        result = (contactGroupToLDIF(contactGroupList, str) || result); // order matters
    }
    return result;
}

bool LDIFConverter::contactGroupToLDIF(const ContactGroup &contactGroup, QString &str)
{
    if (contactGroup.dataCount() <= 0) {
        return false;
    }
    QTextStream t(&str, QIODevice::WriteOnly | QIODevice::Append);

    t << "objectclass: top\n";
    t << "objectclass: groupOfNames\n";

    for (int i = 0; i < contactGroup.dataCount(); ++i) {
        const ContactGroup::Data &data = contactGroup.data(i);
        const QString value = QStringLiteral("cn=%1,mail=%2").arg(data.name(), data.email());
        ldif_out(t, QStringLiteral("member"), value);
    }

    t << "\n";
    return true;
}

bool LDIFConverter::contactGroupToLDIF(const ContactGroup::List &contactGroupList, QString &str)
{
    if (contactGroupList.isEmpty()) {
        return false;
    }

    bool result = true;
    for (const ContactGroup &group : contactGroupList) {
        result = (contactGroupToLDIF(group, str) || result); // order matters
    }
    return result;
}

bool LDIFConverter::addresseeToLDIF(const AddresseeList &addrList, QString &str)
{
    if (addrList.isEmpty()) {
        return false;
    }

    bool result = true;
    for (const Addressee &addr : addrList) {
        result = (addresseeToLDIF(addr, str) || result); // order matters
    }
    return result;
}

static QString countryName(const QString &isoCodeOrName)
{
    const auto c = KCountry::fromAlpha2(isoCodeOrName);
    return c.isValid() ? c.name() : isoCodeOrName;
}

bool LDIFConverter::addresseeToLDIF(const Addressee &addr, QString &str)
{
    if (addr.isEmpty()) {
        return false;
    }

    QTextStream t(&str, QIODevice::WriteOnly | QIODevice::Append);

    const Address homeAddr = addr.address(Address::Home);
    const Address workAddr = addr.address(Address::Work);

    ldif_out(t, QStringLiteral("dn"), QStringLiteral("cn=%1,mail=%2").arg(addr.formattedName().simplified(), addr.preferredEmail()));
    t << "objectclass: top\n";
    t << "objectclass: person\n";
    t << "objectclass: organizationalPerson\n";

    ldif_out(t, QStringLiteral("givenname"), addr.givenName());
    ldif_out(t, QStringLiteral("sn"), addr.familyName());
    ldif_out(t, QStringLiteral("cn"), addr.formattedName().simplified());
    ldif_out(t, QStringLiteral("uid"), addr.uid());
    ldif_out(t, QStringLiteral("nickname"), addr.nickName());
    ldif_out(t, QStringLiteral("xmozillanickname"), addr.nickName());
    ldif_out(t, QStringLiteral("mozillanickname"), addr.nickName());

    ldif_out(t, QStringLiteral("mail"), addr.preferredEmail());
    const QStringList emails = addr.emails();
    const int numEmails = emails.count();
    for (int i = 1; i < numEmails; ++i) {
        if (i == 0) {
            // nothing
        } else if (i == 1) {
            ldif_out(t, QStringLiteral("mozillasecondemail"), emails[1]);
        } else {
            ldif_out(t, QStringLiteral("othermailbox"), emails[i]);
        }
    }
    // ldif_out( t, "mozilla_AIMScreenName: %1\n", "screen_name" );

    ldif_out(t, QStringLiteral("telephonenumber"), addr.phoneNumber(PhoneNumber::Work).number());
    ldif_out(t, QStringLiteral("facsimiletelephonenumber"), addr.phoneNumber(PhoneNumber::Fax).number());
    ldif_out(t, QStringLiteral("homephone"), addr.phoneNumber(PhoneNumber::Home).number());
    ldif_out(t, QStringLiteral("mobile"),
             addr.phoneNumber(PhoneNumber::Cell).number()); // Netscape 7
    ldif_out(t, QStringLiteral("cellphone"),
             addr.phoneNumber(PhoneNumber::Cell).number()); // Netscape 4.x
    ldif_out(t, QStringLiteral("pager"), addr.phoneNumber(PhoneNumber::Pager).number());
    ldif_out(t, QStringLiteral("pagerphone"), addr.phoneNumber(PhoneNumber::Pager).number());

    ldif_out(t, QStringLiteral("streethomeaddress"), homeAddr.street());
    ldif_out(t, QStringLiteral("postalcode"), workAddr.postalCode());
    ldif_out(t, QStringLiteral("postofficebox"), workAddr.postOfficeBox());

    QStringList streets = homeAddr.street().split(QLatin1Char('\n'));
    const int numberOfStreets(streets.count());
    if (numberOfStreets > 0) {
        ldif_out(t, QStringLiteral("homepostaladdress"), streets.at(0)); // Netscape 7
    }
    if (numberOfStreets > 1) {
        ldif_out(t, QStringLiteral("mozillahomepostaladdress2"), streets.at(1)); // Netscape 7
    }
    ldif_out(t, QStringLiteral("mozillahomelocalityname"), homeAddr.locality()); // Netscape 7
    ldif_out(t, QStringLiteral("mozillahomestate"), homeAddr.region());
    ldif_out(t, QStringLiteral("mozillahomepostalcode"), homeAddr.postalCode());
    ldif_out(t, QStringLiteral("mozillahomecountryname"), countryName(homeAddr.country()));
    ldif_out(t, QStringLiteral("locality"), workAddr.locality());
    ldif_out(t, QStringLiteral("streetaddress"), workAddr.street()); // Netscape 4.x

    streets = workAddr.street().split(QLatin1Char('\n'));
    const int streetsCount = streets.count();
    if (streetsCount > 0) {
        ldif_out(t, QStringLiteral("street"), streets.at(0));
    }
    if (streetsCount > 1) {
        ldif_out(t, QStringLiteral("mozillaworkstreet2"), streets.at(1));
    }
    ldif_out(t, QStringLiteral("countryname"), countryName(workAddr.country()));
    ldif_out(t, QStringLiteral("l"), workAddr.locality());
    ldif_out(t, QStringLiteral("c"), countryName(workAddr.country()));
    ldif_out(t, QStringLiteral("st"), workAddr.region());

    ldif_out(t, QStringLiteral("title"), addr.title());
    ldif_out(t, QStringLiteral("vocation"), addr.prefix());
    ldif_out(t, QStringLiteral("ou"), addr.role());
    ldif_out(t, QStringLiteral("o"), addr.organization());
    ldif_out(t, QStringLiteral("organization"), addr.organization());
    ldif_out(t, QStringLiteral("organizationname"), addr.organization());

    // Compatibility with older kabc versions.
    if (!addr.department().isEmpty()) {
        ldif_out(t, QStringLiteral("department"), addr.department());
    } else {
        ldif_out(t, QStringLiteral("department"), addr.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Department")));
    }

    ldif_out(t, QStringLiteral("workurl"), addr.url().url().toDisplayString());
    ldif_out(t, QStringLiteral("homeurl"), addr.url().url().toDisplayString());
    ldif_out(t, QStringLiteral("mozillahomeurl"), addr.url().url().toDisplayString());

    ldif_out(t, QStringLiteral("description"), addr.note());
    if (addr.revision().isValid()) {
        ldif_out(t, QStringLiteral("modifytimestamp"), dateToVCardString(addr.revision()));
    }

    const QDate birthday = addr.birthday().date();
    if (birthday.isValid()) {
        const int year = birthday.year();
        if (year > 0) {
            ldif_out(t, QStringLiteral("birthyear"), QString::number(year));
        }
        ldif_out(t, QStringLiteral("birthmonth"), QString::number(birthday.month()));
        ldif_out(t, QStringLiteral("birthday"), QString::number(birthday.day()));
    }

    t << "\n";

    return true;
}

/* convert from LDIF stream */
bool LDIFConverter::LDIFToAddressee(const QString &str, AddresseeList &addrList, ContactGroup::List &contactGroupList, const QDateTime &dt)
{
    if (str.isEmpty()) {
        return true;
    }

    bool endldif = false;
    bool end = false;
    Ldif ldif;
    Ldif::ParseValue ret;
    Addressee a;
    Address homeAddr;
    Address workAddr;
    int birthday = -1;
    int birthmonth = -1;
    int birthyear = -1;
    ContactGroup contactGroup;
    ldif.setLdif(str.toLatin1());
    QDateTime qdt = dt;
    if (!qdt.isValid()) {
        qdt = QDateTime::currentDateTime();
    }
    a.setRevision(qdt);
    homeAddr = Address(Address::Home);
    workAddr = Address(Address::Work);

    do {
        ret = ldif.nextItem();
        switch (ret) {
        case Ldif::Item: {
            QString fieldname = ldif.attr().toLower();
            QString value = QString::fromUtf8(ldif.value());
            evaluatePair(a, homeAddr, workAddr, fieldname, value, birthday, birthmonth, birthyear, contactGroup);
            break;
        }
        case Ldif::EndEntry:
            if (contactGroup.count() == 0) {
                // if the new address is not empty, append it
                QDate birthDate(birthyear, birthmonth, birthday);
                if (birthDate.isValid()) {
                    a.setBirthday(birthDate);
                }

                if (!a.formattedName().isEmpty() || !a.name().isEmpty() || !a.familyName().isEmpty()) {
                    if (!homeAddr.isEmpty()) {
                        a.insertAddress(homeAddr);
                    }
                    if (!workAddr.isEmpty()) {
                        a.insertAddress(workAddr);
                    }
                    addrList.append(a);
                }
            } else {
                contactGroupList.append(contactGroup);
            }
            a = Addressee();
            contactGroup = ContactGroup();
            a.setRevision(qdt);
            homeAddr = Address(Address::Home);
            workAddr = Address(Address::Work);
            break;
        case Ldif::MoreData:
            if (endldif) {
                end = true;
            } else {
                ldif.endLdif();
                endldif = true;
                break;
            }
        default:
            break;
        }
    } while (!end);

    return true;
}

void KContacts::evaluatePair(Addressee &a,
                             Address &homeAddr,
                             Address &workAddr,
                             QString &fieldname,
                             QString &value,
                             int &birthday,
                             int &birthmonth,
                             int &birthyear,
                             ContactGroup &contactGroup)
{
    if (fieldname == QLatin1String("dn")) { // ignore
        return;
    }

    if (fieldname.startsWith(QLatin1Char('#'))) {
        return;
    }

    if (fieldname.isEmpty() && !a.note().isEmpty()) {
        // some LDIF export filters are broken and add additional
        // comments on stand-alone lines. Just add them to the notes for now.
        a.setNote(a.note() + QLatin1Char('\n') + value);
        return;
    }

    if (fieldname == QLatin1String("givenname")) {
        a.setGivenName(value);
        return;
    }

    if (fieldname == QLatin1String("xmozillanickname") //
        || fieldname == QLatin1String("nickname") //
        || fieldname == QLatin1String("mozillanickname")) {
        a.setNickName(value);
        return;
    }

    if (fieldname == QLatin1String("sn")) {
        a.setFamilyName(value);
        return;
    }

    if (fieldname == QLatin1String("uid")) {
        a.setUid(value);
        return;
    }
    if (fieldname == QLatin1String("mail") //
        || fieldname == QLatin1String("mozillasecondemail") /* mozilla */
        || fieldname == QLatin1String("othermailbox") /*TheBat!*/) {
        if (a.emails().indexOf(value) == -1) {
            a.addEmail(value);
        }
        return;
    }

    if (fieldname == QLatin1String("title")) {
        a.setTitle(value);
        return;
    }

    if (fieldname == QLatin1String("vocation")) {
        a.setPrefix(value);
        return;
    }

    if (fieldname == QLatin1String("cn")) {
        a.setFormattedName(value);
        return;
    }

    if (fieldname == QLatin1Char('o') || fieldname == QLatin1String("organization") // Exchange
        || fieldname == QLatin1String("organizationname")) { // Exchange
        a.setOrganization(value);
        return;
    }

    // clang-format off
    if (fieldname == QLatin1String("description")
        || fieldname == QLatin1String("mozillacustom1")
        || fieldname == QLatin1String("mozillacustom2")
        || fieldname == QLatin1String("mozillacustom3")
        || fieldname == QLatin1String("mozillacustom4")
        || fieldname == QLatin1String("custom1")
        || fieldname == QLatin1String("custom2")
        || fieldname == QLatin1String("custom3")
        || fieldname == QLatin1String("custom4")) {
        if (!a.note().isEmpty()) {
            a.setNote(a.note() + QLatin1Char('\n'));
        }
        a.setNote(a.note() + value);
        return;
    }
    // clang-format on

    if (fieldname == QLatin1String("homeurl") //
        || fieldname == QLatin1String("workurl") //
        || fieldname == QLatin1String("mozillahomeurl")) {
        if (a.url().url().isEmpty()) {
            ResourceLocatorUrl url;
            url.setUrl(QUrl(value));
            a.setUrl(url);
            return;
        }
        if (a.url().url().toDisplayString() == QUrl(value).toDisplayString()) {
            return;
        }
        // TODO: current version of kabc only supports one URL.
        // TODO: change this with KDE 4
    }

    if (fieldname == QLatin1String("homephone")) {
        a.insertPhoneNumber(PhoneNumber(value, PhoneNumber::Home));
        return;
    }

    if (fieldname == QLatin1String("telephonenumber")) {
        a.insertPhoneNumber(PhoneNumber(value, PhoneNumber::Work));
        return;
    }
    if (fieldname == QLatin1String("mobile") /* mozilla/Netscape 7 */
        || fieldname == QLatin1String("cellphone")) {
        a.insertPhoneNumber(PhoneNumber(value, PhoneNumber::Cell));
        return;
    }

    if (fieldname == QLatin1String("pager") // mozilla
        || fieldname == QLatin1String("pagerphone")) { // mozilla
        a.insertPhoneNumber(PhoneNumber(value, PhoneNumber::Pager));
        return;
    }

    if (fieldname == QLatin1String("facsimiletelephonenumber")) {
        a.insertPhoneNumber(PhoneNumber(value, PhoneNumber::Fax));
        return;
    }

    if (fieldname == QLatin1String("xmozillaanyphone")) { // mozilla
        a.insertPhoneNumber(PhoneNumber(value, PhoneNumber::Work));
        return;
    }

    if (fieldname == QLatin1String("streethomeaddress") //
        || fieldname == QLatin1String("mozillahomestreet")) { // thunderbird
        homeAddr.setStreet(value);
        return;
    }

    if (fieldname == QLatin1String("street") //
        || fieldname == QLatin1String("postaladdress")) { // mozilla
        workAddr.setStreet(value);
        return;
    }
    if (fieldname == QLatin1String("mozillapostaladdress2") //
        || fieldname == QLatin1String("mozillaworkstreet2")) { // mozilla
        workAddr.setStreet(workAddr.street() + QLatin1Char('\n') + value);
        return;
    }

    if (fieldname == QLatin1String("postalcode")) {
        workAddr.setPostalCode(value);
        return;
    }

    if (fieldname == QLatin1String("postofficebox")) {
        workAddr.setPostOfficeBox(value);
        return;
    }

    if (fieldname == QLatin1String("homepostaladdress")) { // Netscape 7
        homeAddr.setStreet(value);
        return;
    }

    if (fieldname == QLatin1String("mozillahomepostaladdress2")) { // mozilla
        homeAddr.setStreet(homeAddr.street() + QLatin1Char('\n') + value);
        return;
    }

    if (fieldname == QLatin1String("mozillahomelocalityname")) { // mozilla
        homeAddr.setLocality(value);
        return;
    }

    if (fieldname == QLatin1String("mozillahomestate")) { // mozilla
        homeAddr.setRegion(value);
        return;
    }

    if (fieldname == QLatin1String("mozillahomepostalcode")) { // mozilla
        homeAddr.setPostalCode(value);
        return;
    }

    if (fieldname == QLatin1String("mozillahomecountryname")) { // mozilla
        if (value.length() <= 2) {
            value = countryName(value);
        }
        homeAddr.setCountry(value);
        return;
    }

    if (fieldname == QLatin1String("locality")) {
        workAddr.setLocality(value);
        return;
    }

    if (fieldname == QLatin1String("streetaddress")) { // Netscape 4.x
        workAddr.setStreet(value);
        return;
    }

    if (fieldname == QLatin1String("countryname") //
        || fieldname == QLatin1Char('c')) { // mozilla
        if (value.length() <= 2) {
            value = countryName(value);
        }
        workAddr.setCountry(value);
        return;
    }

    if (fieldname == QLatin1Char('l')) { // mozilla
        workAddr.setLocality(value);
        return;
    }

    if (fieldname == QLatin1String("st")) {
        workAddr.setRegion(value);
        return;
    }

    if (fieldname == QLatin1String("ou")) {
        a.setRole(value);
        return;
    }

    if (fieldname == QLatin1String("department")) {
        a.setDepartment(value);
        return;
    }

    if (fieldname == QLatin1String("member")) {
        // this is a mozilla list member (cn=xxx, mail=yyy)
        const QStringList list = value.split(QLatin1Char(','));
        QString name;
        QString email;

        const QLatin1String cnTag("cn=");
        const QLatin1String mailTag("mail=");
        for (const auto &str : list) {
            if (str.startsWith(cnTag)) {
                name = QStringView(str).mid(cnTag.size()).trimmed().toString();
            } else if (str.startsWith(mailTag)) {
                email = QStringView(str).mid(mailTag.size()).trimmed().toString();
            }
        }

        if (!name.isEmpty() && !email.isEmpty()) {
            email = QLatin1String(" <") + email + QLatin1Char('>');
        }
        ContactGroup::Data data;
        data.setEmail(email);
        data.setName(name);
        contactGroup.append(data);
        return;
    }

    if (fieldname == QLatin1String("modifytimestamp")) {
        if (value == QLatin1String("0Z")) { // ignore
            return;
        }
        QDateTime dt = VCardStringToDate(value);
        if (dt.isValid()) {
            a.setRevision(dt);
            return;
        }
    }

    if (fieldname == QLatin1String("display-name")) {
        contactGroup.setName(value);
        return;
    }

    if (fieldname == QLatin1String("objectclass")) { // ignore
        return;
    }

    if (fieldname == QLatin1String("birthyear")) {
        bool ok;
        birthyear = value.toInt(&ok);
        if (!ok) {
            birthyear = -1;
        }
        return;
    }
    if (fieldname == QLatin1String("birthmonth")) {
        birthmonth = value.toInt();
        return;
    }
    if (fieldname == QLatin1String("birthday")) {
        birthday = value.toInt();
        return;
    }
    if (fieldname == QLatin1String("xbatbirthday")) {
        const QStringView str{value};
        QDate dt(str.mid(0, 4).toInt(), str.mid(4, 2).toInt(), str.mid(6, 2).toInt());
        if (dt.isValid()) {
            a.setBirthday(dt);
        }
        return;
    }
    qCWarning(KCONTACTS_LOG) << QStringLiteral("LDIFConverter: Unknown field for '%1': '%2=%3'\n").arg(a.formattedName(), fieldname, value);
}
