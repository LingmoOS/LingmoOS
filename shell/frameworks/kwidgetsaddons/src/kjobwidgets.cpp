/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kjobwidgets.h"
#include <QVariant>
#include <QWidget>
#include <QWindow>

void KJobWidgets::setWindow(QObject *job, QWidget *widget)
{
    job->setProperty("widget", QVariant::fromValue(widget));

    QWindow *windowHandle = widget ? widget->windowHandle() : nullptr;
    job->setProperty("window", QVariant::fromValue(windowHandle));
    if (windowHandle) {
        job->setProperty("window-id", QVariant::fromValue(windowHandle->winId()));
    }
}

#if KWIDGETSADDONS_BUILD_DEPRECATED_SINCE(6, 5)
void KJobWidgets::setWindowHandle(QObject *job, QWindow *window)
{
    job->setProperty("window", QVariant::fromValue(window));
    if (window) {
        job->setProperty("window-id", QVariant::fromValue(window->winId()));
    }
}
#endif

QWidget *KJobWidgets::window(QObject *job)
{
    return job->property("widget").value<QWidget *>();
}

#if KWIDGETSADDONS_BUILD_DEPRECATED_SINCE(6, 5)
QWindow *KJobWidgets::windowHandle(QObject *job)
{
    return job->property("window").value<QWindow *>();
}
#endif

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

void KJobWidgets::updateUserTimestamp(QObject *job, unsigned long time)
{
    unsigned long currentTimestamp = userTimestamp(job);
    if (currentTimestamp == 0 || timestampCompare(time, currentTimestamp) > 0) {
        job->setProperty("userTimestamp", qulonglong(time));
    }
}

unsigned long KJobWidgets::userTimestamp(QObject *job)
{
    return job->property("userTimestamp").toULongLong();
}
