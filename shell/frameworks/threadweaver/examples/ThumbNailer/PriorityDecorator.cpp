/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QThread>

#include "PriorityDecorator.h"

using namespace ThreadWeaver;

PriorityDecorator::PriorityDecorator(int priority, ThreadWeaver::JobInterface *job, bool autoDelete)
    : IdDecorator(job, autoDelete)
    , m_priority(priority)
{
}

void PriorityDecorator::run(JobPointer self, Thread *thread)
{
    IdDecorator::run(self, thread);
}

int PriorityDecorator::priority() const
{
    return m_priority;
}
