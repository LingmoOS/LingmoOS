/* -*- C++ -*-
    The Queue class in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef THREADWEAVER_QUEUE_H
#define THREADWEAVER_QUEUE_H

#include <QObject>

#include "queuesignals.h"
#include "queuestream.h"

namespace ThreadWeaver
{
class Job;
class State;

/** @brief Queue implements a ThreadWeaver job queue.
 *
 * Queues process jobs enqueued in them by automatically assigning them to worker threads they manage.
 * Applications using ThreadWeaver can make use of a global Queue which is instantiated on demand, or
 * create multiple queues as needed. A job assigned to a queue will be processed by that specific queue.
 *
 * Worker threads are created by the queues as needed. To create a customized global queue,
 * see GlobalQueueFactory.
 *
 * @see GlobalQueueFactory
 * @see Queue::enqueue()
 * @see Queue::instance()
 */
class THREADWEAVER_EXPORT Queue : public QueueSignals
{
    Q_OBJECT
public:
    explicit Queue(QObject *parent = nullptr);
    /** @brief Construct a queue with a customized implementation
     * The queue takes ownership and will delete the implementation upon destruction. */
    explicit Queue(QueueSignals *implementation, QObject *parent = nullptr);
    ~Queue() override;

    QueueStream stream();

    const State *state() const override;

    void setMaximumNumberOfThreads(int cap) override;
    int maximumNumberOfThreads() const override;
    int currentNumberOfThreads() const override;

    static ThreadWeaver::Queue *instance();
    void enqueue(const QList<JobPointer> &jobs) override;
    void enqueue(const JobPointer &job);
    bool dequeue(const JobPointer &) override;
    void dequeue() override;
    void finish() override;
    void suspend() override;
    void resume() override;
    bool isEmpty() const override;
    bool isIdle() const override;
    int queueLength() const override;
    void requestAbort() override;
    void reschedule() override;
    void shutDown() override;

    /** @brief Interface for the global queue factory. */
    struct GlobalQueueFactory {
        virtual ~GlobalQueueFactory()
        {
        }
        virtual Queue *create(QObject *parent) = 0;
    };
    static void setGlobalQueueFactory(GlobalQueueFactory *factory);

private:
    class Private;
    Private *const d;
};

}

#endif // THREADWEAVER_QUEUE_H
