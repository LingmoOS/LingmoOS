/*
    SPDX-FileCopyrightText: 2017 Lim Yuen Hoe <yuenhoe86@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef NEXTCLOUDJOB_H
#define NEXTCLOUDJOB_H

#include <QString>
#include <QUrl>
#include <purpose/pluginbase.h>

class NextcloudJob : public Purpose::Job
{
    Q_OBJECT
public:
    NextcloudJob(QObject *parent)
        : Purpose::Job(parent)
    {
    }
    void start() override;

private Q_SLOTS:
    void gotCredentials(KJob *job);
};
#endif /* NEXTCLOUDJOB_H */
