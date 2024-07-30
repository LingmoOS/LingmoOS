/* -*- C++ -*-
    Shared pointer based jobs that are managed by the caller in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MANAGEDJOBPOINTER_H
#define MANAGEDJOBPOINTER_H

#include <QSharedPointer>

#include "jobinterface.h"

namespace ThreadWeaver
{
inline void doNotDeleteJob(JobInterface *)
{
}

template<typename T>
class ManagedJobPointer : public QSharedPointer<T>
{
public:
    ManagedJobPointer(T *job)
        : QSharedPointer<T>(job, doNotDeleteJob)
    {
    }
};

}

#endif // MANAGEDJOBPOINTER_H
