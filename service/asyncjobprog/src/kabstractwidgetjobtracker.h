/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Matej Koss <koss@miesto.sk>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KABSTRACTWIDGETJOBTRACKER_H
#define KABSTRACTWIDGETJOBTRACKER_H

#include <KJobTrackerInterface>
#include <kjobwidgets_export.h>

#include <memory>

class KJob;
class QWidget;
class KAbstractWidgetJobTrackerPrivate;

/**
 * @class KAbstractWidgetJobTracker kabstractwidgetjobtracker.h KAbstractWidgetJobTracker
 *
 * The base class for widget based job trackers.
 */
class KJOBWIDGETS_EXPORT KAbstractWidgetJobTracker : public KJobTrackerInterface
{
    Q_OBJECT

public:
    /**
     * Creates a new KAbstractWidgetJobTracker
     *
     * @param parent the parent of this object and of the widget displaying the job progresses
     */
    explicit KAbstractWidgetJobTracker(QWidget *parent = nullptr);

    /**
     * Destroys a KAbstractWidgetJobTracker
     */
    ~KAbstractWidgetJobTracker() override;

    // KDE5: move this two virtual methods to be placed correctly (ereslibre)
public Q_SLOTS:
    /**
     * Register a new job in this tracker.
     * Note that job trackers inheriting from this class can have only one job
     * registered at a time.
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

public:
    /**
     * The widget associated to this tracker.
     *
     * @param job the job that is assigned the widget we want to return
     * @return the widget displaying the job progresses
     */
    virtual QWidget *widget(KJob *job) = 0;

    /**
     * This controls whether the job should be canceled if the dialog is closed.
     *
     * @param job the job's widget that will be stopped when closing
     * @param stopOnClose If true the job will be stopped if the dialog is closed,
     * otherwise the job will continue even on close.
     * @see stopOnClose()
     */
    void setStopOnClose(KJob *job, bool stopOnClose);

    /**
     * Checks whether the job will be killed when the dialog is closed.
     *
     * @param job the job's widget that will be stopped when closing
     * @return true if the job is killed on close event, false otherwise.
     * @see setStopOnClose()
     */
    bool stopOnClose(KJob *job) const;

    /**
     * This controls whether the dialog should be deleted or only cleaned when
     * the KJob is finished (or canceled).
     *
     * If your dialog is an embedded widget and not a separate window, you should
     * setAutoDelete(false) in the constructor of your custom dialog.
     *
     * @param job the job's widget that is going to be auto-deleted
     * @param autoDelete If false the dialog will only call method slotClean.
     * If true the dialog will be deleted.
     * @see autoDelete()
     */
    void setAutoDelete(KJob *job, bool autoDelete);

    /**
     * Checks whether the dialog should be deleted or cleaned.
     *
     * @param job the job's widget that will be auto-deleted
     * @return false if the dialog only calls slotClean, true if it will be
     *         deleted
     * @see setAutoDelete()
     */
    bool autoDelete(KJob *job) const;

protected Q_SLOTS:
    /**
     * Called when a job is finished, in any case. It is used to notify
     * that the job is terminated and that progress UI (if any) can be hidden.
     *
     * @param job the job that emitted this signal
     */
    void finished(KJob *job) override;

    /**
     * This method should be called for correct cancellation of IO operation
     * Connect this to the progress widgets buttons etc.
     *
     * @param job The job that is being stopped
     */
    virtual void slotStop(KJob *job);

    /**
     * This method should be called for pause/resume
     * Connect this to the progress widgets buttons etc.
     *
     * @param job The job that is being suspended
     */
    virtual void slotSuspend(KJob *job);

    /**
     * This method should be called for pause/resume
     * Connect this to the progress widgets buttons etc.
     *
     * @param job The job that is being resumed
     */
    virtual void slotResume(KJob *job);

    /**
     * This method is called when the widget should be cleaned (after job is finished).
     * redefine this for custom behavior.
     *
     * @param job The job that is being cleaned
     */
    virtual void slotClean(KJob *job);

Q_SIGNALS:
    /**
     * Emitted when the user aborted the operation
     *
     * @param job The job that has been stopped
     */
    void stopped(KJob *job);

    /**
     * Emitted when the user suspended the operation
     *
     * @param job The job that has been suspended
     */
    void suspend(KJob *job);

    /**
     * Emitted when the user resumed the operation
     *
     * @param job The job that has been resumed
     */
    void resume(KJob *job);

protected:
    KAbstractWidgetJobTracker(KAbstractWidgetJobTrackerPrivate &dd, QWidget *parent = nullptr);

protected:
    friend class KStatusBarJobTracker;
    friend class KWidgetJobTracker;
    std::unique_ptr<KAbstractWidgetJobTrackerPrivate> const d;
    // KF6 TODO: change d to d_ptr, use normal Q_DECLARE_PRIVATE, remove friend

private:
    Q_DECLARE_PRIVATE_D(d, KAbstractWidgetJobTracker)
};

#endif
