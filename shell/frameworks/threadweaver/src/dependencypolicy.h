/* -*- C++ -*-
    This file implements the DependencyPolicy class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: DebuggingAids.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#ifndef DEPENDENCYPOLICY_H
#define DEPENDENCYPOLICY_H

#include <QtGlobal>

// template <typename T> class QList;

#include "queuepolicy.h"

namespace ThreadWeaver
{
class JobInterface;
class Dependency;

/** @brief DependencyPolicy implements execution-time dependencies dependencies between Jobs.
 *
 *  To declare that Job B can only be executed when Job A is finished, call addDependency.
 *
 *  Be aware of circular dependencies. All dependencies on a Job will be removed if the Job object is destructed.
 *  Sequence uses dependencies to implement the ordered execution of the sequence elements.
 */
class THREADWEAVER_EXPORT DependencyPolicy : public QueuePolicy
{
public:
    /** Destructor. */
    ~DependencyPolicy() override;

    /** @brief Add jobB as a dependency of jobA.
     *  jobA will only be executed after jobB has been successfully processed.
     *  @param jobA the depending job
     *  @param jobB the job jobA depends on
     */
    void addDependency(JobPointer jobA, JobPointer jobB);
    void addDependency(const Dependency &dep);

    /** @brief Remove a dependency.
     *  The dependency of jobA on jobB is removed. If no dependencies are left for jobA, canRun will return true.
     *  Returns false if the given object is not dependency of this job.
     *  @param jobA the depending job
     *  @param jobB the job jobA depends on
     *  @return true if dependency existed, false otherwise
     */
    bool removeDependency(JobPointer jobA, JobPointer jobB);
    bool removeDependency(const Dependency &dep);

    /** @brief Resolve all dependencies for a job.
     *  This method is called after the Job has been finished, or when it is deleted without being executed (performed by the
     *  destructor). The method will remove all entries stating that another Job depends on this one.
     */
    void resolveDependencies(JobPointer);

    // FIXME remove
    //    /** @brief Retrieve a list of dependencies of this job. */
    //    QList<JobPointer> getDependencies(JobPointer) const;

    static DependencyPolicy &instance();

    bool canRun(JobPointer) override;

    void free(JobPointer) override;

    void release(JobPointer) override;

    void destructed(JobInterface *job) override;

    bool isEmpty() const;

protected:
    /** @brief Query whether the job has an unresolved dependency.
     *  In case it does, the policy will return false from canRun().
     */
    bool hasUnresolvedDependencies(JobPointer) const;

private:
    DependencyPolicy();
    class Private;
    Private *const d;
};

}

#endif
