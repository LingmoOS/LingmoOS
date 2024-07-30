/* -*- C++ -*-
    A decorator to make jobs into QObjects in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qobjectdecorator.h"
#include "collection.h"
#include "managedjobpointer.h"
#include "sequence.h"

namespace ThreadWeaver
{
QObjectDecorator::QObjectDecorator(JobInterface *decoratee, QObject *parent)
    : QObject(parent)
    , IdDecorator(decoratee)
{
}

QObjectDecorator::QObjectDecorator(JobInterface *decoratee, bool autoDelete, QObject *parent)
    : QObject(parent)
    , IdDecorator(decoratee, autoDelete)
{
}

void QObjectDecorator::defaultBegin(const JobPointer &self, Thread *thread)
{
    Q_ASSERT(job());
    Q_EMIT started(self);
    job()->defaultBegin(self, thread);
}

void QObjectDecorator::defaultEnd(const JobPointer &self, Thread *thread)
{
    Q_ASSERT(job());
    job()->defaultEnd(self, thread);
    if (!self->success()) {
        Q_EMIT failed(self);
    }
    Q_EMIT done(self);
}

}

#include "moc_qobjectdecorator.cpp"
