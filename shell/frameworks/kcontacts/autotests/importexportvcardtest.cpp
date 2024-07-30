/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "importexportvcardtest.h"
#include "vcardtool_p.h"
#include <QDebug>
#include <QTest>

ImportExportVCardTest::ImportExportVCardTest(QObject *parent)
    : QObject(parent)
{
}

ImportExportVCardTest::~ImportExportVCardTest()
{
}

static void compareBuffers(const QByteArray &outputData, const QByteArray &expected)
{
    if (outputData != expected) {
        qDebug() << " outputData " << outputData;
        qDebug() << " expected " << expected;
    }
    const QList<QByteArray> outputLines = outputData.split('\n');
    const QList<QByteArray> outputRefLines = expected.split('\n');
    for (int i = 0; i < qMin(outputLines.count(), outputRefLines.count()); ++i) {
        const QByteArray &actual = outputLines.at(i);
        const QByteArray &expect = outputRefLines.at(i);
        if (actual != expect) {
            qCritical() << "Mismatch at output line" << (i + 1);
            QCOMPARE(actual, expect);
            QCOMPARE(actual.size(), expect.size());
        }
    }
    QCOMPARE(outputLines.count(), outputRefLines.count());
    QCOMPARE(outputData.size(), expected.size());
}

void ImportExportVCardTest::shouldExportFullTestVcard4()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "FN:Sherlock Holmes\r\n"
        "TEL;VALUE=uri;PREF=1;TYPE=\"voice,home\":tel:+44-555-555-5555;ext=5555\r\n"
        "TEL;VALUE=uri;TYPE=\"voice,cell,text\":tel:+44-555-555-6666\r\n"
        "TEL;VALUE=uri;TYPE=\"voice,work\":tel:+44-555-555-7777\r\n"
        "N:Holmes;Sherlock;;Mr;;\r\n"
        "KIND:individual\r\n"
        "NICKNAME:Shirley\r\n"
        "PHOTO:\r\n"
        "BDAY:19531015T231000Z\r\n"
        "ANNIVERSARY:19960415\r\n"
        "GENDER:M\r\n"
        "ADR;GEO=\"geo:51.5237,0.1585\";LABEL=\"Mr Sherlock Holmes, 221B Baker Street, London NW1, England, United Kingdom\":;;221B Baker "
        "Street;London;;NW1;United Kingdom\r\n"
        "EMAIL;TYPE=home:sherlock.holmes@gmail.com\r\n"
        "EMAIL;PREF=1;TYPE=work:detective@sherlockholmes.com\r\n"
        "IMPP;PREF=1:xmpp:detective@sherlockholmes.com\r\n"
        "LANG;TYPE=work;PREF=1:en\r\n"
        "LANG;TYPE=work;PREF=2:fr\r\n"
        "TZ:London/Europe\r\n"
        "GEO:geo:51.5237,0.1585\r\n"
        "TITLE;ALTID=1;LANGUAGE=fr:Patron\r\n"
        "TITLE;ALTID=2;LANGUAGE=en:Boss\r\n"
        "ROLE:Detective\r\n"
        "UID:urn:uuid:b8767877-b4a1-4c70-9acc-505d3819e519\r\n"
        "CATEGORIES:FICTION,LITERATURE\r\n"
        "PRODID:-//KADDRESSBOOK//NONSGML Version 1//EN\r\n"
        "REV:20140722T222710Z\r\n"
        "URL;TYPE=home:https://sherlockholmes.com\r\n"
        "KEY;MEDIATYPE=application/pgp-keys:https://sherlockholmes.com/sherlock-holmes.pub.asc\r\n"
        "CALURI;PREF=1:https://sherlockholmes.com/calendar/sherlockholmes\r\n"
        "FBURL;PREF=1:https://sherlockholmes.com/busy/detective\r\n"
        "CALADRURI;PREF=1:mailto:detective@sherlockholmes.com\r\n"
        "END:VCARD\r\n\r\n");
    QByteArray vcardexpected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "ADR;GEO=\"geo:51.523701,0.158500\";LABEL=\"Mr Sherlock Holmes, 221B Baker Stre\r\n"
        " et, London NW1, England, United Kingdom\";TYPE:;;221B Baker Street;London;;\r\n"
        " NW1;United Kingdom\r\n"
        "ANNIVERSARY:19960415\r\n"
        "BDAY:19531015T231000Z\r\n"
        "CALADRURI;PREF=1:mailto:detective@sherlockholmes.com\r\n"
        "CALURI;PREF=1:https://sherlockholmes.com/calendar/sherlockholmes\r\n"
        "CATEGORIES:FICTION,LITERATURE\r\n"
        "EMAIL;PREF=1;TYPE=work:detective@sherlockholmes.com\r\n"
        "EMAIL;TYPE=home:sherlock.holmes@gmail.com\r\n"
        "FBURL;PREF=1:https://sherlockholmes.com/busy/detective\r\n"
        "FN:Sherlock Holmes\r\n"
        "GENDER:M\r\n"
        "GEO:geo:51.523701,0.158500\r\n"
        "IMPP;PREF=1:xmpp:detective@sherlockholmes.com\r\n"
        "KEY;MEDIATYPE=application/pgp-keys:https://sherlockholmes.com/sherlock-holm\r\n"
        " es.pub.asc\r\nKIND:individual\r\n"
        "LANG;PREF=1;TYPE=work:en\r\n"
        "LANG;PREF=2;TYPE=work:fr\r\n"
        "N:Holmes;Sherlock;;Mr;\r\n"
        "NICKNAME:Shirley\r\n"
        "PRODID:-//KADDRESSBOOK//NONSGML Version 1//EN\r\n"
        "REV:20140722T222710Z\r\n"
        "ROLE:Detective\r\n"
        "TEL;TYPE=\"home,voice\";PREF=1;VALUE=uri:tel:+44-555-555-5555;ext=5555\r\n"
        "TEL;TYPE=\"cell,voice\";VALUE=uri:tel:+44-555-555-6666\r\n"
        "TEL;TYPE=\"voice,work\";VALUE=uri:tel:+44-555-555-7777\r\n"
        "TITLE;ALTID=1;LANGUAGE=fr:Patron\r\n"
        "TITLE;ALTID=2;LANGUAGE=en:Boss\r\n"
        "TZ:+00:00\r\n"
        "UID:urn:uuid:b8767877-b4a1-4c70-9acc-505d3819e519\r\n"
        "URL;TYPE=home:https://sherlockholmes.com\r\n"
        "END:VCARD\r\n\r\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);

    const QByteArray result = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    compareBuffers(result, vcardexpected);
}

