/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "event.h"
#include "icalformat.h"
#include "todo.h"

#include <QDebug>
#include <QTest>

using namespace KCalendarCore;

class IncidencesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testClone()
    {
        ICalFormat f;

        Event::Ptr event1 = Event::Ptr(new Event);
        event1->setSummary(QStringLiteral("Test Event"));
        event1->recurrence()->setDaily(2);
        event1->recurrence()->setDuration(3);
        event1->setSchedulingID(QStringLiteral("foo"));
        QString eventString1 = f.toString(event1.staticCast<Incidence>());

        Incidence::Ptr event2 = Incidence::Ptr(event1->clone());
        QCOMPARE(event1->uid(), event2->uid());
        QCOMPARE(event1->schedulingID(), event2->schedulingID());

        QString eventString2 = f.toString(event2.staticCast<Incidence>());
        QCOMPARE(eventString1, eventString2);

        Todo::Ptr todo1 = Todo::Ptr(new Todo);
        todo1->setSummary(QStringLiteral("Test todo"));
        QString todoString1 = f.toString(todo1.staticCast<Incidence>());

        Incidence::Ptr todo2 = Incidence::Ptr(todo1->clone());
        QString todoString2 = f.toString(todo2);
        QCOMPARE(todoString1, todoString2);
    }
};

QTEST_APPLESS_MAIN(IncidencesTest)

#include "incidencestest.moc"
