/*
  This file is part of the KContacts framework.
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "converter/kcontacts/vcardconverter.h"
#include "kcontacts/vcarddrag.h"

#include <QMimeData>
using namespace KContacts;

class VCardDragTest : public QObject
{
    Q_OBJECT

public:
    VCardDragTest();

private:
    Addressee mAddressee1;
    Addressee mAddressee2;

private Q_SLOTS:
    void testPopulate();
    void testCanDecode();
    void testFromMimeData();
};

QTEST_MAIN(VCardDragTest)

VCardDragTest::VCardDragTest()
    : QObject()
{
    mAddressee1.setGivenName(QStringLiteral("Kevin"));
    mAddressee1.setFamilyName(QStringLiteral("Krammer"));
    mAddressee1.addEmail(QStringLiteral("kevin.krammer@gmx.at"));

    mAddressee2.setGivenName(QStringLiteral("Tobias"));
    mAddressee2.setFamilyName(QStringLiteral("König"));
    mAddressee2.addEmail(QStringLiteral("tokoe@kde.org"));
}

void VCardDragTest::testPopulate()
{
    VCardConverter converter;
    const QByteArray vcard = converter.createVCard(mAddressee1);
    QVERIFY(!vcard.isEmpty());

    QMimeData *data = new QMimeData();
    bool result = VCardDrag::populateMimeData(data, vcard);
    QVERIFY(result);
    QVERIFY(data->hasFormat(KContacts::Addressee::mimeType()));
    QCOMPARE(data->data(KContacts::Addressee::mimeType()), vcard);
    delete data;

    data = new QMimeData();
    result = VCardDrag::populateMimeData(data, Addressee::List() << mAddressee1);
    QVERIFY(result);
    QVERIFY(data->hasFormat(KContacts::Addressee::mimeType()));
    QCOMPARE(data->data(KContacts::Addressee::mimeType()), vcard);
    delete data;

    const QByteArray vcards = converter.createVCards(Addressee::List() << mAddressee1 << mAddressee2);
    data = new QMimeData();
    result = VCardDrag::populateMimeData(data, Addressee::List() << mAddressee1 << mAddressee2);
    QVERIFY(result);
    QVERIFY(data->hasFormat(KContacts::Addressee::mimeType()));
    QCOMPARE(data->data(KContacts::Addressee::mimeType()), vcards);
    delete data;

    data = new QMimeData();
    result = VCardDrag::populateMimeData(data, Addressee::List());
    QVERIFY(!result);
    QVERIFY(!data->hasFormat(KContacts::Addressee::mimeType()));
    delete data;
}

void VCardDragTest::testCanDecode()
{
    VCardConverter converter;
    const QByteArray vcard = converter.createVCard(mAddressee1);
    QVERIFY(!vcard.isEmpty());

    QMimeData *data = new QMimeData();
    data->setData(Addressee::mimeType(), vcard);
    QVERIFY(VCardDrag::canDecode(data));
    delete data;

    data = new QMimeData();
    data->setData(QStringLiteral("text/x-vcard"), vcard);
    QVERIFY(VCardDrag::canDecode(data));
    delete data;

    data = new QMimeData();
    data->setData(QStringLiteral("application/octetstream"), vcard);
    QVERIFY(!VCardDrag::canDecode(data));
    delete data;
}

void VCardDragTest::testFromMimeData()
{
    VCardConverter converter;
    const QByteArray vcard = converter.createVCard(mAddressee1);
    QVERIFY(!vcard.isEmpty());

    QByteArray content;

    QMimeData *data = new QMimeData();
    data->setData(Addressee::mimeType(), vcard);
    bool result = VCardDrag::fromMimeData(data, content);
    QVERIFY(result);
    QCOMPARE(content, vcard);
    delete data;

    data = new QMimeData();
    data->setData(QStringLiteral("text/x-vcard"), vcard);
    result = VCardDrag::fromMimeData(data, content);
    QVERIFY(result);
    QCOMPARE(content, vcard);
    delete data;

    data = new QMimeData();
    data->setData(QStringLiteral("application/octetstream"), vcard);
    result = VCardDrag::fromMimeData(data, content);
    QVERIFY(!result);
    delete data;

    const QByteArray vcards = converter.createVCards(Addressee::List() << mAddressee1 << mAddressee2);
    QVERIFY(!vcards.isEmpty());

    Addressee::List addresseeList;

    data = new QMimeData();
    data->setData(Addressee::mimeType(), vcards);
    result = VCardDrag::fromMimeData(data, addresseeList);
    QVERIFY(result);
    QCOMPARE(addresseeList.count(), 2);
    QCOMPARE(addresseeList[0], mAddressee1);
    QCOMPARE(addresseeList[1], mAddressee2);
    delete data;

    data = new QMimeData();
    data->setData(QStringLiteral("text/x-vcard"), vcards);
    result = VCardDrag::fromMimeData(data, addresseeList);
    QVERIFY(result);
    QCOMPARE(addresseeList.count(), 2);
    QCOMPARE(addresseeList[0], mAddressee1);
    QCOMPARE(addresseeList[1], mAddressee2);
    delete data;

    data = new QMimeData();
    data->setData(QStringLiteral("application/octetstream"), vcards);
    result = VCardDrag::fromMimeData(data, addresseeList);
    QVERIFY(!result);
    delete data;

    data = new QMimeData();
    data->setData(Addressee::mimeType(), QByteArray());
    result = VCardDrag::fromMimeData(data, addresseeList);
    QVERIFY(!result);
    delete data;
}

#include "vcarddragtest.moc"
