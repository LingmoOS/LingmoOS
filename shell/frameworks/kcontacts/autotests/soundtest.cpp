/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "soundtest.h"
#include "addressee.h"
#include "kcontacts/sound.h"
#include "vcardtool_p.h"
#include <QTest>

QTEST_MAIN(SoundTest)

static QByteArray testData()
{
    static QByteArray data;

    if (data.isNull()) {
        for (int i = 0; i < 20; ++i) {
            data.append(char(i));
        }
    }

    return data;
}

void SoundTest::emptyTest()
{
    KContacts::Sound sound;

    QVERIFY(sound.isEmpty());
}

void SoundTest::storeTestIntern()
{
    KContacts::Sound sound;

    sound.setData(testData());

    QVERIFY(sound.isIntern() == true);
    QVERIFY(sound.data() == testData());
}

void SoundTest::storeTestExtern()
{
    KContacts::Sound sound;

    sound.setUrl(QLatin1String("http://myhomepage.com/sound.wav"));

    QVERIFY(sound.isIntern() == false);
    QVERIFY(sound.url() == QLatin1String("http://myhomepage.com/sound.wav"));
}

void SoundTest::equalsTestIntern()
{
    KContacts::Sound sound1;
    KContacts::Sound sound2;

    sound1.setData(testData());
    sound2.setData(testData());

    QVERIFY(sound1 == sound2);
}

void SoundTest::equalsTestExtern()
{
    KContacts::Sound sound1;
    KContacts::Sound sound2;

    sound1.setUrl(QStringLiteral("http://myhomepage.com/sound.wav"));
    sound2.setUrl(QStringLiteral("http://myhomepage.com/sound.wav"));

    QVERIFY(sound1 == sound2);
}

void SoundTest::differsTest()
{
    KContacts::Sound sound1;
    KContacts::Sound sound2;

    sound1.setUrl(QStringLiteral("http://myhomepage.com/sound.wav"));
    sound2.setData(testData());

    QVERIFY(sound1 != sound2);
}

void SoundTest::assignmentTestIntern()
{
    KContacts::Sound sound1;
    KContacts::Sound sound2;

    sound1.setData(testData());

    sound2 = sound1;

    QVERIFY(sound1 == sound2);
}

void SoundTest::assignmentTestExtern()
{
    KContacts::Sound sound1;
    KContacts::Sound sound2;

    sound1.setUrl(QStringLiteral("http://myhomepage.com/sound.wav"));

    sound2 = sound1;

    QVERIFY(sound1 == sound2);
}

void SoundTest::serializeTest()
{
    KContacts::Sound sound1;
    KContacts::Sound sound2;

    sound1.setUrl(QStringLiteral("http://myhomepage.com/sound.wav"));
    sound1.setData(testData());

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << sound1;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> sound2;

    QVERIFY(sound1 == sound2);
}

void SoundTest::shouldParseSource()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "SOUND;VALUE=URI:http://myhomepage.com/sound.wav\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QVERIFY(!lst.at(0).sound().isEmpty());
    KContacts::Sound sound = lst.at(0).sound();
    QVERIFY(!sound.isIntern());
    QCOMPARE(sound.url(), QStringLiteral("http://myhomepage.com/sound.wav"));
}

void SoundTest::shouldGenerateVCard4WithData()
{
    KContacts::Addressee::List lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Sound sound1;

    sound1.setUrl(QStringLiteral("http://myhomepage.com/sound.wav"));
    sound1.setData(testData());
    addr.setSound(sound1);

    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected;
    expected = QByteArray(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "SOUND;ENCODING=b:AAECAwQFBgcICQoLDA0ODxAREhM=\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void SoundTest::shouldGenerateVCard4WithUrl()
{
    KContacts::Addressee::List lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Sound sound1;

    sound1.setUrl(QStringLiteral("http://myhomepage.com/sound.wav"));
    addr.setSound(sound1);

    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected;
    expected = QByteArray(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "SOUND;VALUE=URI:http://myhomepage.com/sound.wav\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void SoundTest::shouldGenerateVCard3WithData()
{
    KContacts::Addressee::List lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Sound sound1;

    sound1.setUrl(QStringLiteral("http://myhomepage.com/sound.wav"));
    sound1.setData(testData());
    addr.setSound(sound1);

    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected;
    expected = QByteArray(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "SOUND;ENCODING=b:AAECAwQFBgcICQoLDA0ODxAREhM=\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void SoundTest::shouldGenerateVCard3WithUrl()
{
    KContacts::Addressee::List lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Sound sound1;

    sound1.setUrl(QStringLiteral("http://myhomepage.com/sound.wav"));
    addr.setSound(sound1);

    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected;
    expected = QByteArray(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "SOUND;VALUE=URI:http://myhomepage.com/sound.wav\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

#include "moc_soundtest.cpp"
