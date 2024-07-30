/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2007-2008 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "testjournal.h"
#include "journal.h"

#include <QTest>
QTEST_MAIN(JournalTest)

using namespace KCalendarCore;

void JournalTest::testValidity()
{
    QDate dt = QDate::currentDate();
    Journal journal;
    journal.setDtStart(QDateTime(dt, {}));
    journal.setAllDay(true);
    journal.setSummary(QStringLiteral("Journal Summary"));
    journal.setDescription(QStringLiteral("This is a description of my journal"));
    journal.setLocation(QStringLiteral("the place"));
    QCOMPARE(journal.typeStr(), QByteArray("Journal"));
    QVERIFY(journal.summary() == QLatin1String("Journal Summary"));
    QVERIFY(journal.location() == QLatin1String("the place"));
}

void JournalTest::testCompare()
{
    QDate dt = QDate::currentDate();
    Journal journal1;
    journal1.setDtStart(QDateTime(dt, {}));
    journal1.setAllDay(true);
    journal1.setSummary(QStringLiteral("Journal Summary"));
    journal1.setDescription(QStringLiteral("This is a description of my journal"));
    journal1.setLocation(QStringLiteral("the place"));

    Journal journal2;
    journal2.setDtStart(QDateTime(dt, {}).addDays(1));
    journal2.setAllDay(true);
    journal2.setSummary(QStringLiteral("Journal2 Summary"));
    journal2.setDescription(QStringLiteral("This is a description of another journal"));
    journal2.setLocation(QStringLiteral("the other place"));

    QVERIFY(!(journal1 == journal2));
    QVERIFY(journal2.summary() == QLatin1String("Journal2 Summary"));
}

void JournalTest::testClone()
{
    QDate dt = QDate::currentDate();
    Journal journal1;
    journal1.setDtStart(QDateTime(dt, {}));
    journal1.setAllDay(true);
    journal1.setSummary(QStringLiteral("Journal1 Summary"));
    journal1.setDescription(QStringLiteral("This is a description of the first journal"));
    journal1.setLocation(QStringLiteral("the place"));

    Journal *journal2 = journal1.clone();
    QVERIFY(journal1.summary() == journal2->summary());
    QVERIFY(journal1.dtStart() == journal2->dtStart());
    QVERIFY(journal1.description() == journal2->description());
    QVERIFY(journal1.location() == journal2->location());
    delete journal2;
}

void JournalTest::testRich()
{
    QDate dt = QDate::currentDate();
    Journal journal1;
    journal1.setDtStart(QDateTime(dt, {}));
    journal1.setAllDay(true);
    journal1.setSummary(QStringLiteral("<html><b><i>Journal1 Summary</i></b></html>"), true);
    journal1.setDescription(QStringLiteral("<html>This is a of the <b>first</b> journal</html>"), true);
    journal1.setLocation(QStringLiteral("<qt><h1>the place</h1></qt>"), true);
    QVERIFY(journal1.summaryIsRich());
    QVERIFY(journal1.descriptionIsRich());
    QVERIFY(journal1.locationIsRich());
}

void JournalTest::testCopyConstructor()
{
    QDate dt = QDate::currentDate();
    Journal journal1;
    journal1.setDtStart(QDateTime(dt, {}));
    journal1.setAllDay(true);
    journal1.setSummary(QStringLiteral("Journal1 Summary"));
    journal1.setDescription(QStringLiteral("This is a description of the first journal"), true);
    journal1.setLocation(QStringLiteral("the place"));

    Journal journal2 {journal1};
    QVERIFY(journal2.descriptionIsRich());
    QVERIFY(journal1 == journal2);
}

void JournalTest::testAssign()
{
    QDate dt = QDate::currentDate();
    Journal journal1;
    journal1.setDtStart(QDateTime(dt, {}));
    journal1.setAllDay(true);
    journal1.setSummary(QStringLiteral("Journal1 Summary"));
    journal1.setDescription(QStringLiteral("This is a description of the first journal"), true);
    journal1.setLocation(QStringLiteral("the place"));

    Journal journal2;
    IncidenceBase *ib2 = & journal2;
    *ib2 = journal1;     // Use IncidenceBase's virtual assignment.
    QVERIFY(journal2.descriptionIsRich());
    QVERIFY(journal1 == journal2);
}

void JournalTest::testSerializer_data()
{
    QTest::addColumn<KCalendarCore::Journal::Ptr>("journal");

    Journal::Ptr journal1 = Journal::Ptr(new Journal());

    QTest::newRow("journal") << journal1;
}

void JournalTest::testSerializer()
{
    QFETCH(KCalendarCore::Journal::Ptr, journal);
    IncidenceBase::Ptr incidenceBase = journal.staticCast<KCalendarCore::IncidenceBase>();

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << incidenceBase;

    Journal::Ptr journal2 = Journal::Ptr(new Journal());
    IncidenceBase::Ptr incidenceBase2 = journal2.staticCast<KCalendarCore::IncidenceBase>();
    QVERIFY(*journal != *journal2);
    QDataStream stream2(&array, QIODevice::ReadOnly);
    stream2 >> incidenceBase2;
    QVERIFY(*journal == *journal2);
}

#include "moc_testjournal.cpp"
