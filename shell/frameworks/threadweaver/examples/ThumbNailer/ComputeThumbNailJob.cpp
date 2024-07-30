/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QMutexLocker>

#include <ThreadWeaver/ResourceRestrictionPolicy>

#include "ComputeThumbNailJob.h"
#include "Image.h"

using namespace ThreadWeaver;

ComputeThumbNailJob::ComputeThumbNailJob(Image *image, ThreadWeaver::ResourceRestrictionPolicy *throttle)
    : m_image(image)
{
    QMutexLocker l(mutex());
    assignQueuePolicy(throttle);
}

int ComputeThumbNailJob::priority() const
{
    return Image::Step_ComputeThumbNail;
}

void ComputeThumbNailJob::run(JobPointer, Thread *)
{
    Q_ASSERT(m_image);
    m_image->computeThumbNail();
}
