/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006-2009 Allen Winter <winter@kde.org>
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testperson.h"
#include "person.h"

#include <QDataStream>
#include <QTest>

QTEST_MAIN(PersonTest)

using namespace KCalendarCore;

void PersonTest::testValidity()
{
    Person person(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    QVERIFY(person.name() == QLatin1String("fred"));
}

void PersonTest::testCompare()
{
    Person person1(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    Person person2(QStringLiteral("wilma"), QStringLiteral("wilma@flintstone.com"));
    Person person3 = Person::fromFullName(QStringLiteral("fred <fred@flintstone.com>"));
    Person person1copy {person1}; // test copy constructor
    Person person1assign;
    person1assign = person1; // test operator=

    QVERIFY(!(person1 == person2));
    QVERIFY(person1 == person3);
    QVERIFY(person1 == person1copy);
    QVERIFY(person1 == person1assign);
    QVERIFY(person1.name() == QLatin1String("fred"));
    QVERIFY(person2.email() == QLatin1String("wilma@flintstone.com"));
    QVERIFY(person3.name() == QLatin1String("fred"));
    QVERIFY(person3.email() == QLatin1String("fred@flintstone.com"));
}

void PersonTest::testStringify()
{
    Person person1(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    Person person2(QStringLiteral("wilma"), QStringLiteral("wilma@flintstone.com"));
    QVERIFY(person1.fullName() == QLatin1String("fred <fred@flintstone.com>"));
    QVERIFY(person2.fullName() == QLatin1String("wilma <wilma@flintstone.com>"));

    person1.setName(QLatin1String(""));
    QVERIFY(person1.fullName() == QLatin1String("fred@flintstone.com"));
    person1.setEmail(QString());
    QVERIFY(person1.fullName().isEmpty());
}

void PersonTest::testDataStreamIn()
{
    Person person1(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));

    QByteArray byteArray;
    QDataStream out_stream(&byteArray, QIODevice::WriteOnly);

    out_stream << person1;

    QDataStream in_stream(&byteArray, QIODevice::ReadOnly);

    QString name;
    QString email;
    int count;

    in_stream >> name;
    QVERIFY(name == QLatin1String("fred"));

    in_stream >> email;
    QVERIFY(email == QLatin1String("fred@flintstone.com"));

    in_stream >> count;
    QCOMPARE(count, 0);
}

void PersonTest::testDataStreamOut()
{
    Person person1(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));

    QByteArray byteArray;
    QDataStream out_stream(&byteArray, QIODevice::WriteOnly);

    out_stream << person1;

    QDataStream in_stream(&byteArray, QIODevice::ReadOnly);
    Person person2;

    in_stream >> person2;

    QVERIFY(person2.name() == person1.name());
    QVERIFY(person2.email() == person1.email());
}

#include "moc_testperson.cpp"
