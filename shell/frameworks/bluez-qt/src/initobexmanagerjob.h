/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_INITOBEXMANAGERJOB_H
#define BLUEZQT_INITOBEXMANAGERJOB_H

#include "bluezqt_export.h"
#include "job.h"

namespace BluezQt
{
class ObexManager;

/**
 * @class BluezQt::InitObexManagerJob initobexmanagerjob.h <BluezQt/InitObexManagerJob>
 *
 * Init obex manager job.
 *
 * This class represents a job that initializes ObexManager.
 */
class BLUEZQT_EXPORT InitObexManagerJob : public Job
{
    Q_OBJECT
    Q_PROPERTY(ObexManager *manager READ manager)

public:
    /**
     * Destroys an InitObexManagerJob object.
     */
    ~InitObexManagerJob() override;

    /**
     * Returns a manager that is being initialized.
     *
     * @return manager
     */
    ObexManager *manager() const;

Q_SIGNALS:
    /**
     * Indicates that the job have finished.
     */
    void result(InitObexManagerJob *job);

private:
    BLUEZQT_NO_EXPORT explicit InitObexManagerJob(ObexManager *manager);

    void doStart() override;
    void doEmitResult() override;

    std::unique_ptr<class InitObexManagerJobPrivate> const d;

    friend class InitObexManagerJobPrivate;
    friend class ObexManager;
};

} // namespace BluezQt

#endif // BLUEZQT_INITOBEXMANAGERJOB_H
