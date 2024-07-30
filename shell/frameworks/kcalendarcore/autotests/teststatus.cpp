/*
 *  SPDX-FileCopyrightText: 2022 Glen Ditchfield <GJDitchfield@acm.org>
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "event.h"
#include "incidence.h"
#include "journal.h"
#include "todo.h"

#include <QTest>

using namespace KCalendarCore;

class TestStatus: public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testOnlyValidStatusAllowed_data()
    {
        QTest::addColumn<Incidence::Status>("status");
        QTest::addColumn<bool>("okForEvent");
        QTest::addColumn<bool>("okForTodo");
        QTest::addColumn<bool>("okForJournal");

        QTest::newRow("StatusNone") << Incidence::StatusNone << true << true << true;
        QTest::newRow("StatusTentative") << Incidence::StatusTentative << true << false << false;
        QTest::newRow("StatusConfirmed") << Incidence::StatusConfirmed << true << false << false;
        QTest::newRow("StatusCompleted") << Incidence::StatusCompleted << false << true << false;
        QTest::newRow("StatusNeedsAction") << Incidence::StatusNeedsAction << false << true << false;
        QTest::newRow("StatusCanceled") << Incidence::StatusCanceled << true << true << true;
        QTest::newRow("StatusInProcess") << Incidence::StatusInProcess << false << true << false;
        QTest::newRow("StatusDraft") << Incidence::StatusDraft << false << false << true;
        QTest::newRow("StatusFinal") << Incidence::StatusFinal << false << false << true;
        QTest::newRow("StatusX") << Incidence::StatusX << false << false << false;
    }

    void testOnlyValidStatusAllowed()
    {
        QFETCH(Incidence::Status, status);
        QFETCH(bool, okForEvent);
        QFETCH(bool, okForTodo);
        QFETCH(bool, okForJournal);

        Event e;
        e.setStatus(status);
        QVERIFY( (e.status() == status) == okForEvent);

        Todo t;
        t.setStatus(status);
        QVERIFY( (t.status() == status) == okForTodo);

        Journal j;
        j.setStatus(status);
        QVERIFY( (j.status() == status) == okForJournal);
    }
};

QTEST_MAIN(TestStatus)
#include "teststatus.moc"

