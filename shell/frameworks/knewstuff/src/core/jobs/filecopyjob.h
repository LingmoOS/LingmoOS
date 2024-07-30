/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FILECOPYJOB_H
#define FILECOPYJOB_H

#include "jobbase.h"

#include <QUrl>

#include <memory>

namespace KNSCore
{
class FileCopyJobPrivate;
class FileCopyJob : public KJob
{
    Q_OBJECT
public:
    explicit FileCopyJob(const QUrl &source, const QUrl &destination, int permissions = -1, JobFlags flags = DefaultFlags, QObject *parent = nullptr);
    explicit FileCopyJob(QObject *parent = nullptr);
    ~FileCopyJob() override;

    Q_SCRIPTABLE void start() override;

    QUrl destUrl() const;
    QUrl srcUrl() const;

    // This will create either a FileCopyJob, or an instance of
    // a subclass, depending on the nature of the URLs passed to
    // it
    static FileCopyJob *file_copy(const QUrl &source, const QUrl &destination, int permissions = -1, JobFlags flags = DefaultFlags, QObject *parent = nullptr);

protected Q_SLOTS:
    void handleProgressUpdate(qlonglong current, qlonglong total);
    void handleCompleted();
    void handleError(const QString &errorMessage);

private:
    const std::unique_ptr<FileCopyJobPrivate> d;
};

}

#endif // FILECOPYJOB_H
