/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef YOUTUBEJOBCOMPOSITE_H
#define YOUTUBEJOBCOMPOSITE_H

#include <purpose/job.h>

class YoutubeJobComposite : public Purpose::Job
{
public:
    YoutubeJobComposite();

    void start() override;

private Q_SLOTS:
    void subjobFinished(KJob *subjob);

private:
    int m_pendingJobs;
};

#endif // YOUTUBEJOBCOMPOSITE_H
