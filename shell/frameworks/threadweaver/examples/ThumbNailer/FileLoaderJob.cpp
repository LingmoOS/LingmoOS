/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QMutexLocker>

#include <ThreadWeaver/ResourceRestrictionPolicy>

#include "FileLoaderJob.h"
#include "Image.h"

using namespace ThreadWeaver;

FileLoaderJob::FileLoaderJob(Image *image, ThreadWeaver::ResourceRestrictionPolicy *throttle)
    : m_image(image)
{
    QMutexLocker l(mutex());
    assignQueuePolicy(throttle);
}

int FileLoaderJob::priority() const
{
    return Image::Step_LoadFile;
}

void FileLoaderJob::run(JobPointer, Thread *)
{
    Q_ASSERT(m_image);
    m_image->loadFile();
}
