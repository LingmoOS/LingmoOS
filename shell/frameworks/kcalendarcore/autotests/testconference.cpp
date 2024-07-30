/*
  This file is part of the kcalcore library.
  SPDX-FileCopyrightText: 2020 Daniel Vrátil <dvratil@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testconference.h"
#include "conference.h"
#include "filestorage.h"
#include "memorycalendar.h"

#include <QDataStream>
#include <QTest>

QTEST_MAIN(ConferenceTest)

using namespace KCalendarCore;

void ConferenceTest::testValidity()
{
    {
        Conference test;
        QVERIFY(test.isNull());
    }

    {
        Conference test(QUrl(QStringLiteral("tel:000326870")), QStringLiteral("Phone call for conference"));
        QVERIFY(!test.isNull());
    }
}

void ConferenceTest::testCompare()
{
    Conference conf1{QUrl{QStringLiteral("tel:123456789")}, QStringLiteral("Conference call"), {QStringLiteral("PHONE")}, QStringLiteral("en")};
    Conference conf2{QUrl{QStringLiteral("xmpp:conference@conference.conference")},
                     QStringLiteral("Conference chat"),
                     {QStringLiteral("CHAT")},
                     QStringLiteral("en")};
    QVERIFY(conf1 != conf2);

    conf2.setUri(QUrl{QStringLiteral("tel:123456789")});
    conf2.setLabel(QStringLiteral("Conference call"));
    conf2.setFeatures({QStringLiteral("PHONE")});
    conf2.setLanguage(QStringLiteral("en"));

    QVERIFY(conf1 == conf2);
}

void ConferenceTest::testAssign()
{
    Conference conf1{QUrl{QStringLiteral("sip:1234-5678@sip.example")}, QStringLiteral("SIP Call")};
    Conference conf2;
    conf2 = conf1;
    QCOMPARE(conf1, conf2);

    conf2.setLanguage(QStringLiteral("en"));
    QVERIFY(!(conf1 == conf2));
}

void ConferenceTest::testCopyConstructor()
{
    Conference conf1{QUrl{QStringLiteral("sip:1234-5678@sip.example")}, QStringLiteral("SIP Call")};
    Conference conf3 {conf1};
    QCOMPARE(conf3, conf1);
}

void ConferenceTest::testDataStream()
{
    Conference conf1;
    conf1.setUri(QUrl{QStringLiteral("tel:000326870")});
    conf1.setLabel(QStringLiteral("Phone conference"));
    conf1.addFeature(QStringLiteral("PHONE"));
    conf1.setLanguage(QStringLiteral("en"));

    QByteArray byteArray;
    QDataStream out_stream(&byteArray, QIODevice::WriteOnly);

    out_stream << conf1;

    QDataStream in_stream(&byteArray, QIODevice::ReadOnly);

    Conference conf2;

    in_stream >> conf2;

    QCOMPARE(conf2.uri(), conf1.uri());
    QCOMPARE(conf2.label(), conf1.label());
    QCOMPARE(conf2.features(), conf1.features());
    QCOMPARE(conf2.language(), conf1.language());
}

void ConferenceTest::testLoading()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    FileStorage store(cal, QLatin1String(ICALTESTDATADIR) + QLatin1String("test_conference.ics"));
    QVERIFY(store.load());
    const auto events = cal->events();
    QCOMPARE(events.size(), 1);

    const auto event = events.at(0);
    const auto conferences = event->conferences();
    QCOMPARE(conferences.size(), 1);
    const auto conference = conferences.at(0);
    QCOMPARE(conference.uri(), QUrl{QStringLiteral("https://corp.kde.example/call/efi83r28")});
    QCOMPARE(conference.features(), (QStringList{QStringLiteral("AUDIO"), QStringLiteral("VIDEO")}));
    QCOMPARE(conference.label(), QStringLiteral("Join NextCloud Talk, password is 12345"));
}

#include "moc_testconference.cpp"
