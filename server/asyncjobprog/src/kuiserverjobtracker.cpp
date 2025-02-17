/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kuiserverjobtracker.h"
#include "kuiserverjobtracker_p.h"

#include "debug.h"
#include "jobviewiface.h"

#include <KJob>

#include <QtGlobal>
#include <QApplication>
#include <QDBusConnection>
#include <QIcon>

Q_GLOBAL_STATIC(KSharedUiServerProxy, serverProxy)

class Q_DECL_HIDDEN KUiServerJobTracker::Private
{
public:
    Private(KUiServerJobTracker *parent)
        : q(parent)
    {
    }

    KUiServerJobTracker *const q;

    void _k_killJob();

    static void updateDestUrl(KJob *job, org::kde::JobViewV2 *jobView);

    QHash<KJob *, org::kde::JobViewV2 *> progressJobView;

    QMetaObject::Connection serverRegisteredConnection;
};

void KUiServerJobTracker::Private::_k_killJob()
{
    org::kde::JobViewV2 *jobView = qobject_cast<org::kde::JobViewV2 *>(q->sender());

    if (jobView) {
        KJob *job = progressJobView.key(jobView);

        if (job) {
            job->kill(KJob::EmitResult);
        }
    }
}

void KUiServerJobTracker::Private::updateDestUrl(KJob *job, org::kde::JobViewV2 *jobView)
{
    const QVariant destUrl = job->property("destUrl");
    if (destUrl.isValid()) {
        jobView->setDestUrl(QDBusVariant(destUrl));
    }
}

KUiServerJobTracker::KUiServerJobTracker(QObject *parent)
    : KJobTrackerInterface(parent)
    , d(new Private(this))
{
}

KUiServerJobTracker::~KUiServerJobTracker()
{
    if (!d->progressJobView.isEmpty()) {
        qWarning() << "A KUiServerJobTracker instance contains" << d->progressJobView.size() << "stalled jobs";
    }

    qDeleteAll(d->progressJobView);

    delete d;
}

void KUiServerJobTracker::registerJob(KJob *job)
{
    // Already registered job?
    if (d->progressJobView.contains(job)) {
        return;
    }

    // Watch the server registering/unregistering and re-register the jobs as needed
    if (!d->serverRegisteredConnection) {
        d->serverRegisteredConnection = connect(serverProxy(), &KSharedUiServerProxy::serverRegistered, this, [this]() {
            // Remember the list of jobs to re-register and then delete the old ones
            const QList<KJob *> staleJobs = d->progressJobView.keys();

            qDeleteAll(d->progressJobView);
            d->progressJobView.clear();

            for (KJob *job : staleJobs) {
                registerJob(job);
            }
        });
    }

    const QString appName = QCoreApplication::applicationName();
    // This will only work if main() used QIcon::fromTheme.
    QString programIconName = QApplication::windowIcon().name();

    if (programIconName.isEmpty()) {
        programIconName = appName;
    }

    QPointer<KJob> jobWatch = job;
    QDBusReply<QDBusObjectPath> reply = serverProxy()->uiserver()->requestView(appName, programIconName, job->capabilities());

    // If we got a valid reply, register the interface for later usage.
    if (reply.isValid()) {
        org::kde::JobViewV2 *jobView = new org::kde::JobViewV2(QStringLiteral("org.kde.JobViewServer"), reply.value().path(), QDBusConnection::sessionBus());
        if (!jobWatch) {
            // qCDebug(KJOBWIDGETS) << "deleted out from under us when asking the server proxy for the view";
            jobView->terminate(QString());
            delete jobView;
            return;
        }

        QObject::connect(jobView, SIGNAL(cancelRequested()), this, SLOT(_k_killJob()));
        QObject::connect(jobView, &org::kde::JobViewV2::suspendRequested, job, &KJob::suspend);
        QObject::connect(jobView, &org::kde::JobViewV2::resumeRequested, job, &KJob::resume);

        d->updateDestUrl(job, jobView);

        if (!jobWatch) {
            // qCDebug(KJOBWIDGETS) << "deleted out from under us when creating the dbus interface";
            jobView->terminate(QString());
            delete jobView;
            return;
        }

        d->progressJobView.insert(job, jobView);
    } else if (!jobWatch) {
        qWarning() << "Uh-oh...KUiServerJobTracker was trying to forward a job, but it was deleted from under us."
                   << "kuiserver *may* have a stranded job. we can't do anything about it because the returned objectPath is invalid.";
        return;
    }

    KJobTrackerInterface::registerJob(job);
}

void KUiServerJobTracker::unregisterJob(KJob *job)
{
    KJobTrackerInterface::unregisterJob(job);

    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobViewV2 *jobView = d->progressJobView.take(job);

    d->updateDestUrl(job, jobView);

    jobView->setError(job->error());

    if (job->error()) {
        jobView->terminate(job->errorText());
    } else {
        jobView->terminate(QString());
    }

    delete jobView;
}

void KUiServerJobTracker::finished(KJob *job)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobViewV2 *jobView = d->progressJobView.take(job);

    d->updateDestUrl(job, jobView);

    jobView->setError(job->error());

    if (job->error()) {
        jobView->terminate(job->errorText());
    } else {
        jobView->terminate(QString());
    }
}

