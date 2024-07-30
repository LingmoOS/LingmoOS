/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QMutexLocker>

#include <ThreadWeaver/ResourceRestrictionPolicy>

#include "Image.h"
#include "ImageLoaderJob.h"

using namespace ThreadWeaver;

ImageLoaderJob::ImageLoaderJob(Image *image, ThreadWeaver::ResourceRestrictionPolicy *throttle)
    : m_image(image)
{
    QMutexLocker l(mutex());
    assignQueuePolicy(throttle);
}

int ImageLoaderJob::priority() const
{
    return Image::Step_LoadImage;
}

void ImageLoaderJob::run(JobPointer, Thread *)
{
    Q_ASSERT(m_image);
    m_image->loadImage();
}
