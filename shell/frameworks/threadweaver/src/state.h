/* -*- C++ -*-
    This file implements the state handling in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: State.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef THREADWEAVER_STATE_H
#define THREADWEAVER_STATE_H

#include <QString>

#include "queueinterface.h"
#include "queuesignals.h"
#include "threadweaver_export.h"
#include "weaverinterface.h"

namespace ThreadWeaver
{
class Job;
class Thread;

/** All weaver objects maintain a state of operation which can be
 *  queried by the application. See the threadweaver documentation on
 *  how the different states are related.
 */

enum StateId {
    /** The object is in the state of construction and has not yet
     *  started to process jobs. */
    InConstruction = 0,
    /** Jobs are being processed. */
    WorkingHard,
    /** Job processing is suspended, but some jobs which where already
     *  in progress are not finished yet. */
    Suspending,
    /** Job processing is suspended, and no jobs are being
     *  processed. */
    Suspended,
    /** The object is being destructed. Jobs might still be processed,
     *  the destructor will wait for all threads to exit and then
     *  end. */
    ShuttingDown,
    /** The object is being destructed, and all threads have
     *  exited. No jobs are handled anymore. */
    Destructed,
    /** Not a state, but a sentinel for the number of defined states. */
    NoOfStates,
};

/** We use a State pattern to handle the system state in ThreadWeaver. */
class THREADWEAVER_EXPORT State : public QueueInterface, public WeaverInterface
{
public:
    /** Default constructor. */
    explicit State(QueueSignals *weaver);

    /** Destructor. */
    ~State() override;

    /** The ID of the current state.
     *  @see StateNames, StateID
     */
    QString stateName() const;

    /** The state Id. */
    virtual StateId stateId() const = 0;

    /** The state has been changed so that this object is responsible for
     *  state handling. */
    virtual void activated();

protected:
    /** The Weaver interface this state handles. */
    virtual QueueInterface *weaver();
    virtual const QueueInterface *weaver() const;

private:
    class Private;
    Private *const d;
};

}

#endif // THREADWEAVER_STATE_H
