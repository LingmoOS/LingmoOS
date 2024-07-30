/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "event.h"
#include "icalformat.h"

#include <QDebug>
#include <QTest>
#include <qtest.h>

using namespace KCalendarCore;

class TestToString : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testToString()
    {
        Event::Ptr ev = Event::Ptr(new Event);
        ev->setSummary(QStringLiteral("Griazi"));
        ICalFormat iformat;
        QString icalstr = iformat.toICalString(ev);

        Incidence::Ptr ev2 = iformat.fromString(icalstr);
        QVERIFY(ev2);
        QCOMPARE(ev2->summary(), QLatin1String("Griazi"));
    }
};

QTEST_APPLESS_MAIN(TestToString)

#include "testtostring.moc"
