/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kjobwindows.h"
#include <QObject>
#include <QVariant>
#include <QWindow>

void KJobWindows::setWindow(QObject *job, QWindow *window)
{
    job->setProperty("window", QVariant::fromValue(window));
    if (window) {
        job->setProperty("window-id", QVariant::fromValue(window->winId()));
    }
}

QWindow *KJobWindows::window(QObject *job)
{
    return job->property("window").value<QWindow *>();
}
