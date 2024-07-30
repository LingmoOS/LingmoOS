/* -*- C++ -*-
    This file is part of ThreadWeaver, a KDE framework.

    SPDX-FileCopyrightText: 2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef THREADWEAVER_QUEUESIGNALS_H
#define THREADWEAVER_QUEUESIGNALS_H

#include "queueinterface.h"
#include <QObject>

namespace ThreadWeaver
{
namespace Private
{
class QueueSignals_Private;
}

/** @brief QueueSignals declares the Qt signals shared by the Queue and Weaver classes. */
class THREADWEAVER_EXPORT QueueSignals : public QObject, public QueueInterface
{
    Q_OBJECT
public:
    explicit QueueSignals(QObject *parent = nullptr);
    explicit QueueSignals(ThreadWeaver::Private::QueueSignals_Private *d, QObject *parent = nullptr);
    ~QueueSignals() override;

Q_SIGNALS:
    /** @brief Emitted when the Queue has completed all jobs currently queued.
     *
     * The Queue emits finished() when the job queue is empty, and the last job currently processed by a worker threads was
     * completed. Beware that if multiple jobs are enqueued repeatedly one by one, this signal might be emitted multiple times, because the
     * queued jobs where processed before new ones could be queued. To avoid this, queue all relevant jobs in a single operation,
     * using for example a QueueStream or a Collection.
     */
    void finished();

    /** @brief The Queue has been suspended.
     *
     * When the Queue is suspended, worker threads will not be assigned new jobs to process. Jobs waiting in the queue will not be
     * started until processing is resumed. When suspend() is called, the worker threads will continue to process the job currently
     * assigned to them. When the last thread finishes it's current assignment, suspended() is emitted.
     *
     * @see suspend()
     */
    void suspended();

    /** @brief Emitted when the processing state of the Queue has changed. */
    void stateChanged(ThreadWeaver::State *);

protected:
    ThreadWeaver::Private::QueueSignals_Private *d();
    const ThreadWeaver::Private::QueueSignals_Private *d() const;

private:
    ThreadWeaver::Private::QueueSignals_Private *m_d;
};

}

#endif // THREADWEAVER_QUEUESIGNALS_H
