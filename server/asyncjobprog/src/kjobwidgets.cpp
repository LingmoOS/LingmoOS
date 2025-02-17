/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kjobwidgets.h"
#include <KJob>
#include <QVariant>
#include <QWidget>
#include <QWindow>

void KJobWidgets::setWindow(KJob *job, QWidget *widget)
{
    job->setProperty("widget", QVariant::fromValue(widget));
    KJobWindows::setWindow(job, widget ? widget->windowHandle() : nullptr);
}

QWidget *KJobWidgets::window(KJob *job)
{
    return job->property("widget").value<QWidget *>();
}

// duplicated from kwindowsystem
static int timestampCompare(unsigned long time1_, unsigned long time2_) // like strcmp()
{
    quint32 time1 = time1_;
    quint32 time2 = time2_;
    if (time1 == time2) {
        return 0;
    }
    return quint32(time1 - time2) < 0x7fffffffU ? 1 : -1; // time1 > time2 -> 1, handle wrapping
}

void KJobWidgets::updateUserTimestamp(KJob *job, unsigned long time)
{
    unsigned long currentTimestamp = userTimestamp(job);
    if (currentTimestamp == 0 || timestampCompare(time, currentTimestamp) > 0) {
        job->setProperty("userTimestamp", qulonglong(time));
    }
}

unsigned long KJobWidgets::userTimestamp(KJob *job)
{
    return job->property("userTimestamp").toULongLong();
}

void KJobWindows::setWindow(KJob *job, QWindow *window)
{
    job->setProperty("window", QVariant::fromValue(window));
    if (window) {
        job->setProperty("window-id", QVariant::fromValue(window->winId()));
    }
}

QWindow *KJobWindows::window(KJob *job)
{
    return job->property("window").value<QWindow *>();
}
