/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COMPUTETHUMBNAILJOB_H
#define COMPUTETHUMBNAILJOB_H

#include <ThreadWeaver/ThreadWeaver>

class Image;

namespace ThreadWeaver
{
class ResourceRestrictionPolicy;
}

class ComputeThumbNailJob : public ThreadWeaver::Job
{
public:
    ComputeThumbNailJob(Image *image, ThreadWeaver::ResourceRestrictionPolicy *throttle);
    int priority() const override;
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread) override;

private:
    Image *m_image;
};

#endif // COMPUTETHUMBNAILJOB_H
