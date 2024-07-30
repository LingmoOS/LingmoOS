/* -*- C++ -*-
    This file is part of ThreadWeaver. It declares the Thread class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef THREADWEAVER_THREAD_H
#define THREADWEAVER_THREAD_H

#include <QMutex>
#include <QThread>

#include "jobpointer.h"
#include "threadweaver_export.h"

namespace ThreadWeaver
{
class Job;
class Weaver;

/** @brief Thread represents a worker thread in a Queue's inventory.
 *
 * Threads are created and managed by queues on demand. A Thread will try to retrieve and process
 * jobs from the queue until it is told to exit. */
class THREADWEAVER_EXPORT Thread : public QThread
{
    Q_OBJECT
public:
    /** @brief Create a thread.
     *
     *  @param parent the parent Weaver
     */
    explicit Thread(Weaver *parent = nullptr);

    /** The destructor. */
    ~Thread() override;

    /** @brief The run method is reimplemented to execute jobs from the queue.
     *
     * Whenever the thread is idle, it will ask its Weaver parent for a Job to do. The Weaver will either return a Job or a null
     * pointer. When a null pointer is returned, it tells the thread to exit.
     */
    void run() override;

    /** @brief Returns the thread id.
     *
     * This id marks the respective Thread object, and must therefore not be confused with, e.g., the pthread thread ID.
     * The way threads are implemented and identified is platform specific. id() is the only way to uniquely identify a thread
     * within ThreadWeaver.
     */
    unsigned int id() const;

    /** @brief Request the abortion of the job that is processed currently.
     *
     * If there is no current job, this method will do nothing, but can safely be called. It forwards the request to the
     * current Job.
     */
    void requestAbort();

private:
    class Private;
    Private *const d;
};

}

#endif
