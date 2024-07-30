/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "job.h"

using namespace KUserFeedback::Console;

Job::Job(QObject* parent)
    : QObject(parent)
{
}

Job::~Job() = default;

void Job::emitError(const QString& msg)
{
    Q_EMIT error(msg);
    deleteLater();
}

void Job::emitFinished()
{
    Q_EMIT finished();
    deleteLater();
}

#include "moc_job.cpp"
