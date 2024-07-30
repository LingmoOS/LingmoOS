/*
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>
    SPDX-FileCopyrightText: 2021 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUPDATELAUNCHENVIRONMENTJOB_H
#define KUPDATELAUNCHENVIRONMENTJOB_H

#include <kdbusaddons_export.h>

#include <QProcessEnvironment>

#include <memory>

class QString;
class KUpdateLaunchEnvironmentJobPrivate;

/**
 * @class KUpdateLaunchEnvironmentJob updatelaunchenvironmentjob.h <KUpdateLaunchEnvironmentJob>
 *
 * Job for updating the launch environment.
 *
 * This job adds or updates an environment variable in process environment that will be used
 * when a process is launched:
 * This includes:
 *  - DBus activation
 *  - Systemd units
 *  - Plasma-session
 *
 * Environment variables are sanitized before uploading.
 *
 * This object deletes itself after completion, similar to KJobs
 *
 * Porting from KF5 to KF6:
 *
 * The class UpdateLaunchEnvironmentJob was renamed to KUpdateLaunchEnvironmentJob.
 *
 * @since 6.0
 */
class KDBUSADDONS_EXPORT KUpdateLaunchEnvironmentJob : public QObject
{
    Q_OBJECT

public:
    explicit KUpdateLaunchEnvironmentJob(const QProcessEnvironment &environment);
    ~KUpdateLaunchEnvironmentJob() override;

Q_SIGNALS:
    void finished();

private:
    KDBUSADDONS_NO_EXPORT void start();

private:
    std::unique_ptr<KUpdateLaunchEnvironmentJobPrivate> const d;
};

#endif
