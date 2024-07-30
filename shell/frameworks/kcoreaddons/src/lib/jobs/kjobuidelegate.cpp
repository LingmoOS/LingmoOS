/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kjobuidelegate.h"
#include "kcoreaddons_debug.h"
#include "kjob.h"

#include <QDebug>

class KJobUiDelegatePrivate
{
public:
    KJobUiDelegatePrivate(KJobUiDelegate *delegate)
        : q(delegate)
        , autoErrorHandling(false)
        , autoWarningHandling(true)
    {
    }

    KJobUiDelegate *const q;

    KJob *job = nullptr;
    bool autoErrorHandling : 1;
    bool autoWarningHandling : 1;

    void connectJob(KJob *job);
    void _k_result();
};

KJobUiDelegate::KJobUiDelegate(Flags flags)
    : QObject()
    , d(new KJobUiDelegatePrivate(this))
{
    if (flags & AutoErrorHandlingEnabled) {
        d->autoErrorHandling = true;
    }
    if (flags & AutoWarningHandlingEnabled) {
        d->autoWarningHandling = true;
    }
}

KJobUiDelegate::~KJobUiDelegate() = default;

bool KJobUiDelegate::setJob(KJob *job)
{
    if (d->job != nullptr) {
        qCWarning(KCOREADDONS_DEBUG) << "Trying to attach UI delegate:" << this << "to job" << job //
                                     << "but this delegate is already attached to a different job" << d->job;
        return false;
    }

    d->job = job;
    setParent(job);

    return true;
}

KJob *KJobUiDelegate::job() const
{
    return d->job;
}

void KJobUiDelegate::showErrorMessage()
{
    if (d->job->error() != KJob::KilledJobError) {
        qWarning() << d->job->errorString();
    }
}

void KJobUiDelegate::setAutoErrorHandlingEnabled(bool enable)
{
    d->autoErrorHandling = enable;
}

bool KJobUiDelegate::isAutoErrorHandlingEnabled() const
{
    return d->autoErrorHandling;
}

void KJobUiDelegate::setAutoWarningHandlingEnabled(bool enable)
{
    d->autoWarningHandling = enable;
}

bool KJobUiDelegate::isAutoWarningHandlingEnabled() const
{
    return d->autoWarningHandling;
}

void KJobUiDelegate::slotWarning(KJob *job, const QString &message)
{
    Q_UNUSED(job)
    Q_UNUSED(message)
}

void KJobUiDelegate::connectJob(KJob *job)
{
    connect(job, &KJob::result, this, [this]() {
        d->_k_result();
    });
    connect(job, &KJob::warning, this, &KJobUiDelegate::slotWarning);
}

void KJobUiDelegatePrivate::_k_result()
{
    if (job->error() && autoErrorHandling) {
        q->showErrorMessage();
    }
}

#include "moc_kjobuidelegate.cpp"
