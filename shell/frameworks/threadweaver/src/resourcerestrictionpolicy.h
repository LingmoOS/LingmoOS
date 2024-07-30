/* -*- C++ -*-
    This file declares the ResourceRestrictionPolicy class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: Job.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef RESOURCE_RESTRICTION_POLICY_H
#define RESOURCE_RESTRICTION_POLICY_H

#include <QtGlobal>

#include "jobpointer.h"
#include "queuepolicy.h"

namespace ThreadWeaver
{
class JobInterface;

/** @brief ResourceRestrictionPolicy is used to limit the number of concurrent accesses to the same resource.
 *
 *  If a set of Jobs accesses a resource that can be overloaded, this may degrade application performance. For
 *  example, loading too many files from the hard disc at the same time may lead to longer load times.
 *  ResourceRestrictionPolicy can be used to cap the number of accesses. Resource restriction policies are
 *  shared between the affected jobs. All jobs that share a resource restriction policy have to acquire
 *  permission from the policy before they can run. In this way, resource restrictions can be compared to
 *  semaphores, only that they require no locking at the thread level.
 *  The  example uses a resource restriction to limit the number of images files that are loaded from
 *  the disk at the same time.
 */

class THREADWEAVER_EXPORT ResourceRestrictionPolicy : public QueuePolicy
{
public:
    explicit ResourceRestrictionPolicy(int cap = 0);
    ~ResourceRestrictionPolicy() override;

    /** @brief Cap the number of simultaneously executing jobs.
     *  Capping the amount of jobs will make sure that at max the number of jobs executing at any time is
     *  limited to the capped amount. Note that immediately after setting the amount of running jobs may be
     *  higher than the set amount. This setting only limits the starting of new jobs.
     *  @param newCap the new cap to limit the amount of parallel jobs.
     */
    void setCap(int newCap);
    int cap() const;
    bool canRun(JobPointer) override;
    void free(JobPointer) override;
    void release(JobPointer) override;
    void destructed(JobInterface *job) override;

private:
    class Private;
    Private *const d;
};

}

#endif // RESOURCE_RESTRICTION_POLICY_H
