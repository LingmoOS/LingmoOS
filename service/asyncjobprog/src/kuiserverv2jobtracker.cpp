/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kuiserverv2jobtracker.h"
#include "kuiserverv2jobtracker_p.h"

#include "jobviewv3iface.h"
#include "debug.h"

#include <KJob>

#include <QtGlobal>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QGuiApplication>
#include <QTimer>
#include <QHash>
#include <QVariantMap>

Q_GLOBAL_STATIC(KSharedUiServerV2Proxy, serverProxy)

struct JobView
{
    QTimer *delayTimer = nullptr;
    org::kde::JobViewV3 *jobView = nullptr;
    QVariantMap currentState;
    QVariantMap pendingUpdates;
};

class KUiServerV2JobTrackerPrivate
{
public:
    KUiServerV2JobTrackerPrivate(KUiServerV2JobTracker *parent)
        : q(parent)
    {
        updateTimer.setInterval(0);
        updateTimer.setSingleShot(true);
        QObject::connect(&updateTimer, &QTimer::timeout, q, [this] {
            sendAllUpdates();
        });
    }

    KUiServerV2JobTracker *const q;

    void sendAllUpdates();
    void sendUpdate(JobView &view);
    void scheduleUpdate(KJob *job, const QString &key, const QVariant &value);

    void updateDestUrl(KJob *job);

    void requestView(KJob *job, const QString &desktopEntry);

    QHash<KJob *, JobView> jobViews;
    QTimer updateTimer;

    QMetaObject::Connection serverRegisteredConnection;
};

void KUiServerV2JobTrackerPrivate::scheduleUpdate(KJob *job, const QString &key, const QVariant &value)
{
    auto &view = jobViews[job];
    view.currentState[key] = value;
    view.pendingUpdates[key] = value;

    if (!updateTimer.isActive()) {
        updateTimer.start();
    }
}

void KUiServerV2JobTrackerPrivate::sendAllUpdates()
{
    for (auto it = jobViews.begin(), end = jobViews.end(); it != end; ++it) {
        sendUpdate(it.value());
    }
}

void KUiServerV2JobTrackerPrivate::sendUpdate(JobView &view)
{
    if (!view.jobView) {
        return;
    }

    const QVariantMap updates = view.pendingUpdates;
    if (updates.isEmpty()) {
        return;
    }

    view.jobView->update(updates);
    view.pendingUpdates.clear();
}

void KUiServerV2JobTrackerPrivate::updateDestUrl(KJob *job)
{
    scheduleUpdate(job, QStringLiteral("destUrl"), job->property("destUrl").toString());
}

void KUiServerV2JobTrackerPrivate::requestView(KJob *job, const QString &desktopEntry)
{
    QPointer<KJob> jobGuard = job;
    auto &view = jobViews[job];

    QVariantMap hints = view.currentState;
    // Tells Plasma to show the job view right away, since the delay is always handled on our side
    hints.insert(QStringLiteral("immediate"), true);
    // Must not clear currentState as only Plasma 5.22+ will use properties from "hints",
    // there must still be a full update() call for earlier versions!

    if (job->isFinishedNotificationHidden()) {
        hints.insert(QStringLiteral("transient"), true);
    }

    auto reply = serverProxy()->uiserver()->requestView(desktopEntry, job->capabilities(), hints);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, q);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, q, [this, watcher, jobGuard, job] {
        QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError()) {
            qCWarning(KJOBWIDGETS) << "Failed to register job with KUiServerV2JobTracker" << reply.error().message();
            jobViews.remove(job);
            return;
        }

        const QString viewObjectPath = reply.value().path();
        auto *jobView = new org::kde::JobViewV3(QStringLiteral("org.kde.JobViewServer"), viewObjectPath, QDBusConnection::sessionBus());

        auto &view = jobViews[job];

        if (jobGuard) {
            QObject::connect(jobView, &org::kde::JobViewV3::cancelRequested, job, [job] {
                job->kill(KJob::EmitResult);
            });
            QObject::connect(jobView, &org::kde::JobViewV3::suspendRequested, job, &KJob::suspend);
            QObject::connect(jobView, &org::kde::JobViewV3::resumeRequested, job, &KJob::resume);

            view.jobView = jobView;
        }

        // Now send the full current job state over
        jobView->update(view.currentState);
        // which also contains all pending updates
        view.pendingUpdates.clear();

        // Job was deleted or finished in the meantime
        if (!jobGuard || view.currentState.value(QStringLiteral("terminated")).toBool()) {
            const uint errorCode = view.currentState.value(QStringLiteral("errorCode")).toUInt();
            const QString errorMessage = view.currentState.value(QStringLiteral("errorMessage")).toString();

            jobView->terminate(errorCode, errorMessage, QVariantMap() /*hints*/);
            delete jobView;

            jobViews.remove(job);
        }
    });
}

