/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_REQUEST_STATE_JOB_H
#define SOLID_REQUEST_STATE_JOB_H

#include <Solid/Job>

#include "power.h"
#include "solid_export.h"

namespace Solid
{
class RequestStateJobPrivate;
/**
 * Tries to set the device under the state indicated via setState()
 *
 * On success this job will contain no error and the device will
 * be set to the desired state (for example Sleep). On error
 * use error() and errorText() to know more about the issue.
 *
 * @note In some devices changing the system state might happen
 * too fast for the callee of this job to be notified (for example
 * the system might suspend before result() is emitted.
 */
class SOLID_EXPORT RequestStateJob : public Job
{
    Q_OBJECT

public:
    enum Error {
        Unsupported = Job::UserDefinedError,
    };

    explicit RequestStateJob(QObject *parent = nullptr);

    void setState(Power::InhibitionType state);

private Q_SLOTS:
    void doStart() override;

    /**
     * We have to re-declare the signal because
     * if not, Q_PROPERTY wouldn't work.
     */
Q_SIGNALS:
    void result(Solid::Job *);

private:
    Q_DECLARE_PRIVATE(RequestStateJob)
};
} // Solid namespace
#endif // SOLID_REQUEST_STATE_JOB_H
