/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef IMAGELOADERJOB_H
#define IMAGELOADERJOB_H

#include <ThreadWeaver/ThreadWeaver>

class Image;

namespace ThreadWeaver
{
class ResourceRestrictionPolicy;
}

class ImageLoaderJob : public ThreadWeaver::Job
{
public:
    explicit ImageLoaderJob(Image *image, ThreadWeaver::ResourceRestrictionPolicy *throttle);
    int priority() const override;
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread) override;

private:
    Image *m_image;
};

#endif // IMAGELOADERJOB_H
