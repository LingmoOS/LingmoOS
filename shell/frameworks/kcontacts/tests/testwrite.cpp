/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 KDE-PIM team <kde-pim@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QFile>

#include <KLocalizedString>

#include <QCommandLineParser>
#include <QCoreApplication>

#include "converter/kcontacts/vcardconverter.h"
#include "kcontacts/address.h"
#include "kcontacts/addressee.h"
#include "kcontacts/key.h"
#include "kcontacts/phonenumber.h"
#include "kcontacts/picture.h"
#include "kcontacts/secrecy.h"
#include "kcontacts/sound.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    KContacts::Addressee addressee;

    addressee.setNameFromString(QLatin1String("Mr. Tobias Koenig Jr."));
    addressee.setNickName(QLatin1String("tokoe"));
    addressee.setBirthday(QDateTime(QDate(1982, 7, 19).startOfDay()));
    addressee.setMailer(QLatin1String("mutt1.2"));
    addressee.setTimeZone(KContacts::TimeZone(+2));

    KContacts::Geo geo;
    geo.setLatitude(30);
    geo.setLongitude(51);
    addressee.setGeo(geo);

    addressee.setTitle(QLatin1String("nerd"));
    addressee.setRole(QLatin1String("Maintainer"));
    addressee.setOrganization(QLatin1String("KDE"));
    addressee.setNote(QLatin1String("never\ntouch a running system"));
    addressee.setProductId(QLatin1String("testId"));
    addressee.setRevision(QDateTime::currentDateTime());
    addressee.setSortString(QLatin1String("koenig"));
    KContacts::ResourceLocatorUrl url;
    url.setUrl(QUrl(QLatin1String("http://wgess16.dyndns.org")));
    addressee.setUrl(url);
    addressee.setSecrecy(KContacts::Secrecy(KContacts::Secrecy::Confidential));

    KContacts::Email kdemail(QLatin1String("tokoe@kde.org"));
    kdemail.setPreferred(true);
    addressee.addEmail(kdemail);

    KContacts::Email yahoomail(QLatin1String("tokoe82@yahoo.de"));
    yahoomail.setPreferred(true);
    addressee.addEmail(yahoomail);

    KContacts::PhoneNumber phone1(QLatin1String("3541523475"), KContacts::PhoneNumber::Pref | KContacts::PhoneNumber::Home);
    KContacts::PhoneNumber phone2(QLatin1String("+46745673475"), KContacts::PhoneNumber::Work);
    addressee.insertPhoneNumber(phone1);
    addressee.insertPhoneNumber(phone2);

    KContacts::Key key(QLatin1String("secret key"), KContacts::Key::X509);
    addressee.insertKey(key);

    QStringList categories;
    categories << QLatin1String("Friends") << QLatin1String("School") << QLatin1String("KDE");
    addressee.setCategories(categories);

    KContacts::Address a(KContacts::Address::Work | KContacts::Address::Postal | KContacts::Address::Parcel);
    a.setStreet(QLatin1String("6544 Battleford Drive"));
    a.setLocality(QLatin1String("Raleigh"));
    a.setRegion(QLatin1String("NC"));
    a.setPostalCode(QLatin1String("27613-3502"));
    a.setCountry(QLatin1String("U.S.A."));
    addressee.insertAddress(a);

    addressee.insertCustom(QLatin1String("1hsdf"), QLatin1String("ertuer"), QLatin1String("iurt"));
    addressee.insertCustom(QLatin1String("2hsdf"), QLatin1String("ertuer"), QLatin1String("iurt"));
    addressee.insertCustom(QLatin1String("3hsdf"), QLatin1String("ertuer"), QLatin1String("iurt"));

    KContacts::Addressee::List list;
    for (int i = 0; i < 1000; ++i) {
        KContacts::Addressee addr = addressee;
        addr.setUid(QString::number(i));
        list.append(addr);
    }

    KContacts::VCardConverter converter;
    QByteArray txt = converter.createVCards(list);

    QFile file(QLatin1String("out.vcf"));
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug("Can't open file '%s' for writing", qPrintable(file.fileName()));
        return 1;
    }

    file.write(txt);
    file.close();

    return 0;
}
