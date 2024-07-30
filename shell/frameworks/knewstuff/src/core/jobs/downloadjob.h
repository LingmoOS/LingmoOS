/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef DOWNLOADJOB_H
#define DOWNLOADJOB_H

#include "filecopyjob.h"

#include <memory>

namespace KNSCore
{
class DownloadJobPrivate;
class DownloadJob : public FileCopyJob
{
    Q_OBJECT
public:
    explicit DownloadJob(const QUrl &source, const QUrl &destination, int permissions = -1, JobFlags flags = DefaultFlags, QObject *parent = nullptr);
    explicit DownloadJob(QObject *parent = nullptr);
    ~DownloadJob() override;

    Q_SCRIPTABLE void start() override;

protected Q_SLOTS:
    void handleWorkerCompleted();
    void handleWorkerError(const QString &error);

private:
    const std::unique_ptr<DownloadJobPrivate> d;
};

}

#endif // DOWNLOADJOB_H
