/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_STATES_JOB_H
#define SOLID_STATES_JOB_H

#include "power.h"
#include "solid_export.h"

#include <Solid/Job>

namespace Solid
{
class StatesJobPrivate;
/**
 * Returns the states supported on the device
 *
 * Different devices and different operating systems support
 * a different set of states such Sleep or Hibernation.
 *
 */
class SOLID_EXPORT StatesJob : public Job
{
    Q_OBJECT

public:
    explicit StatesJob(QObject *parent = nullptr);

    /**
     * Once the job is finished, returns the supported Power::InhibitionTypes
     */
    Power::InhibitionTypes states() const;

private Q_SLOTS:
    void doStart() override;

    /**
     * We have to re-declare the signal because
     * if not, Q_PROPERTY wouldn't work.
     */
Q_SIGNALS:
    void result(Solid::Job *job);

private:
    Q_DECLARE_PRIVATE(StatesJob)
};
} // Solid namespace
#endif // SOLID_STATES_JOB_H
