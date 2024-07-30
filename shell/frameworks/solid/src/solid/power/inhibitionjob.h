/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_ADD_INHIBITION_JOB
#define SOLID_ADD_INHIBITION_JOB

#include "power.h"

#include <solid/job.h>

#include "solid_export.h"

namespace Solid
{
class Inhibition;
class InhibitionJobPrivate;
class SOLID_EXPORT InhibitionJob : public Job
{
    Q_OBJECT
public:
    enum Error {
        InvalidInhibitions = Job::UserDefinedError,
        EmptyDescription,
    };
    Q_ENUM(Error)

    /**
     * Instantiate InhibitionJob
     *
     * When this job emits result(Solid::Job*) and in case no
     * error has happened, an Inhibition object will be
     * returned using inhibition(). Delete the returned object
     * as soon as the inhibition should be released.
     *
     * At least one action to inhibit and description should be
     * added.
     */
    explicit InhibitionJob(QObject *parent = nullptr);

    /**
     * Sets th list of action to inhibit
     *
     * @param inhibitions list of actions to inhibit
     */
    void setInhibitions(Power::InhibitionTypes inhibitions);

    /**
     * Returns the list of actions this job will inhibit
     *
     * @return List of Power::Imhibitions
     */
    Power::InhibitionTypes inhibitions() const;

    /**
     * Sets the description of this inhibition
     *
     * @param description explains why the inhibition is required
     */
    void setDescription(const QString &description);

    /**
     * Returns the description to be used
     */
    QString description() const;

    /**
     * Returns the inhibition
     *
     * The result of this job is an object called Inhibition
     * which should be kept as long as the inhibition is desired.
     *
     * If this method is called before result(Solid::Job*) is emitted,
     * it will return nullptr.
     */
    Inhibition *inhibition() const;

private Q_SLOTS:
    void doStart() override;

    /**
     * We have to re-declare the signal because
     * if not, Q_PROPERTY wouldn't work.
     */
Q_SIGNALS:
    void result(Solid::Job *);

private:
    Q_DECLARE_PRIVATE(InhibitionJob)
};
}
#endif // SOLID_ADD_INHIBITION_JOB
