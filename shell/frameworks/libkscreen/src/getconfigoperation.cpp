/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "getconfigoperation.h"
#include "backendinterface.h"
#include "backendmanager_p.h"
#include "config.h"
#include "configoperation_p.h"
#include "configserializer_p.h"
#include "log.h"
#include "output.h"

using namespace KScreen;

namespace KScreen
{
class GetConfigOperationPrivate : public ConfigOperationPrivate
{
    Q_OBJECT

public:
    GetConfigOperationPrivate(GetConfigOperation::Options options, GetConfigOperation *qq);

    void backendReady(org::kde::kscreen::Backend *backend) override;
    void onConfigReceived(QDBusPendingCallWatcher *watcher);
    void onEDIDReceived(QDBusPendingCallWatcher *watcher);

public:
    GetConfigOperation::Options options;
    ConfigPtr config;
    // For in-process
    void loadEdid(KScreen::AbstractBackend *backend);

    // For out-of-process
    int pendingEDIDs;
    QPointer<org::kde::kscreen::Backend> mBackend;

private:
    Q_DECLARE_PUBLIC(GetConfigOperation)
};

}

GetConfigOperationPrivate::GetConfigOperationPrivate(GetConfigOperation::Options options, GetConfigOperation *qq)
    : ConfigOperationPrivate(qq)
    , options(options)
{
}

void GetConfigOperationPrivate::backendReady(org::kde::kscreen::Backend *backend)
{
    Q_ASSERT(BackendManager::instance()->method() == BackendManager::OutOfProcess);
    ConfigOperationPrivate::backendReady(backend);

    Q_Q(GetConfigOperation);

    if (!backend) {
        q->setError(tr("Failed to prepare backend"));
        q->emitResult();
        return;
    }

    mBackend = backend;
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(mBackend->getConfig(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &GetConfigOperationPrivate::onConfigReceived);
}

void GetConfigOperationPrivate::onConfigReceived(QDBusPendingCallWatcher *watcher)
{
    Q_ASSERT(BackendManager::instance()->method() == BackendManager::OutOfProcess);
    Q_Q(GetConfigOperation);

    QDBusPendingReply<QVariantMap> reply = *watcher;
    watcher->deleteLater();
    if (reply.isError()) {
        q->setError(reply.error().message());
        q->emitResult();
        return;
    }

    config = ConfigSerializer::deserializeConfig(reply.value());
    if (!config) {
        q->setError(tr("Failed to deserialize backend response"));
        q->emitResult();
        return;
    }

    if (options & GetConfigOperation::NoEDID || config->outputs().isEmpty()) {
        q->emitResult();
        return;
    }

    pendingEDIDs = 0;
    if (!mBackend) {
        q->setError(tr("Backend invalidated"));
        q->emitResult();
        return;
    }
    const auto outputs = config->outputs();
    for (const OutputPtr &output : outputs) {
        if (!output->isConnected()) {
            continue;
        }

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(mBackend->getEdid(output->id()), this);
        watcher->setProperty("outputId", output->id());
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &GetConfigOperationPrivate::onEDIDReceived);
        ++pendingEDIDs;
    }
}

void GetConfigOperationPrivate::onEDIDReceived(QDBusPendingCallWatcher *watcher)
{
    Q_ASSERT(BackendManager::instance()->method() == BackendManager::OutOfProcess);
    Q_Q(GetConfigOperation);

    QDBusPendingReply<QByteArray> reply = *watcher;
    watcher->deleteLater();
    if (reply.isError()) {
        q->setError(reply.error().message());
        q->emitResult();
        return;
    }

    const QByteArray edidData = reply.value();
    const int outputId = watcher->property("outputId").toInt();

    config->output(outputId)->setEdid(edidData);
    if (--pendingEDIDs == 0) {
        q->emitResult();
    }
}

GetConfigOperation::GetConfigOperation(Options options, QObject *parent)
    : ConfigOperation(new GetConfigOperationPrivate(options, this), parent)
{
}

GetConfigOperation::~GetConfigOperation()
{
}

KScreen::ConfigPtr GetConfigOperation::config() const
{
    Q_D(const GetConfigOperation);
    return d->config;
}

void GetConfigOperation::start()
{
    Q_D(GetConfigOperation);
    if (BackendManager::instance()->method() == BackendManager::InProcess) {
        auto backend = d->loadBackend();
        if (!backend) {
            return; // loadBackend() already set error and called emitResult() for us
        }
        d->config = backend->config()->clone();
        d->loadEdid(backend);
        emitResult();
    } else {
        d->requestBackend();
    }
}

void GetConfigOperationPrivate::loadEdid(KScreen::AbstractBackend *backend)
{
    Q_ASSERT(BackendManager::instance()->method() == BackendManager::InProcess);
    if (options & KScreen::ConfigOperation::NoEDID) {
        return;
    }
    if (!config) {
        return;
    }
    auto outputs = config->outputs();
    for (auto it = outputs.begin(); it != outputs.end(); ++it) {
        auto output = *it;
        if (output->edid() == nullptr) {
            const QByteArray edidData = backend->edid(output->id());
            output->setEdid(edidData);
        }
    }
}

#include "getconfigoperation.moc"

#include "moc_getconfigoperation.cpp"
