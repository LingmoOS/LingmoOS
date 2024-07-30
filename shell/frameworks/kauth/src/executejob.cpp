/*
    SPDX-FileCopyrightText: 2009-2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "executejob.h"

#include "BackendsManager.h"
#include "kauthdebug.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QHash>
#include <QTimer>

namespace KAuth
{
class ExecuteJobPrivate
{
    Q_DECLARE_TR_FUNCTIONS(KAuth::ExecuteJob)

public:
    explicit ExecuteJobPrivate(ExecuteJob *parent)
        : q(parent)
    {
    }

    ExecuteJob *q;
    Action action;

    Action::ExecutionMode mode;
    QVariantMap data;

    void doExecuteAction();
    void doAuthorizeAction();
    void actionPerformedSlot(const QString &action, const ActionReply &reply);
    void progressStepSlot(const QString &action, int i);
    void progressStepSlot(const QString &action, const QVariantMap &data);
    void statusChangedSlot(const QString &action, KAuth::Action::AuthStatus status);
};

ExecuteJob::ExecuteJob(const Action &action, Action::ExecutionMode mode, QObject *parent)
    : KJob(parent)
    , d(new ExecuteJobPrivate(this))
{
    d->action = action;
    d->mode = mode;

    HelperProxy *helper = BackendsManager::helperProxy();

    connect(helper, &KAuth::HelperProxy::actionPerformed, this, [this](const QString &action, const ActionReply &reply) {
        d->actionPerformedSlot(action, reply);
    });
    connect(helper, &KAuth::HelperProxy::progressStep, this, [this](const QString &action, int i) {
        d->progressStepSlot(action, i);
    });
    connect(helper, &KAuth::HelperProxy::progressStepData, this, [this](const QString &action, const QVariantMap &data) {
        d->progressStepSlot(action, data);
    });

    connect(BackendsManager::authBackend(), &KAuth::AuthBackend::actionStatusChanged, this, [this](const QString &action, Action::AuthStatus status) {
        d->statusChangedSlot(action, status);
    });
}

ExecuteJob::~ExecuteJob() = default;

Action ExecuteJob::action() const
{
    return d->action;
}

QVariantMap ExecuteJob::data() const
{
    return d->data;
}

void ExecuteJob::start()
{
    if (!d->action.isValid()) {
        qCWarning(KAUTH) << "Tried to start an invalid action: " << d->action.name();
        ActionReply reply(ActionReply::InvalidActionError);
        reply.setErrorDescription(tr("Tried to start an invalid action"));
        d->actionPerformedSlot(d->action.name(), reply);
        return;
    }

    switch (d->mode) {
    case Action::ExecuteMode:
        QTimer::singleShot(0, this, [this]() {
            d->doExecuteAction();
        });
        break;
    case Action::AuthorizeOnlyMode:
        QTimer::singleShot(0, this, [this]() {
            d->doAuthorizeAction();
        });
        break;
    default: {
        ActionReply reply(ActionReply::InvalidActionError);
        reply.setErrorDescription(tr("Unknown execution mode chosen"));
        d->actionPerformedSlot(d->action.name(), reply);
        break;
    }
    }
}

bool ExecuteJob::kill(KillVerbosity verbosity)
{
    BackendsManager::helperProxy()->stopAction(d->action.name(), d->action.helperId());
    KJob::kill(verbosity);
    return true;
}

void ExecuteJobPrivate::doExecuteAction()
{
    // If this action authorizes from the client, let's do it now
    if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromClientCapability) {
        if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::PreAuthActionCapability) {
            BackendsManager::authBackend()->preAuthAction(action.name(), action.parentWindow());
        }

        Action::AuthStatus s = BackendsManager::authBackend()->authorizeAction(action.name());

        if (s == Action::AuthorizedStatus) {
            if (action.hasHelper()) {
                BackendsManager::helperProxy()->executeAction(action.name(), action.helperId(), action.detailsV2(), action.arguments(), action.timeout());
            } else {
                // Done
                actionPerformedSlot(action.name(), ActionReply::SuccessReply());
            }
        } else {
            // Abort if authorization fails
            switch (s) {
            case Action::DeniedStatus:
                actionPerformedSlot(action.name(), ActionReply::AuthorizationDeniedReply());
                break;
            case Action::InvalidStatus:
                actionPerformedSlot(action.name(), ActionReply::InvalidActionReply());
                break;
            case Action::UserCancelledStatus:
                actionPerformedSlot(action.name(), ActionReply::UserCancelledReply());
                break;
            default: {
                ActionReply r(ActionReply::BackendError);
                r.setErrorDescription(tr("Unknown status for the authentication procedure"));
                actionPerformedSlot(action.name(), r);
                break;
            }
            }
        }
    } else if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromHelperCapability) {
        if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::PreAuthActionCapability) {
            BackendsManager::authBackend()->preAuthAction(action.name(), action.parentWindow());
        }
        if (!action.hasHelper()) {
            ActionReply r(ActionReply::InvalidActionReply());
            r.setErrorDescription(tr("The current backend only allows helper authorization, but this action does not have a helper."));
            actionPerformedSlot(action.name(), r);
            return;
        }
        BackendsManager::helperProxy()->executeAction(action.name(), action.helperId(), action.detailsV2(), action.arguments(), action.timeout());
    } else {
        // There's something totally wrong here
        ActionReply r(ActionReply::BackendError);
        r.setErrorDescription(tr("The backend does not specify how to authorize"));
        actionPerformedSlot(action.name(), r);
    }
}

void ExecuteJobPrivate::doAuthorizeAction()
{
    // Check the status first
    Action::AuthStatus s = action.status();
    if (s == Action::AuthRequiredStatus) {
        // Let's check what to do
        if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromClientCapability) {
            // In this case we can actually try an authorization
            if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::PreAuthActionCapability) {
                BackendsManager::authBackend()->preAuthAction(action.name(), action.parentWindow());
            }

            s = BackendsManager::authBackend()->authorizeAction(action.name());
        } else if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromHelperCapability) {
            // In this case, just throw out success, as the auth will take place later
            s = Action::AuthorizedStatus;
        } else {
            // This should never, never happen
            ActionReply r(ActionReply::BackendError);
            r.setErrorDescription(tr("The backend does not specify how to authorize"));
            actionPerformedSlot(action.name(), r);
        }
    }

    // Return based on the current status
    if (s == Action::AuthorizedStatus) {
        actionPerformedSlot(action.name(), ActionReply::SuccessReply());
    } else {
        actionPerformedSlot(action.name(), ActionReply::AuthorizationDeniedReply());
    }
}

void ExecuteJobPrivate::actionPerformedSlot(const QString &taction, const ActionReply &reply)
{
    if (taction == action.name()) {
        if (reply.failed()) {
            q->setError(reply.errorCode());
            q->setErrorText(reply.errorDescription());
        } else {
            data = reply.data();
        }

        q->emitResult();
    }
}

void ExecuteJobPrivate::progressStepSlot(const QString &taction, int i)
{
    if (taction == action.name()) {
        q->setPercent(i);
    }
}

void ExecuteJobPrivate::progressStepSlot(const QString &taction, const QVariantMap &data)
{
    if (taction == action.name()) {
        Q_EMIT q->newData(data);
    }
}

void ExecuteJobPrivate::statusChangedSlot(const QString &taction, Action::AuthStatus status)
{
    if (taction == action.name()) {
        Q_EMIT q->statusChanged(status);
    }
}

} // namespace Auth

#include "moc_executejob.cpp"
