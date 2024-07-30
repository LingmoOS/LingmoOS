/* -*- C++ -*-
    This file implements the state handling in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: State.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include "state.h"

namespace ThreadWeaver
{
const char *const StateNames[NoOfStates] = {"InConstruction", "WorkingHard", "Suspending", "Suspended", "ShuttingDown", "Destructed"};

class Q_DECL_HIDDEN State::Private
{
public:
    Private(QueueInterface *theWeaver)
        : weaver(theWeaver)
    {
        Q_ASSERT_X(sizeof StateNames / sizeof StateNames[0] == NoOfStates, "State::Private ctor", "Make sure to keep StateId and StateNames in sync!");
    }

    /** The Weaver we relate to. */
    QueueInterface *const weaver;
};

State::State(QueueSignals *weaver)
    : d(new Private(weaver))
{
}

State::~State()
{
    delete d;
}

QString State::stateName() const
{
    return QLatin1String(StateNames[stateId()]);
}

void State::activated()
{
}

QueueInterface *State::weaver()
{
    return d->weaver;
}

const QueueInterface *State::weaver() const
{
    return d->weaver;
}

}
