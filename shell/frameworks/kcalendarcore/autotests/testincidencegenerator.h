/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2004 Till Adam <adam@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTINCIDENCEGENERATOR_H
#define TESTINCIDENCEGENERATOR_H

#include "../event.h"
#include "../journal.h"
#include "../todo.h"
using namespace KCalendarCore;

static Event *makeTestEvent()
{
    Event *event = new Event();
    event->setSummary("Test Event");
    event->recurrence()->setDaily(2);
    event->recurrence()->setDuration(3);
    return event;
}

static Todo *makeTestTodo()
{
    Todo *todo = new Todo();
    todo->setSummary("Test Todo");
    todo->setPriority(5);
    return todo;
}

static Journal *makeTestJournal()
{
    Journal *journal = new Journal();
    journal->setSummary("Test Journal");
    return journal;
}

#endif
