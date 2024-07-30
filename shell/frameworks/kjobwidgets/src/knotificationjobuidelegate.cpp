/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knotificationjobuidelegate.h"

#include <QString>

#include <KJob>
#include <KNotification>

class KNotificationJobUiDelegatePrivate
{
public:
    void showNotification(KNotification::StandardEvent standardEvent, const QString &text);

    QString description;
};

void KNotificationJobUiDelegatePrivate::showNotification(KNotification::StandardEvent standardEvent, const QString &text)
{
    QString title = description;
    if (standardEvent == KNotification::Error && !title.isEmpty()) {
        //: Job name, e.g. Copying has failed
        title = KNotificationJobUiDelegate::tr("%1 (Failed)").arg(title);
    }
    KNotification::event(standardEvent, title, text);
}

KNotificationJobUiDelegate::KNotificationJobUiDelegate(KJobUiDelegate::Flags flags)
    : KJobUiDelegate(flags)
    , d(new KNotificationJobUiDelegatePrivate)
{
}

KNotificationJobUiDelegate::~KNotificationJobUiDelegate() = default;

bool KNotificationJobUiDelegate::setJob(KJob *job)
{
    const bool ok = KJobUiDelegate::setJob(job);

    if (ok) {
        connect(job, &KJob::description, this, [this](KJob *, const QString &title, const QPair<QString, QString> &, const QPair<QString, QString> &) {
            d->description = title;
        });
    }

    return ok;
}

void KNotificationJobUiDelegate::showErrorMessage()
{
    if (job()->error() == KJob::KilledJobError) {
        return;
    }

    d->showNotification(KNotification::Error, job()->errorString());
}

void KNotificationJobUiDelegate::slotWarning(KJob *job, const QString &message)
{
    Q_UNUSED(job);

    if (isAutoErrorHandlingEnabled()) {
        d->showNotification(KNotification::Notification, message);
    }
}

#include "moc_knotificationjobuidelegate.cpp"
