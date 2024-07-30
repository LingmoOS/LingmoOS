/* -*- C++ -*-
    A dependency between jobs.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dependency.h"
#include "jobinterface.h"
#include "managedjobpointer.h"

namespace ThreadWeaver
{
Dependency::Dependency(const JobPointer &dependent, const JobPointer &dependee)
    : m_dependent(dependent)
    , m_dependee(dependee)
{
}

Dependency::Dependency(JobInterface *dependent, JobInterface *dependee)
    : m_dependent(ManagedJobPointer<JobInterface>(dependent))
    , m_dependee(ManagedJobPointer<JobInterface>(dependee))
{
}

Dependency::Dependency(const JobPointer &dependent, JobInterface *dependee)
    : m_dependent(dependent)
    , m_dependee(ManagedJobPointer<JobInterface>(dependee))
{
}

Dependency::Dependency(JobInterface *dependent, const JobPointer &dependee)
    : m_dependent(ManagedJobPointer<JobInterface>(dependent))
    , m_dependee(dependee)
{
}

JobPointer Dependency::dependent() const
{
    return m_dependent;
}

JobPointer Dependency::dependee() const
{
    return m_dependee;
}

}