KUiServerV2JobTracker::KUiServerV2JobTracker(QObject *parent)
    : KJobTrackerInterface(parent)
    , d(new KUiServerV2JobTrackerPrivate(this))
{
    qDBusRegisterMetaType<qulonglong>();
}

KUiServerV2JobTracker::~KUiServerV2JobTracker()
{
    if (!d->jobViews.isEmpty()) {
        qCWarning(KJOBWIDGETS) << "A KUiServerV2JobTracker instance contains"
                               << d->jobViews.size() << "stalled jobs";
    }

    delete d;
}

void KUiServerV2JobTracker::registerJob(KJob *job)
{
    if (d->jobViews.contains(job)) {
        return;
    }

    QString desktopEntry = job->property("desktopFileName").toString();
    if (desktopEntry.isEmpty()) {
        desktopEntry = QGuiApplication::desktopFileName();
    }

    if (desktopEntry.isEmpty()) {
        qCWarning(KJOBWIDGETS) << "Cannot register a job with KUiServerV2JobTracker without QGuiApplication::desktopFileName";
        return;
    }

    // Watch the server registering/unregistering and re-register the jobs as needed
    if (!d->serverRegisteredConnection) {
        d->serverRegisteredConnection = connect(serverProxy(), &KSharedUiServerV2Proxy::serverRegistered, this, [this]() {
            const auto staleViews = d->jobViews;

            // Delete the old views, remove the old struct but keep the state,
            // register the job again (which checks for presence, hence removing first)
            // and then restore its previous state, which is safe because the DBus
            // is async and is only processed once event loop returns
            for (auto it = staleViews.begin(), end = staleViews.end(); it != end; ++it) {
                KJob *job = it.key();
                const JobView &view = it.value();

                const auto oldState = view.currentState;

                // It is possible that the KJob has been deleted already so do not
                // use or deference if marked as terminated
                if (oldState.value(QStringLiteral("terminated")).toBool()) {
                    const uint errorCode = oldState.value(QStringLiteral("errorCode")).toUInt();
                    const QString errorMessage = oldState.value(QStringLiteral("errorMessage")).toString();

                    if (view.jobView) {
                        view.jobView->terminate(errorCode, errorMessage, QVariantMap() /*hints*/);
                    }

                    delete view.jobView;
                    d->jobViews.remove(job);
                } else {
                    delete view.jobView;
                    d->jobViews.remove(job); // must happen before registerJob

                    registerJob(job);

                    d->jobViews[job].currentState = oldState;
                }
            }
        });
    }

    // Send along current job state
    if (job->isSuspended()) {
        suspended(job);
    }
    if (job->error()) {
        d->scheduleUpdate(job, QStringLiteral("errorCode"), static_cast<uint>(job->error()));
        d->scheduleUpdate(job, QStringLiteral("errorMessage"), job->errorText());
    }
    for (int i = KJob::Bytes; i <= KJob::Items; ++i) {
        const auto unit = static_cast<KJob::Unit>(i);

        if (job->processedAmount(unit) > 0) {
            processedAmount(job, unit, job->processedAmount(unit));
        }
        if (job->totalAmount(unit) > 0) {
            totalAmount(job, unit, job->totalAmount(unit));
        }
    }
    if (job->percent() > 0) {
        percent(job, job->percent());
    }
    d->updateDestUrl(job);

    if (job->property("immediateProgressReporting").toBool()) {
        d->requestView(job, desktopEntry);
    } else {
        QPointer<KJob> jobGuard = job;

        QTimer *delayTimer = new QTimer();
        delayTimer->setSingleShot(true);
        connect(delayTimer, &QTimer::timeout, this, [this, job, jobGuard, desktopEntry] {
            auto &view = d->jobViews[job];
            if (view.delayTimer) {
                view.delayTimer->deleteLater();
                view.delayTimer = nullptr;
            }

            if (jobGuard) {
                d->requestView(job, desktopEntry);
            }
        });

        d->jobViews[job].delayTimer = delayTimer;
        delayTimer->start(500);
    }

    KJobTrackerInterface::registerJob(job);
}

void KUiServerV2JobTracker::unregisterJob(KJob *job)
{
    KJobTrackerInterface::unregisterJob(job);
    finished(job);
}

void KUiServerV2JobTracker::finished(KJob *job)
{
    d->updateDestUrl(job);

    // send all pending updates before terminating to ensure state is correct
    auto &view = d->jobViews[job];
    d->sendUpdate(view);

    if (view.delayTimer) {
        delete view.delayTimer;
        d->jobViews.remove(job);
    } else if (view.jobView) {
        view.jobView->terminate(static_cast<uint>(job->error()),
                                job->error() ? job->errorText() : QString(),
                                QVariantMap() /*hints*/);
        delete view.jobView;
        d->jobViews.remove(job);
    } else {
        // Remember that the job finished in the meantime and
        // terminate the JobView once it arrives
        d->scheduleUpdate(job, QStringLiteral("terminated"), true);
        if (job->error()) {
            d->scheduleUpdate(job, QStringLiteral("errorCode"), static_cast<uint>(job->error()));
            d->scheduleUpdate(job, QStringLiteral("errorMessage"),  job->errorText());
        }
    }
}

