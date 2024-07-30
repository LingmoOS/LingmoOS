/* -*- C++ -*-
    A decorator to make jobs into QObjects in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef THREADWEAVER_QOBJECTDECORATOR_H
#define THREADWEAVER_QOBJECTDECORATOR_H

#include <QObject>

#include "iddecorator.h"
#include "threadweaver_export.h"

namespace ThreadWeaver
{
class Collection;
class Sequence;

class THREADWEAVER_EXPORT QObjectDecorator : public QObject, public IdDecorator
{
    Q_OBJECT
public:
    explicit QObjectDecorator(JobInterface *decoratee, QObject *parent = nullptr);
    explicit QObjectDecorator(JobInterface *decoratee, bool autoDelete, QObject *parent = nullptr);

Q_SIGNALS:
    /** This signal is emitted when this job is being processed by a thread. */
    void started(ThreadWeaver::JobPointer);
    /** This signal is emitted when the job has been finished (no matter if it succeeded or not). */
    void done(ThreadWeaver::JobPointer);
    /** This job has failed.
     *
     * This signal is emitted when success() returns false after the job is executed. */
    void failed(ThreadWeaver::JobPointer);

protected:
    void defaultBegin(const JobPointer &job, Thread *thread) override;
    void defaultEnd(const JobPointer &job, Thread *thread) override;
};

typedef QSharedPointer<QObjectDecorator> QJobPointer;

}

#endif // THREADWEAVER_QOBJECTDECORATOR_H
