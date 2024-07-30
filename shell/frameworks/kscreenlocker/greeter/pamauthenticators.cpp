/*
    SPDX-FileCopyrightText: 2023 Janet Blackquill <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QDebug>
#include <algorithm>

#include "kscreenlocker_greet_logging.h"
#include "pamauthenticator.h"
#include "pamauthenticators.h"

struct PamAuthenticators::Private {
    std::unique_ptr<PamAuthenticator> interactive;
    std::vector<std::unique_ptr<PamAuthenticator>> noninteractive;
    PamAuthenticator::NoninteractiveAuthenticatorTypes computedTypes = PamAuthenticator::NoninteractiveAuthenticatorType::None;
    AuthenticatorsState state = AuthenticatorsState::Idle;
    bool graceLocked = false;
    bool hadPrompt = false;

    void recomputeNoninteractiveAuthenticationTypes()
    {
        PamAuthenticator::NoninteractiveAuthenticatorTypes result = PamAuthenticator::NoninteractiveAuthenticatorType::None;
        for (auto &&noninteractive : noninteractive) {
            if (noninteractive->isAvailable()) {
                result |= noninteractive->authenticatorType();
            }
        }
        computedTypes = result;
    }
    void cancelNoninteractive()
    {
        for (auto &&noninteractive : noninteractive) {
            noninteractive->cancel();
        }
    }
};

PamAuthenticators::PamAuthenticators(std::unique_ptr<PamAuthenticator> &&interactive,
                                     std::vector<std::unique_ptr<PamAuthenticator>> &&noninteractive,
                                     QObject *parent)
    : QObject(parent)
    , d(new Private{std::move(interactive), std::move(noninteractive)})
{
    connect(d->interactive.get(), &PamAuthenticator::succeeded, this, [this] {
        qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Success from interactive authenticator" << qUtf8Printable(d->interactive->service());
        Q_EMIT succeeded();
    });
    connect(d->interactive.get(), &PamAuthenticator::failed, this, [this] {
        qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Failure from interactive authenticator" << qUtf8Printable(d->interactive->service());
        setState(AuthenticatorsState::Idle);
        d->cancelNoninteractive();
        Q_EMIT failed(PamAuthenticator::NoninteractiveAuthenticatorType::None, d->interactive.get());
    });
    for (auto &&noninteractive : d->noninteractive) {
        connect(noninteractive.get(), &PamAuthenticator::succeeded, this, [this, &noninteractive] {
            qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Success from non-interactive authenticator" << qUtf8Printable(noninteractive->service());
            Q_EMIT succeeded();
        });
        connect(noninteractive.get(), &PamAuthenticator::availableChanged, this, [this, &noninteractive] {
            qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Availability changed for non-interactive authenticator"
                                         << qUtf8Printable(noninteractive->service()) << noninteractive->isAvailable();
            d->recomputeNoninteractiveAuthenticationTypes();
            Q_EMIT authenticatorTypesChanged();
        });
        connect(noninteractive.get(), &PamAuthenticator::failed, this, [this, &noninteractive] {
            qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Non-interactive authenticator" << qUtf8Printable(noninteractive->service()) << "failed";
            Q_EMIT failed(noninteractive->authenticatorType(), noninteractive.get());
        });
        connect(noninteractive.get(), &PamAuthenticator::infoMessage, this, [this, &noninteractive]() {
            if (!d->hadPrompt) {
                d->hadPrompt = true;
                Q_EMIT hadPromptChanged();
            }
            qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Info message from non-interactive authenticator" << qUtf8Printable(noninteractive->service());
            Q_EMIT noninteractiveInfo(noninteractive->authenticatorType(), noninteractive.get());
        });
        connect(noninteractive.get(), &PamAuthenticator::errorMessage, this, [this, &noninteractive]() {
            qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Error message from non-interactive authenticator " << qUtf8Printable(noninteractive->service());
            Q_EMIT noninteractiveError(noninteractive->authenticatorType(), noninteractive.get());
        });
    }

    // connect the delegated signals
    connect(d->interactive.get(), &PamAuthenticator::busyChanged, this, [this] {
        qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Interactive authenticator" << qUtf8Printable(d->interactive->service()) << "changed business";
        Q_EMIT busyChanged();
    });
    connect(d->interactive.get(), &PamAuthenticator::prompt, this, [this] {
        if (!d->hadPrompt) {
            d->hadPrompt = true;
            Q_EMIT hadPromptChanged();
        }
        qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Normal prompt from interactive authenticator" << qUtf8Printable(d->interactive->service());
        Q_EMIT promptChanged();
    });
    connect(d->interactive.get(), &PamAuthenticator::promptForSecret, this, [this] {
        if (!d->hadPrompt) {
            d->hadPrompt = true;
            Q_EMIT hadPromptChanged();
        }
        qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Secret prompt from interactive authenticator" << qUtf8Printable(d->interactive->service());
        Q_EMIT promptForSecretChanged();
    });
    connect(d->interactive.get(), &PamAuthenticator::infoMessage, this, [this] {
        qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Info message from interactive authenticator" << qUtf8Printable(d->interactive->service());
        Q_EMIT infoMessageChanged();
    });
    connect(d->interactive.get(), &PamAuthenticator::errorMessage, this, [this] {
        qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: Error message from interactive authenticator" << qUtf8Printable(d->interactive->service());
        Q_EMIT errorMessageChanged();
    });
}

PamAuthenticators::~PamAuthenticators()
{
}

bool PamAuthenticators::isUnlocked() const
{
    return d->interactive->isUnlocked() || std::any_of(d->noninteractive.cbegin(), d->noninteractive.cend(), [](auto &&t) {
               return t->isUnlocked();
           });
}

PamAuthenticators::AuthenticatorsState PamAuthenticators::state() const
{
    return d->state;
}

void PamAuthenticators::startAuthenticating()
{
    if (d->state == AuthenticatorsState::Authenticating || d->graceLocked) {
        return;
    }

    qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: starting authenticators";
    d->interactive->tryUnlock();
    for (auto &&noninteractive : d->noninteractive) {
        noninteractive->tryUnlock();
    }
    setState(AuthenticatorsState::Authenticating);
}

void PamAuthenticators::stopAuthenticating()
{
    qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: stopping authenticators";
    for (auto &&noninteractive : d->noninteractive) {
        noninteractive->cancel();
    }
    d->interactive->cancel();
    setState(AuthenticatorsState::Idle);
}

void PamAuthenticators::setState(AuthenticatorsState state)
{
    if (d->state == state) {
        return;
    }
    qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: state changing from" << d->state << "to" << state;
    d->state = state;
    Q_EMIT stateChanged();
}

// these properties are delegated to interactive authenticator

bool PamAuthenticators::isBusy() const
{
    return d->interactive->isBusy();
}

QString PamAuthenticators::prompt() const
{
    return d->interactive->getPrompt();
}

QString PamAuthenticators::promptForSecret() const
{
    return d->interactive->getPromptForSecret();
}

QString PamAuthenticators::infoMessage() const
{
    return d->interactive->getInfoMessage();
}

QString PamAuthenticators::errorMessage() const
{
    return d->interactive->getErrorMessage();
}

void PamAuthenticators::respond(const QByteArray &response)
{
    qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: responding to interactive authenticator";
    return d->interactive->respond(response);
}

void PamAuthenticators::cancel()
{
    qCDebug(KSCREENLOCKER_GREET) << "PamAuthenticators: cancelling interactive authenticator";
    return d->interactive->cancel();
}

PamAuthenticator::NoninteractiveAuthenticatorTypes PamAuthenticators::authenticatorTypes() const
{
    return d->computedTypes;
}

void PamAuthenticators::setGraceLocked(bool b)
{
    d->graceLocked = b;
}

bool PamAuthenticators::hadPrompt() const
{
    return d->hadPrompt;
}
