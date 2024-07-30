/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "event.h"
#include "freebusy.h"
#include "icalformat.h"
#include "memorycalendar.h"

#include <iostream>

#include <QDate>
#include <QTimeZone>

using namespace KCalendarCore;
using namespace std;

int main(int argc, char **argv)
{
    ICalFormat f;

    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));

    Event::Ptr event1 = Event::Ptr(new Event);
    event1->setSummary(QStringLiteral("A"));
    event1->setDtStart(QDateTime(QDate(2006, 1, 1), QTime(12, 0, 0)));
    event1->setDtEnd(QDateTime(QDate(2006, 1, 1), QTime(13, 0, 0)));
    event1->setAllDay(false);
    event1->recurrence()->setDaily(1);
    event1->recurrence()->setEndDateTime(QDateTime(QDate(2006, 1, 3), QTime(13, 0, 0)));
    cout << f.toICalString(event1).toLocal8Bit().data() << endl;
    cal->addEvent(event1);

    Event::Ptr event2 = Event::Ptr(new Event);
    event2->setSummary(QStringLiteral("B"));
    event2->setDtStart(QDateTime(QDate(2006, 1, 1), QTime(13, 0, 0)));
    event2->setDtEnd(QDateTime(QDate(2006, 1, 1), QTime(14, 0, 0)));
    event2->setAllDay(false);
    event2->recurrence()->setDaily(1);
    event2->recurrence()->setEndDateTime(QDateTime(QDate(2006, 1, 4), QTime(13, 0, 0)));
    cout << f.toICalString(event2).toLocal8Bit().data() << endl;
    cal->addEvent(event2);

    QDateTime start(QDate(2006, 1, 2), QTime(0, 0, 0));
    QDateTime end(QDate(2006, 1, 3), QTime(0, 0, 0));

    FreeBusy::Ptr freebusy = FreeBusy::Ptr(new FreeBusy(cal->rawEvents(start.date(), end.date()), start, end));
    QString result = f.createScheduleMessage(freebusy, iTIPPublish);
    cout << result.toLocal8Bit().data() << endl;

    return 0;
}
