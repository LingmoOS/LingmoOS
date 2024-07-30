/* -*- C++ -*-
    Wrap functors in jobs in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LAMBDA_H
#define LAMBDA_H

#include "job.h"
#include "threadweaver_export.h"

namespace ThreadWeaver
{
/** @brief Lambda is a template that takes any type on which operator() is available, and executes it in run(). */
template<typename T>
class Lambda : public Job
{
public:
    explicit Lambda(T t_)
        : t(t_)
    {
    }

protected:
    void run(JobPointer, Thread *) override
    {
        t();
    }

private:
    T t;
};

}

#endif // LAMBDA_H