void ImportExportVCardTest::shouldExportMiscElementVcard4()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "UID:urn:uuid:4fbe8971-0bc3-424c-9c26-36c3e1eff6b1\r\n"
        "FN;PID=1.1:J. Doe\r\n"
        "N:Doe;J.;;;\r\n"
        "EMAIL;PID=1.1:jdoe@example.com\r\n"
        "EMAIL;PID=2.1:boss@example.com\r\n"
        "TEL;PID=1.1;VALUE=uri:tel:+1-555-555-5555\r\n"
        "TEL;PID=2.1;VALUE=uri:tel:+1-666-666-6666\r\n"
        "CLIENTPIDMAP:1;urn:uuid:53e374d9-337e-4727-8803-a1e9c14e0556\r\n"
        "END:VCARD\r\n\r\n");
    QByteArray vcardexpected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "CLIENTPIDMAP:1;urn:uuid:53e374d9-337e-4727-8803-a1e9c14e0556\r\n"
        "EMAIL;PID=1.1:jdoe@example.com\r\n"
        "EMAIL;PID=2.1:boss@example.com\r\n"
        "FN:J. Doe\r\n"
        "N:Doe;J.;;;\r\n"
        "TEL;PID=1.1;VALUE=uri:tel:+1-555-555-5555\r\n"
        "TEL;PID=2.1;VALUE=uri:tel:+1-666-666-6666\r\n"
        "UID:urn:uuid:4fbe8971-0bc3-424c-9c26-36c3e1eff6b1\r\n"
        "END:VCARD\r\n\r\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);

    const QByteArray result = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    compareBuffers(result, vcardexpected);
}

void ImportExportVCardTest::shouldExportMemberElementVcard4()
{
    QByteArray vcarddata(
        "BEGIN:VCARD"
        "VERSION:4.0\r\n"
        "KIND:group\r\n"
        "FN:Funky distribution list\r\n"
        "MEMBER:mailto:subscriber1@example.com\r\n"
        "MEMBER:xmpp:subscriber2@example.com\r\n"
        "MEMBER:sip:subscriber3@example.com\r\n"
        "MEMBER:tel:+1-418-555-5555\r\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\r\n"
        "END:VCARD\r\n");

    QByteArray vcardexpected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "FN:Funky distribution list\r\n"
        "KIND:group\r\n"
        "MEMBER:mailto:subscriber1@example.com\r\n"
        "MEMBER:xmpp:subscriber2@example.com\r\n"
        "MEMBER:sip:subscriber3@example.com\r\n"
        "MEMBER:tel:+1-418-555-5555\r\n"
        "N:;;;;\r\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\r\n"
        "END:VCARD\r\n\r\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);

    const QByteArray result = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    compareBuffers(result, vcardexpected);
}

void ImportExportVCardTest::shouldExportMissingNewlineVcard4()
{
    QByteArray vcarddata(
        "BEGIN:VCARD"
        "VERSION:4.0\r\n"
        "KIND:group\r\n"
        "FN:Funky distribution list\r\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\r\n"
        "END:VCARD");

    QByteArray vcardexpected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "FN:Funky distribution list\r\n"
        "KIND:group\r\n"
        "N:;;;;\r\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\r\n"
        "END:VCARD\r\n\r\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);

    const QByteArray result = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    compareBuffers(result, vcardexpected);
}

// TODO please make this data driven before copy/pasting more methods here...

QTEST_MAIN(ImportExportVCardTest)

#include "moc_importexportvcardtest.cpp"
