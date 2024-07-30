/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_INITMANAGERJOB_H
#define BLUEZQT_INITMANAGERJOB_H

#include "bluezqt_export.h"
#include "job.h"

#include <memory>

namespace BluezQt
{
class Manager;

/**
 * @class BluezQt::InitManagerJob initmanagerjob.h <BluezQt/InitManagerJob>
 *
 * Init manager job.
 *
 * This class represents a job that initializes Manager.
 */
class BLUEZQT_EXPORT InitManagerJob : public Job
{
    Q_OBJECT
    Q_PROPERTY(Manager *manager READ manager)

public:
    /**
     * Destroys an InitManagerJob object.
     */
    ~InitManagerJob() override;

    /**
     * Returns a manager that is being initialized.
     *
     * @return manager
     */
    Manager *manager() const;

Q_SIGNALS:
    /**
     * Indicates that the job have finished.
     */
    void result(InitManagerJob *job);

private:
    explicit InitManagerJob(Manager *manager);

    void doStart() override;
    void doEmitResult() override;

    std::unique_ptr<class InitManagerJobPrivate> const d;

    friend class InitManagerJobPrivate;
    friend class Manager;
};

} // namespace BluezQt

#endif // BLUEZQT_INITMANAGERJOB_H
