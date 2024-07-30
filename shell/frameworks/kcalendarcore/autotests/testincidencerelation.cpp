/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "testincidencerelation.h"
#include "todo.h"

#include <QTest>
QTEST_MAIN(IncidenceRelationTest)

using namespace KCalendarCore;

void IncidenceRelationTest::testRelations()
{
    // Build the following tree:
    // todo1
    // \- todo2
    //    \- todo3

    // Then make todo3 independent:
    // todo3
    // todo1
    // \- todo2

    Todo::Ptr todo1 = Todo::Ptr(new Todo());
    todo1->setSummary(QStringLiteral("todo"));

    Todo::Ptr todo2 = Todo::Ptr(new Todo());
    todo2->setSummary(QStringLiteral("sub-todo"));

    Todo::Ptr todo3 = Todo::Ptr(new Todo());
    todo3->setSummary(QStringLiteral("sub-sub-todo"));

    todo3->setRelatedTo(todo2->uid());
    todo2->setRelatedTo(todo1->uid());

    QCOMPARE(todo3->relatedTo(), todo2->uid());
    QCOMPARE(todo2->relatedTo(), todo1->uid());
    QCOMPARE(todo1->relatedTo(), QString());

    todo3->setRelatedTo(QString());

    QCOMPARE(todo3->relatedTo(), QString());
    QCOMPARE(todo2->relatedTo(), todo1->uid());
    QCOMPARE(todo1->relatedTo(), QString());
}

#include "moc_testincidencerelation.cpp"
