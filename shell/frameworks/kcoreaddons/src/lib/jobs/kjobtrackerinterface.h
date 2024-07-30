/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJOBTRACKERINTERFACE_H
#define KJOBTRACKERINTERFACE_H

#include <kcoreaddons_export.h>
#include <kjob.h>

#include <QObject>
#include <QPair>

#include <memory>

/**
 * @class KJobTrackerInterface kjobtrackerinterface.h KJobTrackerInterface
 *
 * The interface to implement to track the progresses of a job.
 */
class KCOREADDONS_EXPORT KJobTrackerInterface : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new KJobTrackerInterface
     *
     * @param parent the parent object
     */
    explicit KJobTrackerInterface(QObject *parent = nullptr);

    /**
     * Destroys a KJobTrackerInterface
     */
    ~KJobTrackerInterface() override;

public Q_SLOTS:
    /**
     * Register a new job in this tracker.
     * The default implementation connects the following KJob signals
     * to the respective protected slots of this class:
     *  - finished() (also connected to the unregisterJob() slot)
     *  - suspended()
     *  - resumed()
     *  - description()
     *  - infoMessage()
     *  - totalAmount()
     *  - processedAmount()
     *  - percent()
     *  - speed()
     *
     * If you re-implement this method, you may want to call the default
     * implementation or add at least:
     *
     * @code
     * connect(job, &KJob::finished, this, &MyJobTracker::unregisterJob);
     * @endcode
     *
     * so that you won't have to manually call unregisterJob().
     *
     * @param job the job to register
     * @see unregisterJob()
     */
    virtual void registerJob(KJob *job);

    /**
     * Unregister a job from this tracker.
     * @note You need to manually call this method only if you re-implemented
     * registerJob() without connecting KJob::finished to this slot.
     *
     * @param job the job to unregister
     * @see registerJob()
     */
    virtual void unregisterJob(KJob *job);

protected Q_SLOTS:
    /**
     * Called when a job is finished, in any case. It is used to notify
     * that the job is terminated and that progress UI (if any) can be hidden.
     *
     * @param job the job that emitted this signal
     */
    virtual void finished(KJob *job);

    /**
     * Called when a job is suspended.
     *
     * @param job the job that emitted this signal
     */
    virtual void suspended(KJob *job);

    /**
     * Called when a job is resumed.
     *
     * @param job the job that emitted this signal
     */
    virtual void resumed(KJob *job);

    /**
     * Called to display general description of a job. A description has
     * a title and two optional fields which can be used to complete the
     * description.
     *
     * Examples of titles are "Copying", "Creating resource", etc.
     * The fields of the description can be "Source" with an URL, and,
     * "Destination" with an URL for a "Copying" description.
     * @param job the job that emitted this signal
     * @param title the general description of the job
     * @param field1 first field (localized name and value)
     * @param field2 second field (localized name and value)
     */
    virtual void description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2);

    /**
     * Called to display state information about a job.
     * Examples of message are "Resolving host", "Connecting to host...", etc.
     *
     * @param job the job that emitted this signal
     * @param message the info message
     */
    virtual void infoMessage(KJob *job, const QString &message);

    /**
     * Emitted to display a warning about a job.
     *
     * @param job the job that emitted this signal
     * @param message the warning message
     */
    virtual void warning(KJob *job, const QString &message);

    /**
     * Called when we know the amount a job will have to process. The unit of this
     * amount is provided too. It can be called several times for a given job if the job
     * manages several different units.
     *
     * @param job the job that emitted this signal
     * @param unit the unit of the total amount
     * @param amount the total amount
     */
    virtual void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount);

    /**
     * Regularly called to show the progress of a job by giving the current amount.
     * The unit of this amount is provided too. It can be called several times for a given
     * job if the job manages several different units.
     *
     * @param job the job that emitted this signal
     * @param unit the unit of the processed amount
     * @param amount the processed amount
     */
    virtual void processedAmount(KJob *job, KJob::Unit unit, qulonglong amount);

    /**
     * Called to show the overall progress of the job.
     * Note that this is not called for finished jobs.
     *
     * @param job the job that emitted this signal
     * @param percent the percentage
     */
    virtual void percent(KJob *job, unsigned long percent);

    /**
     * Called to show the speed of the job.
     *
     * @param job the job that emitted this signal
     * @param value the current speed of the job
     */
    virtual void speed(KJob *job, unsigned long value);

private:
    std::unique_ptr<class KJobTrackerInterfacePrivate> const d;
};

#endif
