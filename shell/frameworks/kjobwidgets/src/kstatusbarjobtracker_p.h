/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSTATUSBARJOBTRACKER_P_H
#define KSTATUSBARJOBTRACKER_P_H

#include "kabstractwidgetjobtracker_p.h"
#include "kstatusbarjobtracker.h"

#include <QBoxLayout>
#include <QMap>
#include <QStackedWidget>
#include <QWidget>

class QPushButton;
class QCheckBox;
class QLabel;
class QProgressBar;

class KStatusBarJobTrackerPrivate : public KAbstractWidgetJobTrackerPrivate
{
    Q_DECLARE_PUBLIC(KStatusBarJobTracker)

public:
    KStatusBarJobTrackerPrivate(KStatusBarJobTracker *q, QWidget *parent, bool withStopButton)
        : KAbstractWidgetJobTrackerPrivate(q)
        , parent(parent)
        , currentProgressWidget(nullptr)
        , showStopButton(withStopButton)
    {
    }

    class ProgressWidget;

    QWidget *parent;
    QMap<KJob *, ProgressWidget *> progressWidget;
    ProgressWidget *currentProgressWidget;
    bool showStopButton;
};

class KStatusBarJobTrackerPrivate::ProgressWidget : public QWidget
{
    Q_OBJECT

public:
    ProgressWidget(KJob *job, KStatusBarJobTracker *object, QWidget *parent)
        : q(object)
        , job(job)
    {
        init(job, parent);
    }

    ~ProgressWidget() override
    {
        beingDeleted = true;
        delete widget;

        q->unregisterJob(job);
    }

    KStatusBarJobTracker *const q;
    KJob *const job;

    QWidget *widget = nullptr;
    QProgressBar *progressBar = nullptr;
    QLabel *label = nullptr;
    QPushButton *button = nullptr;
    QBoxLayout *box = nullptr;
    QStackedWidget *stack = nullptr;

    // qlonglong totalSize = -1;

    KStatusBarJobTracker::StatusBarModes mode = KStatusBarJobTracker::NoInformation;
    bool beingDeleted = false;

    void init(KJob *job, QWidget *parent);

    void setMode(KStatusBarJobTracker::StatusBarModes newMode);

public Q_SLOTS:
    virtual void description(const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2);
    virtual void totalAmount(KJob::Unit unit, qulonglong amount);
    virtual void percent(unsigned long percent);
    virtual void speed(unsigned long value);
    virtual void slotClean();

private Q_SLOTS:
    void killJob();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // KSTATUSBARJOBTRACKER_P_H