void KUiServerV2JobTracker::suspended(KJob *job)
{
    d->scheduleUpdate(job, QStringLiteral("suspended"), true);
}

void KUiServerV2JobTracker::resumed(KJob *job)
{
    d->scheduleUpdate(job, QStringLiteral("suspended"), false);
}

void KUiServerV2JobTracker::description(KJob *job, const QString &title,
                                      const QPair<QString, QString> &field1,
                                      const QPair<QString, QString> &field2)
{
    d->scheduleUpdate(job, QStringLiteral("title"), title);

    d->scheduleUpdate(job, QStringLiteral("descriptionLabel1"), field1.first);
    d->scheduleUpdate(job, QStringLiteral("descriptionValue1"), field1.second);

    d->scheduleUpdate(job, QStringLiteral("descriptionLabel2"), field2.first);
    d->scheduleUpdate(job, QStringLiteral("descriptionValue2"), field2.second);
}

void KUiServerV2JobTracker::infoMessage(KJob *job, const QString &plain, const QString &rich)
{
    Q_UNUSED(rich);
    d->scheduleUpdate(job, QStringLiteral("infoMessage"), plain);
}

void KUiServerV2JobTracker::totalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    switch (unit) {
    case KJob::Bytes:
        d->scheduleUpdate(job, QStringLiteral("totalBytes"), amount);
        break;
    case KJob::Files:
        d->scheduleUpdate(job, QStringLiteral("totalFiles"), amount);
        break;
    case KJob::Directories:
        d->scheduleUpdate(job, QStringLiteral("totalDirectories"), amount);
        break;
    case KJob::Items:
        d->scheduleUpdate(job, QStringLiteral("totalItems"), amount);
        break;
    case KJob::UnitsCount:
        Q_UNREACHABLE();
        break;
    }
}

void KUiServerV2JobTracker::processedAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    switch (unit) {
    case KJob::Bytes:
        d->scheduleUpdate(job, QStringLiteral("processedBytes"), amount);
        break;
    case KJob::Files:
        d->scheduleUpdate(job, QStringLiteral("processedFiles"), amount);
        break;
    case KJob::Directories:
        d->scheduleUpdate(job, QStringLiteral("processedDirectories"), amount);
        break;
    case KJob::Items:
        d->scheduleUpdate(job, QStringLiteral("processedItems"), amount);
        break;
    case KJob::UnitsCount:
        Q_UNREACHABLE();
        break;
    }
}

void KUiServerV2JobTracker::percent(KJob *job, unsigned long percent)
{
    d->scheduleUpdate(job, QStringLiteral("percent"), static_cast<uint>(percent));
}

void KUiServerV2JobTracker::speed(KJob *job, unsigned long speed)
{
    d->scheduleUpdate(job, QStringLiteral("speed"), static_cast<qulonglong>(speed));
}

KSharedUiServerV2Proxy::KSharedUiServerV2Proxy()
    : m_uiserver(new org::kde::JobViewServerV2(QStringLiteral("org.kde.JobViewServer"), QStringLiteral("/JobViewServer"), QDBusConnection::sessionBus()))
    , m_watcher(new QDBusServiceWatcher(QStringLiteral("org.kde.JobViewServer"), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange))
{
    connect(m_watcher.get(), &QDBusServiceWatcher::serviceOwnerChanged, this, &KSharedUiServerV2Proxy::uiserverOwnerChanged);

    // cleanup early enough to avoid issues with dbus at application exit
    // see e.g. https://phabricator.kde.org/D2545
    qAddPostRoutine([]() {
        serverProxy->m_uiserver.reset();
        serverProxy->m_watcher.reset();
    });
}

KSharedUiServerV2Proxy::~KSharedUiServerV2Proxy()
{

}

org::kde::JobViewServerV2 *KSharedUiServerV2Proxy::uiserver()
{
    return m_uiserver.get();
}

void KSharedUiServerV2Proxy::uiserverOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(serviceName);
    Q_UNUSED(oldOwner);

    if (!newOwner.isEmpty()) { // registered
        Q_EMIT serverRegistered();
    } else if (newOwner.isEmpty()) { // unregistered
        Q_EMIT serverUnregistered();
    }
}

#include "moc_kuiserverv2jobtracker.cpp"
#include "moc_kuiserverv2jobtracker_p.cpp"
