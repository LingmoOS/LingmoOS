/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "setconfigoperation.h"

#include "abstractbackend.h"
#include "backendmanager_p.h"
#include "config.h"
#include "configoperation_p.h"
#include "configserializer_p.h"
#include "kscreen_debug.h"
#include "output.h"

#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>

using namespace KScreen;

namespace KScreen
{
class SetConfigOperationPrivate : public ConfigOperationPrivate
{
    Q_OBJECT

public:
    explicit SetConfigOperationPrivate(const KScreen::ConfigPtr &config, ConfigOperation *qq);

    void backendReady(org::kde::kscreen::Backend *backend) override;
    void onConfigSet(QDBusPendingCallWatcher *watcher);
    void normalizeOutputPositions();
    void fixPrimaryOutput();

    KScreen::ConfigPtr config;

private:
    Q_DECLARE_PUBLIC(SetConfigOperation)
};

}

SetConfigOperationPrivate::SetConfigOperationPrivate(const ConfigPtr &config, ConfigOperation *qq)
    : ConfigOperationPrivate(qq)
    , config(config)
{
}

void SetConfigOperationPrivate::backendReady(org::kde::kscreen::Backend *backend)
{
    ConfigOperationPrivate::backendReady(backend);

    Q_Q(SetConfigOperation);

    if (!backend) {
        q->setError(tr("Failed to prepare backend"));
        q->emitResult();
        return;
    }

    const QVariantMap map = ConfigSerializer::serializeConfig(config).toVariantMap();
    if (map.isEmpty()) {
        q->setError(tr("Failed to serialize request"));
        q->emitResult();
        return;
    }

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(backend->setConfig(map), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &SetConfigOperationPrivate::onConfigSet);
}

void SetConfigOperationPrivate::onConfigSet(QDBusPendingCallWatcher *watcher)
{
    Q_Q(SetConfigOperation);

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
    }

    q->emitResult();
}

SetConfigOperation::SetConfigOperation(const ConfigPtr &config, QObject *parent)
    : ConfigOperation(new SetConfigOperationPrivate(config, this), parent)
{
}

SetConfigOperation::~SetConfigOperation()
{
}

ConfigPtr SetConfigOperation::config() const
{
    Q_D(const SetConfigOperation);
    return d->config;
}

void SetConfigOperation::start()
{
    Q_D(SetConfigOperation);
    d->normalizeOutputPositions();
    d->fixPrimaryOutput();
    if (BackendManager::instance()->method() == BackendManager::InProcess) {
        auto backend = d->loadBackend();
        backend->setConfig(d->config);
        emitResult();
    } else {
        d->requestBackend();
    }
}

void SetConfigOperationPrivate::normalizeOutputPositions()
{
    if (!config) {
        return;
    }
    int offsetX = INT_MAX;
    int offsetY = INT_MAX;
    const auto outputs = config->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isPositionable()) {
            continue;
        }
        offsetX = qMin(output->pos().x(), offsetX);
        offsetY = qMin(output->pos().y(), offsetY);
    }

    if (!offsetX && !offsetY) {
        return;
    }
    qCDebug(KSCREEN) << "Correcting output positions by:" << QPoint(offsetX, offsetY);
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isConnected() || !output->isEnabled()) {
            continue;
        }
        QPoint newPos = QPoint(output->pos().x() - offsetX, output->pos().y() - offsetY);
        qCDebug(KSCREEN) << "Moved output from" << output->pos() << "to" << newPos;
        output->setPos(newPos);
    }
}

void SetConfigOperationPrivate::fixPrimaryOutput()
{
    if (!config || !(config->supportedFeatures() & Config::Feature::PrimaryDisplay)) {
        return;
    }
    const auto outputs = config->outputs();
    if (outputs.isEmpty()) {
        return;
    }

    // Here we make sure that:
    // - that our primary is enabled
    // - we have at least a primary
    // - we have exactly 1 primary
    // - we have a primary at all
    bool found = false;
    KScreen::OutputPtr primary;
    KScreen::OutputPtr candidate;
    for (const KScreen::OutputPtr &output : outputs) {
        if (output->isPrimary()) {
            if (!output->isEnabled()) {
                qCDebug(KSCREEN) << "can't be primary if disabled!!" << output;
            } else if (found) {
                qCDebug(KSCREEN) << "can only have 1 primary" << output;
            } else {
                found = true;
                primary = output;
            }
        } else if (output->isEnabled()) {
            candidate = output;
        }
    }

    if (!found && candidate) {
        qCDebug(KSCREEN) << "setting primary instead" << candidate;
        config->setPrimaryOutput(candidate);
    } else if (primary) {
        // ensures all others are set to non-primary. It's OK if all outputs
        // are disabled and `primary` is essentially nullptr.
        config->setPrimaryOutput(primary);
    }
}

#include "setconfigoperation.moc"

#include "moc_setconfigoperation.cpp"
