/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "suspendsession.h"

#include <PowerDevilProfileSettings.h>
#include <powerdevil_debug.h>
#include <powerdevilcore.h>

#include "suspendsessionadaptor.h"

#include <kwinkscreenhelpereffect.h>

#include <KIdleTime>
#include <KJob>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(PowerDevil::BundledActions::SuspendSession, "powerdevilsuspendsessionaction.json")

using namespace std::chrono_literals;
using namespace Qt::StringLiterals;

namespace PowerDevil::BundledActions
{
SuspendSession::SuspendSession(QObject *parent)
    : Action(parent)
    , m_fadeEffect(new PowerDevil::KWinKScreenHelperEffect())
{
    // DBus
    new SuspendSessionAdaptor(this);

    setRequiredPolicies(PowerDevil::PolicyAgent::InterruptSession);

    connect(core()->suspendController(), &SuspendController::resumeFromSuspend, this, [this]() {
        KIdleTime::instance()->simulateUserActivity();

        PowerDevil::PolicyAgent::instance()->setupSystemdInhibition();

        m_fadeEffect->stop();

        Q_EMIT resumingFromSuspend();
    });
}

SuspendSession::~SuspendSession() = default;

void SuspendSession::onWakeupFromIdle()
{
    m_fadeEffect->stop();
}

void SuspendSession::onIdleTimeout(std::chrono::milliseconds timeout)
{
    PolicyAgent::RequiredPolicies unsatisfiablePolicies = PolicyAgent::instance()->requirePolicyCheck(PowerDevil::PolicyAgent::InterruptSession);
    if (unsatisfiablePolicies != PolicyAgent::None) {
        return;
    }

    // we fade the screen to black 5 seconds prior to suspending to alert the user
    if (timeout == m_idleTime - 5s) {
        m_fadeEffect->start();
    } else {
        QVariantMap args{{QStringLiteral("Type"), qToUnderlying(m_autoSuspendAction)}};
        triggerImpl(args);
    }
}

void SuspendSession::triggerImpl(const QVariantMap &args)
{
    qCDebug(POWERDEVIL) << "Suspend session triggered with" << args;

    const auto mode = static_cast<PowerDevil::PowerButtonAction>(args[u"Type"_s].toUInt());

    if (mode == PowerDevil::PowerButtonAction::Sleep || mode == PowerDevil::PowerButtonAction::Hibernate) {
        // don't suspend if shutting down
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.Shutdown"))) {
            qCDebug(POWERDEVIL) << "Not suspending because a shutdown is in progress";
            return;
        }
    }

    // Switch for real action
    switch (static_cast<PowerDevil::PowerButtonAction>(args[u"Type"_s].toUInt())) {
    case PowerDevil::PowerButtonAction::Sleep: {
        Q_EMIT aboutToSuspend();
        auto sleepMode = args.contains(u"SleepMode"_s) ? static_cast<PowerDevil::SleepMode>(args[u"SleepMode"_s].toUInt()) : m_sleepMode;

        if (sleepMode == PowerDevil::SleepMode::SuspendThenHibernate) {
            core()->suspendController()->suspendThenHibernate();
        } else if (sleepMode == PowerDevil::SleepMode::HybridSuspend) {
            core()->suspendController()->hybridSuspend();
        } else {
            core()->suspendController()->suspend();
        }
        break;
    }
    case PowerDevil::PowerButtonAction::Hibernate: {
        Q_EMIT aboutToSuspend();
        core()->suspendController()->hibernate();
        break;
    }
    case PowerDevil::PowerButtonAction::Shutdown: {
        SessionManagement sessionManager;
        sessionManager.requestShutdown(SessionManagement::ConfirmationMode::Skip);
        break;
    }
    case PowerDevil::PowerButtonAction::PromptLogoutDialog: {
        SessionManagement sessionManager;
        sessionManager.requestLogoutPrompt();
        break;
    }
    case PowerDevil::PowerButtonAction::LockScreen: {
        SessionManagement sessionManager;
        sessionManager.lock();
        break;
    }
    default:
        break;
    }
}

bool SuspendSession::loadAction(const PowerDevil::ProfileSettings &profileSettings)
{
    m_sleepMode = static_cast<PowerDevil::SleepMode>(profileSettings.sleepMode());

    if (profileSettings.autoSuspendAction() == qToUnderlying(PowerDevil::PowerButtonAction::NoAction)) {
        return false;
    }

    // Add the idle timeout
    m_idleTime = std::chrono::seconds(profileSettings.autoSuspendIdleTimeoutSec());
    if (m_idleTime != 0s) {
        registerIdleTimeout(m_idleTime - 5s);
        registerIdleTimeout(m_idleTime);
    }
    m_autoSuspendAction = static_cast<PowerDevil::PowerButtonAction>(profileSettings.autoSuspendAction());

    return true;
}

void SuspendSession::suspendToDisk()
{
    triggerSuspendSession(PowerDevil::PowerButtonAction::Hibernate);
}

void SuspendSession::suspendToRam()
{
    triggerImpl({
        {QStringLiteral("Type"), qToUnderlying(PowerDevil::PowerButtonAction::Sleep)},
        {QStringLiteral("SleepMode"), qToUnderlying(PowerDevil::SleepMode::SuspendToRam)},
    });
}

void SuspendSession::suspendHybrid()
{
    triggerImpl({
        {QStringLiteral("Type"), qToUnderlying(PowerDevil::PowerButtonAction::Sleep)},
        {QStringLiteral("SleepMode"), qToUnderlying(PowerDevil::SleepMode::HybridSuspend)},
    });
}

void SuspendSession::triggerSuspendSession(PowerDevil::PowerButtonAction action)
{
    triggerImpl({
        {QStringLiteral("Type"), qToUnderlying(action)},
    });
}
}
#include "suspendsession.moc"

#include "moc_suspendsession.cpp"
