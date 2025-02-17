/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Matej Koss <koss@miesto.sk>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSTATUSBARJOBTRACKER_H
#define KSTATUSBARJOBTRACKER_H

#include <kabstractwidgetjobtracker.h>

class KStatusBarJobTrackerPrivate;

/**
 * @class KStatusBarJobTracker kstatusbarjobtracker.h KStatusBarJobTracker
 *
 * This class implements a job tracker with a widget suited for embedding in a
 * status bar.
 */
class KJOBWIDGETS_EXPORT KStatusBarJobTracker : public KAbstractWidgetJobTracker
{
    Q_OBJECT

public:
    /**
     * @see StatusBarModes
     */
    enum StatusBarMode {
        NoInformation = 0x0000, ///< Does not show any information
        LabelOnly = 0x0001, ///< Shows an informative label for job progress
        ProgressOnly = 0x0002, ///< Shows a progress bar with the job completion
    };

    /**
     * Stores a combination of #StatusBarMode values.
     */
    Q_DECLARE_FLAGS(StatusBarModes, StatusBarMode)

    /**
     * Creates a new KStatusBarJobTracker
     *
     * @param parent the parent of this object and of the widget displaying the job progresses
     * @param button true to display a stop button allowing to kill the job, false otherwise
     */
    explicit KStatusBarJobTracker(QWidget *parent = nullptr, bool button = true);

    /**
     * Destroys a KStatusBarJobTracker
     */
    ~KStatusBarJobTracker() override;

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

    /**
     * The widget associated to this tracker.
     *
     * @return the widget displaying the job progresses
     */
    QWidget *widget(KJob *job) override;

    /**
     * Sets the mode of the status bar.
     *
     * @param statusBarMode what information the status bar will show (see StatusBarMode).
     *                      LabelOnly by default
     */
    void setStatusBarMode(StatusBarModes statusBarMode);

public Q_SLOTS:
    /**
     * The following slots are inherited from KJobTrackerInterface.
     */
    virtual void description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2) override;
    void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount) override;
    void percent(KJob *job, unsigned long percent) override;
    void speed(KJob *job, unsigned long value) override;
    void slotClean(KJob *job) override;

private:
    Q_DECLARE_PRIVATE_D(KAbstractWidgetJobTracker::d, KStatusBarJobTracker)
#if KJOBWIDGETS_BUILD_DEPRECATED_SINCE(5, 79)
    // Unused, kept for ABI compatibility
    const void *__kjobwidgets_d_do_not_use;
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KStatusBarJobTracker::StatusBarModes)

#endif
