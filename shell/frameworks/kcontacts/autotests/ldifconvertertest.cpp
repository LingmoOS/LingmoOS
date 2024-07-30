/*
    This file is part of libkabc.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ldifconvertertest.h"
#include "converter/ldifconverter.h"
#include <QTest>
using namespace KContacts;

LDifConverterTest::LDifConverterTest(QObject *parent)
    : QObject(parent)
{
}

LDifConverterTest::~LDifConverterTest()
{
}

void LDifConverterTest::shouldImportEmail()
{
    QString str = QStringLiteral(
        "dn: cn=laurent,mail=foo@kde.org\n"
        "sn: laurent\n"
        "cn: laurent\n"
        "uid: d1d5cdd4-7d5d-484b-828d-58864d8efe74\n"
        "mail: foo@kde.org\n"
        "objectclass: top_n"
        "objectclass: person\n"
        "objectclass: organizationalPerson");
    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).emails().count(), 1);
    QCOMPARE(lst.at(0).emails().at(0), QStringLiteral("foo@kde.org"));
    QCOMPARE(contactGroup.count(), 0);
}

void LDifConverterTest::shouldImportMultiEmails()
{
    QString str = QStringLiteral(
        "dn: cn=laurent,mail=foo@kde.org\n"
        "sn: laurent\n"
        "cn: laurent\n"
        "uid: d1d5cdd4-7d5d-484b-828d-58864d8efe74\n"
        "mail: foo@kde.org\n"
        "mail: foo2@kde.org\n"
        "objectclass: top_n"
        "objectclass: person\n"
        "objectclass: organizationalPerson");
    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).emails().count(), 2);
    QCOMPARE(lst.at(0).emails().at(0), QStringLiteral("foo@kde.org"));
    QCOMPARE(lst.at(0).emails().at(1), QStringLiteral("foo2@kde.org"));
    QCOMPARE(contactGroup.count(), 0);
}

void LDifConverterTest::shouldImportStandardBirthday()
{
    QString str = QStringLiteral(
        "dn: cn=laurent,mail=foo@kde.org\n"
        "sn: laurent\n"
        "cn: laurent\n"
        "uid: d1d5cdd4-7d5d-484b-828d-58864d8efe74\n"
        "birthyear: 2015\n"
        "birthmonth: 3\n"
        "birthday: 19\n"
        "mail: foo@kde.org\n"
        "objectclass: top_n"
        "objectclass: person\n"
        "objectclass: organizationalPerson");
    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 1);
    QVERIFY(lst.at(0).birthday().date().isValid());
    QCOMPARE(lst.at(0).birthday().date(), QDate(2015, 3, 19));
    QVERIFY(!lst.at(0).birthdayHasTime());
    QCOMPARE(contactGroup.count(), 0);
}

void LDifConverterTest::shouldImportStandardBirthdayWithoutYear()
{
    QString str = QStringLiteral(
        "dn: cn=laurent,mail=foo@kde.org\n"
        "sn: laurent\n"
        "cn: laurent\n"
        "uid: d1d5cdd4-7d5d-484b-828d-58864d8efe74\n"
        "birthmonth: 3\n"
        "birthday: 19\n"
        "mail: foo@kde.org\n"
        "objectclass: top_n"
        "objectclass: person\n"
        "objectclass: organizationalPerson");
    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 1);
    QVERIFY(lst.at(0).birthday().date().isValid());
    QCOMPARE(lst.at(0).birthday().date(), QDate(-1, 3, 19));
    QVERIFY(!lst.at(0).birthdayHasTime());
    QCOMPARE(contactGroup.count(), 0);
}

void LDifConverterTest::shouldImportTheBatsBirthday()
{
    QString str = QStringLiteral(
        "dn: cn=laurent,mail=foo@kde.org\n"
        "sn: laurent\n"
        "cn: laurent\n"
        "uid: d1d5cdd4-7d5d-484b-828d-58864d8efe74\n"
        "xbatBirthday: 20150319\n"
        "mail: foo@kde.org\n"
        "objectclass: top_n"
        "objectclass: person\n"
        "objectclass: organizationalPerson");
    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 1);
    QVERIFY(lst.at(0).birthday().date().isValid());
    QCOMPARE(lst.at(0).birthday().date(), QDate(2015, 3, 19));
    QVERIFY(!lst.at(0).birthdayHasTime());
    QCOMPARE(contactGroup.count(), 0);
}

void LDifConverterTest::shouldImportTheBatsEmails()
{
    QString str = QStringLiteral(
        "dn: cn=laurent,mail=foo@kde.org\n"
        "sn: laurent\n"
        "cn: laurent\n"
        "uid: d1d5cdd4-7d5d-484b-828d-58864d8efe74\n"
        "mail: foo@kde.org\n"
        "othermailbox: foo2@kde.org\n"
        "othermailbox: foo3@kde.org\n"
        "objectclass: top_n"
        "objectclass: person\n"
        "objectclass: organizationalPerson");
    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).emails().count(), 3);
    QCOMPARE(lst.at(0).emails().at(0), QStringLiteral("foo@kde.org"));
    QCOMPARE(lst.at(0).emails().at(1), QStringLiteral("foo2@kde.org"));
    QCOMPARE(lst.at(0).emails().at(2), QStringLiteral("foo3@kde.org"));
    QCOMPARE(contactGroup.count(), 0);
}

void LDifConverterTest::shouldImportTitle()
{
    QString str = QStringLiteral(
        "dn: cn=laurent,mail=foo@kde.org\n"
        "sn: laurent\n"
        "cn: laurent\n"
        "uid: d1d5cdd4-7d5d-484b-828d-58864d8efe74\n"
        "title: foo\n"
        "mail: foo@kde.org\n"
        "objectclass: top_n"
        "objectclass: person\n"
        "objectclass: organizationalPerson");
    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).title(), QStringLiteral("foo"));
    QCOMPARE(contactGroup.count(), 0);
}

void LDifConverterTest::shouldImportWorkStreet()
{
    QString str = QStringLiteral(
        "dn: cn=laurent,mail=foo@kde.org\n"
        "sn: laurent\n"
        "cn: laurent\n"
        "uid: d1d5cdd4-7d5d-484b-828d-58864d8efe74\n"
        "title: foo\n"
        "mail: foo@kde.org\n"
        "street: work address\n"
        "mozillaWorkStreet2: work address next\n"
        "objectclass: top_n"
        "objectclass: person\n"
        "objectclass: organizationalPerson");
    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).address(Address::Work).street(), QStringLiteral("work address\nwork address next"));
    QCOMPARE(contactGroup.count(), 0);
}

void LDifConverterTest::shouldImportContactGroup()
{
    QString str = QStringLiteral(
        "dn: cn=test,mail=\n"
        "cn: test\n"
        "modifyTimeStamp: 20080526T234914Z\n"
        "display-name: Test\n"
        "objectclass: top\n"
        "objectclass: groupOfNames\n"
        "member: cn=Jim Doe,mail=jim.doe@foobar.com\n"
        "member: cn=Jane Doe,mail=jane.doe@foobar.com\n"
        "member: cn=John Doe,mail=john.doe@foobar.com\n");
    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 0);
    QCOMPARE(contactGroup.count(), 1);
    ContactGroup first = contactGroup.first();
    QCOMPARE(first.name(), QStringLiteral("Test"));
    QCOMPARE((int)first.count(), 3);
}

void LDifConverterTest::shouldImportMultiEntries()
{
    QString str = QStringLiteral(
        "dn: cn=test1,mail=test1@test.test\n"
        "sn: test1\n"
        "cn: test1\n"
        "uid: jpgdf2NrLQ\n"
        "mail: test1@test.test\n"
        "modifytimestamp: 20121219T140848Z\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "\n"
        "dn: cn=test2,mail=test2@test.test\n"
        "sn: test2\n"
        "cn: test2\n"
        "uid: ow2mwdUb6A\n"
        "mail: test2@test.test\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n");
    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 2);
    QCOMPARE(contactGroup.count(), 0);
    QCOMPARE(lst.at(0).emails().at(0), QStringLiteral("test1@test.test"));
    QCOMPARE(lst.at(1).emails().at(0), QStringLiteral("test2@test.test"));
}

void LDifConverterTest::shouldImportGroupAndAddress()
{
    QString str = QStringLiteral(
        "dn: cn=laurent,mail=foo@kde.org\n"
        "sn: laurent\n"
        "cn: laurent\n"
        "uid: d1d5cdd4-7d5d-484b-828d-58864d8efe74\n"
        "title: foo\n"
        "mail: foo@kde.org\n"
        "street: work address\n"
        "mozillaWorkStreet2: work address next\n"
        "objectclass: top_n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "\n"
        "dn: cn=test,mail=\n"
        "cn: test\n"
        "modifyTimeStamp: 20080526T234914Z\n"
        "display-name: Test\n"
        "objectclass: top\n"
        "objectclass: groupOfNames\n"
        "member: cn=Jim Doe,mail=jim.doe@foobar.com\n"
        "member: cn=Jane Doe,mail=jane.doe@foobar.com\n"
        "member: cn=John Doe,mail=john.doe@foobar.com\n");

    AddresseeList lst;
    ContactGroup::List contactGroup;
    bool result = LDIFConverter::LDIFToAddressee(str, lst, contactGroup);
    QVERIFY(result);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(contactGroup.count(), 1);
}

void LDifConverterTest::shouldExportEmail()
{
    AddresseeList lst;
    ContactGroup::List contactGroup;
    Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    lst << addr;
    QString str;
    bool result = LDIFConverter::addresseeAndContactGroupToLDIF(lst, contactGroup, str);
    QVERIFY(result);
    QString expected = QStringLiteral(
        "dn: cn=,mail=foo@kde.org\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "uid: testuid\n"
        "mail: foo@kde.org\n\n");

    QCOMPARE(str, expected);
}

void LDifConverterTest::shouldExportMultiEmails()
{
    AddresseeList lst;
    ContactGroup::List contactGroup;
    Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org") << QStringLiteral("foo2@kde.org") << QStringLiteral("foo3@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    lst << addr;
    QString str;
    bool result = LDIFConverter::addresseeAndContactGroupToLDIF(lst, contactGroup, str);
    QVERIFY(result);
    QString expected = QStringLiteral(
        "dn: cn=,mail=foo@kde.org\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "uid: testuid\n"
        "mail: foo@kde.org\n"
        "mozillasecondemail: foo2@kde.org\n"
        "othermailbox: foo3@kde.org\n"
        "\n");

    QCOMPARE(str, expected);
}

void LDifConverterTest::shouldExportBirthday()
{
    AddresseeList lst;
    ContactGroup::List contactGroup;
    Addressee addr;
    QDate date(2015, 3, 3);
    addr.setBirthday(date);
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    lst << addr;
    QString str;
    bool result = LDIFConverter::addresseeAndContactGroupToLDIF(lst, contactGroup, str);
    QVERIFY(result);
    QString expected = QStringLiteral(
        "dn: cn=,mail=foo@kde.org\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "uid: testuid\n"
        "mail: foo@kde.org\n"
        "birthyear: 2015\n"
        "birthmonth: 3\n"
        "birthday: 3\n"
        "\n");

    QCOMPARE(str, expected);
}

void LDifConverterTest::shouldExportBirthdayWithoutYear()
{
    AddresseeList lst;
    ContactGroup::List contactGroup;
    Addressee addr;
    QDate date(-1, 3, 3);
    addr.setBirthday(date);
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    lst << addr;
    QString str;
    bool result = LDIFConverter::addresseeAndContactGroupToLDIF(lst, contactGroup, str);
    QVERIFY(result);
    QString expected = QStringLiteral(
        "dn: cn=,mail=foo@kde.org\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "uid: testuid\n"
        "mail: foo@kde.org\n"
        "birthmonth: 3\n"
        "birthday: 3\n"
        "\n");

    QCOMPARE(str, expected);
}

void LDifConverterTest::shouldExportTitle()
{
    AddresseeList lst;
    ContactGroup::List contactGroup;
    Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setTitle(QStringLiteral("foo"));
    addr.setUid(QStringLiteral("testuid"));
    lst << addr;
    QString str;
    bool result = LDIFConverter::addresseeAndContactGroupToLDIF(lst, contactGroup, str);
    QVERIFY(result);
    QString expected = QStringLiteral(
        "dn: cn=,mail=foo@kde.org\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "uid: testuid\n"
        "mail: foo@kde.org\n"
        "title: foo\n"
        "\n");

    QCOMPARE(str, expected);
}

void LDifConverterTest::shouldExportMultiEntries()
{
    AddresseeList lst;
    ContactGroup::List contactGroup;
    Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setTitle(QStringLiteral("foo"));
    addr.setUid(QStringLiteral("testuid"));
    lst << addr;

    Addressee addr2;
    QDate date(2015, 3, 3);
    addr2.setBirthday(date);
    addr2.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr2.setUid(QStringLiteral("testuid"));
    lst << addr2;

    QString str;
    bool result = LDIFConverter::addresseeAndContactGroupToLDIF(lst, contactGroup, str);
    QVERIFY(result);

    const QString expected = QStringLiteral(
        "dn: cn=,mail=foo@kde.org\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "uid: testuid\n"
        "mail: foo@kde.org\n"
        "title: foo\n"
        "\n"
        "dn: cn=,mail=foo@kde.org\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "uid: testuid\n"
        "mail: foo@kde.org\n"
        "birthyear: 2015\n"
        "birthmonth: 3\n"
        "birthday: 3\n"
        "\n");
    QCOMPARE(str, expected);
}

void LDifConverterTest::shouldExportGroup()
{
    AddresseeList lst;
    ContactGroup::List contactGroup;
    ContactGroup grp;
    ContactGroup::Data data;
    data.setEmail(QStringLiteral("foo@kde.org"));
    data.setName(QStringLiteral("foo"));
    grp.append(data);
    ContactGroup::Data data2;
    data2.setEmail(QStringLiteral("foo2@kde.org"));
    data2.setName(QStringLiteral("foo2"));
    grp.append(data2);
    contactGroup.append(grp);

    QString str;
    bool result = LDIFConverter::addresseeAndContactGroupToLDIF(lst, contactGroup, str);
    QVERIFY(result);

    const QString expected = QStringLiteral(
        "objectclass: top\n"
        "objectclass: groupOfNames\n"
        "member: cn=foo,mail=foo@kde.org\n"
        "member: cn=foo2,mail=foo2@kde.org\n"
        "\n");
    QCOMPARE(str, expected);
}

void LDifConverterTest::shouldExportWorkStreet()
{
    AddresseeList lst;
    Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    Address address(Address::Work);
    address.setStreet(QStringLiteral("work address"));
    address.setPostalCode(QStringLiteral("postal"));
    addr.insertAddress(address);
    lst << addr;
    ContactGroup::List contactGroup;

    QString str;
    bool result = LDIFConverter::addresseeAndContactGroupToLDIF(lst, contactGroup, str);
    QVERIFY(result);

    const QString expected = QStringLiteral(
        "dn: cn=,mail=foo@kde.org\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "uid: testuid\n"
        "mail: foo@kde.org\n"
        "postalcode: postal\n"
        "streetaddress: work address\n"
        "street: work address\n"
        "\n");
    QCOMPARE(str, expected);
}

void LDifConverterTest::shouldExportFullName()
{
    AddresseeList lst;
    Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    addr.setName(QStringLiteral("name"));
    addr.setNickName(QStringLiteral("nickname"));
    addr.setFamilyName(QStringLiteral("familyname"));
    lst << addr;
    ContactGroup::List contactGroup;

    QString str;
    bool result = LDIFConverter::addresseeAndContactGroupToLDIF(lst, contactGroup, str);
    QVERIFY(result);

    const QString expected = QStringLiteral(
        "dn: cn=,mail=foo@kde.org\n"
        "objectclass: top\n"
        "objectclass: person\n"
        "objectclass: organizationalPerson\n"
        "sn: familyname\n"
        "uid: testuid\n"
        "nickname: nickname\n"
        "xmozillanickname: nickname\n"
        "mozillanickname: nickname\n"
        "mail: foo@kde.org\n"
        "\n");

    QCOMPARE(str, expected);
}

void LDifConverterTest::testGarbage()
{
    AddresseeList lst;
    ContactGroup::List contactGroup;
    QString str;
    bool result;

    result = LDIFConverter::addresseeAndContactGroupToLDIF(lst, contactGroup, str);
    QVERIFY(!result);
    result = LDIFConverter::contactGroupToLDIF(contactGroup, str);
    QVERIFY(!result);
    result = LDIFConverter::addresseeToLDIF(lst, str);
    QVERIFY(!result);

    Addressee addr;
    result = LDIFConverter::addresseeToLDIF(addr, str);
    QVERIFY(!result);
}

QTEST_MAIN(LDifConverterTest)

#include "moc_ldifconvertertest.cpp"
