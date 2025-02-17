/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KUISERVERJOBTRACKER_H
#define KUISERVERJOBTRACKER_H

#include <KJobTrackerInterface>
#include <kjobwidgets_export.h>

class KJob;

/**
 * @class KUiServerJobTracker kuiserverjobtracker.h KUiServerJobTracker
 *
 * The interface to implement to track the progresses of a job.
 */
class KJOBWIDGETS_EXPORT KUiServerJobTracker : public KJobTrackerInterface
{
    Q_OBJECT

public:
    /**
     * Creates a new KJobTrackerInterface
     *
     * @param parent the parent object
     */
    explicit KUiServerJobTracker(QObject *parent = nullptr);

    /**
     * Destroys a KJobTrackerInterface
     */
    ~KUiServerJobTracker() override;

    /**
     * Register a new job in this tracker.
     *
     * @param job the job to register
     */
    void registerJob(KJob *job) override;

    /**
     * Unregister a job from this tracker.
     *
     * @param job the job to unregister
     */
    void unregisterJob(KJob *job) override;

protected Q_SLOTS:
    /**
     * The following slots are inherited from KJobTrackerInterface.
     */
    void finished(KJob *job) override;
    void suspended(KJob *job) override;
    void resumed(KJob *job) override;
    virtual void description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2) override;
    void infoMessage(KJob *job, const QString &plain, const QString &rich) override;
    void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount) override;
    void processedAmount(KJob *job, KJob::Unit unit, qulonglong amount) override;
    void percent(KJob *job, unsigned long percent) override;
    void speed(KJob *job, unsigned long value) override;

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void _k_killJob())
};

#endif