void KUiServerJobTracker::suspended(KJob *job)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobViewV2 *jobView = d->progressJobView[job];

    jobView->setSuspended(true);
}

void KUiServerJobTracker::resumed(KJob *job)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobViewV2 *jobView = d->progressJobView[job];

    jobView->setSuspended(false);
}

void KUiServerJobTracker::description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobViewV2 *jobView = d->progressJobView[job];

    jobView->setInfoMessage(title);

    if (field1.first.isNull() || field1.second.isNull()) {
        jobView->clearDescriptionField(0);
    } else {
        jobView->setDescriptionField(0, field1.first, field1.second);
    }

    if (field2.first.isNull() || field2.second.isNull()) {
        jobView->clearDescriptionField(1);
    } else {
        jobView->setDescriptionField(1, field2.first, field2.second);
    }
}

void KUiServerJobTracker::infoMessage(KJob *job, const QString &plain, const QString &rich)
{
    Q_UNUSED(rich)

    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobViewV2 *jobView = d->progressJobView[job];

    jobView->setInfoMessage(plain);
}

void KUiServerJobTracker::totalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobViewV2 *jobView = d->progressJobView[job];

    switch (unit) {
    case KJob::Bytes:
        jobView->setTotalAmount(amount, QStringLiteral("bytes"));
        break;
    case KJob::Files:
        jobView->setTotalAmount(amount, QStringLiteral("files"));
        break;
    case KJob::Directories:
        jobView->setTotalAmount(amount, QStringLiteral("dirs"));
        break;
    case KJob::Items:
        jobView->setTotalAmount(amount, QStringLiteral("items"));
        break;
    case KJob::UnitsCount:
        Q_UNREACHABLE();
        break;
    }
}

void KUiServerJobTracker::processedAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobViewV2 *jobView = d->progressJobView[job];

    switch (unit) {
    case KJob::Bytes:
        jobView->setProcessedAmount(amount, QStringLiteral("bytes"));
        break;
    case KJob::Files:
        jobView->setProcessedAmount(amount, QStringLiteral("files"));
        break;
    case KJob::Directories:
        jobView->setProcessedAmount(amount, QStringLiteral("dirs"));
        break;
    case KJob::Items:
        jobView->setProcessedAmount(amount, QStringLiteral("items"));
        break;
    case KJob::UnitsCount:
        Q_UNREACHABLE();
        break;
    }
}

void KUiServerJobTracker::percent(KJob *job, unsigned long percent)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobViewV2 *jobView = d->progressJobView[job];

    jobView->setPercent(percent);
}

void KUiServerJobTracker::speed(KJob *job, unsigned long value)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobViewV2 *jobView = d->progressJobView[job];

    jobView->setSpeed(value);
}

KSharedUiServerProxy::KSharedUiServerProxy()
    : m_uiserver(new org::kde::JobViewServer(QStringLiteral("org.kde.JobViewServer"), QStringLiteral("/JobViewServer"), QDBusConnection::sessionBus()))
    , m_watcher(new QDBusServiceWatcher(QStringLiteral("org.kde.JobViewServer"), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange))
{
    QDBusConnectionInterface *bus = QDBusConnection::sessionBus().interface();
    if (!bus->isServiceRegistered(QStringLiteral("org.kde.JobViewServer"))) {
        QDBusReply<void> reply = bus->startService(QStringLiteral("org.kde.kuiserver"));
        if (!reply.isValid()) {
            qCCritical(KJOBWIDGETS) << "Couldn't start kuiserver from org.kde.kuiserver.service:" << reply.error();
            return;
        }

        if (!bus->isServiceRegistered(QStringLiteral("org.kde.JobViewServer"))) {
            qCDebug(KJOBWIDGETS) << "The dbus name org.kde.JobViewServer is STILL NOT REGISTERED, even after starting kuiserver. Should not happen.";
            return;
        }

        qCDebug(KJOBWIDGETS) << "kuiserver registered";
    } else {
        qCDebug(KJOBWIDGETS) << "kuiserver found";
    }

    connect(m_watcher.get(), &QDBusServiceWatcher::serviceOwnerChanged, this, &KSharedUiServerProxy::uiserverOwnerChanged);

    // cleanup early enough to avoid issues with dbus at application exit
    // see e.g. https://phabricator.kde.org/D2545
    qAddPostRoutine([]() {
        serverProxy->m_uiserver.reset();
        serverProxy->m_watcher.reset();
    });
}

KSharedUiServerProxy::~KSharedUiServerProxy()
{
}

org::kde::JobViewServer *KSharedUiServerProxy::uiserver()
{
    return m_uiserver.get();
}

void KSharedUiServerProxy::uiserverOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(serviceName);
    Q_UNUSED(oldOwner);

    if (!newOwner.isEmpty()) { // registered
        Q_EMIT serverRegistered();
    } else if (newOwner.isEmpty()) { // unregistered
        Q_EMIT serverUnregistered();
    }
}

#include "moc_kuiserverjobtracker.cpp"
#include "moc_kuiserverjobtracker_p.cpp"
