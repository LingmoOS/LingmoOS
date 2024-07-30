/* -*- C++ -*-
    This file declares the Sequence class.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef JOBSEQUENCE_H
#define JOBSEQUENCE_H

#include "collection.h"

namespace ThreadWeaver
{
namespace Private
{
class Sequence_Private;
}

/** @brief A Sequence is a vector of Jobs that will be executed in a sequence.
 *
 * It is implemented by automatically creating the necessary dependencies between the Jobs in the sequence.
 *
 * Sequence provides a handy cleanup and unwind mechanism: the stop() slot. If it is called, the processing
 * of the sequence will stop, and all its remaining Jobs will be dequeued.
 * A Sequence is the first element of itself. */
class THREADWEAVER_EXPORT Sequence : public Collection
{
public:
    explicit Sequence();

private:
    Private::Sequence_Private *d();
    const Private::Sequence_Private *d() const;
};

}

#endif
