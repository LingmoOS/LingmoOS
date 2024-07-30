/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Matej Koss <koss@miesto.sk>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kstatusbarjobtracker.h"
#include "kjobtrackerformatters_p.h"
#include "kstatusbarjobtracker_p.h"

#include <QCoreApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QObject>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>

KStatusBarJobTracker::KStatusBarJobTracker(QWidget *parent, bool button)
    : KAbstractWidgetJobTracker(*new KStatusBarJobTrackerPrivate(this, parent, button), parent)
{
}

KStatusBarJobTracker::~KStatusBarJobTracker() = default;

void KStatusBarJobTracker::registerJob(KJob *job)
{
    Q_D(KStatusBarJobTracker);

    KAbstractWidgetJobTracker::registerJob(job);

    if (d->progressWidget.contains(job)) {
        return;
    }

    auto *vi = new KStatusBarJobTrackerPrivate::ProgressWidget(job, this, d->parent);
    d->currentProgressWidget = vi;

    d->progressWidget.insert(job, vi);
}

void KStatusBarJobTracker::unregisterJob(KJob *job)
{
    Q_D(KStatusBarJobTracker);

    KAbstractWidgetJobTracker::unregisterJob(job);

    if (!d->progressWidget.contains(job)) {
        return;
    }

    if (d->currentProgressWidget == d->progressWidget[job]) {
        d->currentProgressWidget = nullptr;
    }

    if (!d->progressWidget[job]->beingDeleted) {
        delete d->progressWidget[job];
    }

    d->progressWidget.remove(job);
}

QWidget *KStatusBarJobTracker::widget(KJob *job)
{
    Q_D(KStatusBarJobTracker);

    if (!d->progressWidget.contains(job)) {
        return nullptr;
    }

    return d->progressWidget[job];
}

void KStatusBarJobTracker::setStatusBarMode(StatusBarModes statusBarMode)
{
    Q_D(KStatusBarJobTracker);

    if (!d->currentProgressWidget) {
        return;
    }

    d->currentProgressWidget->setMode(statusBarMode);
}

void KStatusBarJobTracker::description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2)
{
    Q_D(KStatusBarJobTracker);

    if (!d->progressWidget.contains(job)) {
        return;
    }

    d->progressWidget[job]->description(title, field1, field2);
}

void KStatusBarJobTracker::totalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    Q_D(KStatusBarJobTracker);

    if (!d->progressWidget.contains(job)) {
        return;
    }

    d->progressWidget[job]->totalAmount(unit, amount);
}

void KStatusBarJobTracker::percent(KJob *job, unsigned long percent)
{
    Q_D(KStatusBarJobTracker);

    if (!d->progressWidget.contains(job)) {
        return;
    }

    d->progressWidget[job]->percent(percent);
}

void KStatusBarJobTracker::speed(KJob *job, unsigned long value)
{
    Q_D(KStatusBarJobTracker);

    if (!d->progressWidget.contains(job)) {
        return;
    }

    d->progressWidget[job]->speed(value);
}

void KStatusBarJobTracker::slotClean(KJob *job)
{
    Q_D(KStatusBarJobTracker);

    if (!d->progressWidget.contains(job)) {
        return;
    }

    d->progressWidget[job]->slotClean();
}

void KStatusBarJobTrackerPrivate::ProgressWidget::killJob()
{
    job->kill(KJob::EmitResult); // notify that the job has been killed
}

void KStatusBarJobTrackerPrivate::ProgressWidget::init(KJob *job, QWidget *parent)
{
    widget = new QWidget(parent);
    int w = fontMetrics().horizontalAdvance(QStringLiteral(" 999.9 kB/s 00:00:01 ")) + 8;
    box = new QHBoxLayout(widget);
    box->setContentsMargins(0, 0, 0, 0);
    box->setSpacing(0);

    stack = new QStackedWidget(widget);
    box->addWidget(stack);

    if (q->d_func()->showStopButton) {
        button = new QPushButton(QCoreApplication::translate("KStatusBarJobTracker", "Stop"), widget);
        box->addWidget(button);
        connect(button, &QPushButton::clicked, this, &KStatusBarJobTrackerPrivate::ProgressWidget::killJob);
    } else {
        button = nullptr;
    }

    progressBar = new QProgressBar(widget);
    progressBar->installEventFilter(this);
    progressBar->setMinimumWidth(w);
    stack->insertWidget(1, progressBar);

    label = new QLabel(widget);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    label->installEventFilter(this);
    label->setMinimumWidth(w);
    stack->insertWidget(2, label);
    setMinimumSize(sizeHint());

    setMode(KStatusBarJobTracker::LabelOnly);

    q->setAutoDelete(job, true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(widget);
}

void KStatusBarJobTrackerPrivate::ProgressWidget::setMode(KStatusBarJobTracker::StatusBarModes newMode)
{
    mode = newMode;

    if (newMode == KStatusBarJobTracker::NoInformation) {
        stack->hide();

        return;
    }

    if (newMode & KStatusBarJobTracker::LabelOnly) {
        stack->show();
        stack->setCurrentWidget(label);

        return; // TODO: we should make possible to show an informative label and the progress bar
    }

    if (newMode & KStatusBarJobTracker::ProgressOnly) {
        stack->show();
        stack->setCurrentWidget(progressBar);
    }
}

void KStatusBarJobTrackerPrivate::ProgressWidget::description(const QString &title,
                                                              const QPair<QString, QString> &field1,
                                                              const QPair<QString, QString> &field2)
{
    Q_UNUSED(field1);
    Q_UNUSED(field2);

    label->setText(title);
}

void KStatusBarJobTrackerPrivate::ProgressWidget::totalAmount(KJob::Unit unit, qulonglong amount)
{
    Q_UNUSED(unit);
    Q_UNUSED(amount);
#if 0 // currently unused
    if (unit == KJob::Bytes) {
        totalSize = amount;
    }
#endif
}

void KStatusBarJobTrackerPrivate::ProgressWidget::percent(unsigned long percent)
{
    progressBar->setValue(percent);
}

void KStatusBarJobTrackerPrivate::ProgressWidget::speed(unsigned long value)
{
    if (value == 0) { // speed is measured in bytes-per-second
        label->setText(QCoreApplication::translate("KStatusBarJobTracker", " Stalled "));
    } else {
        label->setText(QCoreApplication::translate("KStatusBarJobTracker", " %1/s ").arg(KJobTrackerFormatters::byteSize(value)));
    }
}

void KStatusBarJobTrackerPrivate::ProgressWidget::slotClean()
{
    // we don't want to delete this widget, only clean
    progressBar->setValue(0);
    label->clear();

    setMode(KStatusBarJobTracker::NoInformation);
}

bool KStatusBarJobTrackerPrivate::ProgressWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == progressBar || obj == label) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = static_cast<QMouseEvent *>(event);

            // TODO: we should make possible to show an informative label and the progress bar
            if (e->button() == Qt::LeftButton) { // toggle view on left mouse button
                if (mode == KStatusBarJobTracker::LabelOnly) {
                    setMode(KStatusBarJobTracker::ProgressOnly);
                } else if (mode == KStatusBarJobTracker::ProgressOnly) {
                    setMode(KStatusBarJobTracker::LabelOnly);
                }
                return true;
            }
        }

        return false;
    }

    return QWidget::eventFilter(obj, event);
}

#include "moc_kstatusbarjobtracker.cpp"
#include "moc_kstatusbarjobtracker_p.cpp"
