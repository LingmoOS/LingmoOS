/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *   SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "handlebuttonevents.h"
#include "handlebuttoneventsadaptor.h"

#include <PowerDevilProfileSettings.h>
#include <powerdevil_debug.h>
#include <powerdevilcore.h>
#include <powerdevilenums.h>

#include <QAction>

#include <KActionCollection>
#include <KIdleTime>
#include <KLocalizedString>

#include <KScreen/Config>
#include <KScreen/ConfigMonitor>
#include <KScreen/GetConfigOperation>
#include <KScreen/Output>

#include <KGlobalAccel>
#include <Kirigami/Platform/TabletModeWatcher>

using namespace Qt::StringLiterals;

namespace PowerDevil::BundledActions
{
HandleButtonEvents::HandleButtonEvents(QObject *parent)
    : Action(parent)
    , m_screenConfiguration(nullptr)
{
    new HandleButtonEventsAdaptor(this);
    // We enforce no policies here - after all, we just call other actions - which have their policies.
    setRequiredPolicies(PowerDevil::PolicyAgent::None);
    connect(core()->lidController(), &LidController::lidClosedChanged, this, &HandleButtonEvents::onLidClosedChanged);

    KActionCollection *actionCollection = new KActionCollection(this);
    actionCollection->setComponentDisplayName(i18nc("Name for powerdevil shortcuts category", "Power Management"));

    KGlobalAccel *accel = KGlobalAccel::self();

    QAction *globalAction = actionCollection->addAction(u"Sleep"_s);
    globalAction->setText(i18nc("@action:inmenu Global shortcut", "Suspend"));
    accel->setGlobalShortcut(globalAction, Qt::Key_Sleep);
    connect(globalAction, &QAction::triggered, this, &HandleButtonEvents::sleep);

    globalAction = actionCollection->addAction(u"Hibernate"_s);
    globalAction->setText(i18nc("@action:inmenu Global shortcut", "Hibernate"));
    accel->setGlobalShortcut(globalAction, Qt::Key_Hibernate);
    connect(globalAction, &QAction::triggered, this, &HandleButtonEvents::hibernate);

    globalAction = actionCollection->addAction(u"PowerOff"_s);
    globalAction->setText(i18nc("@action:inmenu Global shortcut", "Power Off"));
    auto powerButtonMode = [globalAction](bool isTablet) {
        if (!isTablet) {
            KGlobalAccel::self()->setGlobalShortcut(globalAction, Qt::Key_PowerOff);
        } else {
            KGlobalAccel::self()->setGlobalShortcut(globalAction, QList<QKeySequence>());
        }
    };
    auto interface = Kirigami::Platform::TabletModeWatcher::self();
    connect(interface, &Kirigami::Platform::TabletModeWatcher::tabletModeChanged, globalAction, powerButtonMode);
    powerButtonMode(interface->isTabletMode());
    connect(globalAction, &QAction::triggered, this, &HandleButtonEvents::powerOffButtonTriggered);

    globalAction = actionCollection->addAction(u"PowerDown"_s);
    globalAction->setText(i18nc("@action:inmenu Global shortcut, used for long presses of the power button", "Power Down"));
    accel->setGlobalShortcut(globalAction, Qt::Key_PowerDown);
    connect(globalAction, &QAction::triggered, this, &HandleButtonEvents::powerDownButtonTriggered);

    connect(new KScreen::GetConfigOperation(KScreen::GetConfigOperation::NoEDID),
            &KScreen::ConfigOperation::finished,
            this,
            [this](KScreen::ConfigOperation *op) {
                m_screenConfiguration = qobject_cast<KScreen::GetConfigOperation *>(op)->config();
                checkOutputs();

                KScreen::ConfigMonitor::instance()->addConfig(m_screenConfiguration);
                connect(KScreen::ConfigMonitor::instance(), &KScreen::ConfigMonitor::configurationChanged, this, &HandleButtonEvents::checkOutputs);
            });

    if (!core()->lidController()->isLidClosed()) {
        m_oldKeyboardBrightness = core()->keyboardBrightnessController()->brightness();
    }
    connect(core()->keyboardBrightnessController(),
            &KeyboardBrightnessController::brightnessInfoChanged,
            this,
            [this](const BrightnessLogic::BrightnessInfo &brightnessInfo) {
                // By the time the lid close is processed, the backend brightness will already be updated.
                // That's why we track the brightness here as long as the lid is open.
                if (!core()->lidController()->isLidClosed()) {
                    m_oldKeyboardBrightness = brightnessInfo.value;
                }
            });
}

HandleButtonEvents::~HandleButtonEvents()
{
}

bool HandleButtonEvents::isSupported()
{
    // we handles keyboard shortcuts in our button handling, users always have a keyboard
    return true;
}

void HandleButtonEvents::onProfileUnload()
{
    m_lidAction = PowerDevil::PowerButtonAction::NoAction;
    m_powerButtonAction = PowerDevil::PowerButtonAction::NoAction;
}

void HandleButtonEvents::onIdleTimeout(std::chrono::milliseconds timeout)
{
    Q_UNUSED(timeout)
}

void HandleButtonEvents::onLidClosedChanged(bool closed)
{
    if (closed) {
        if (m_oldKeyboardBrightness.has_value()) {
            core()->keyboardBrightnessController()->setBrightness(0);
        }

        if (!m_screenConfiguration) {
            // triggering the lid closed action now might suspend the system even though it just started up
            // once there's a screen configuration, checkOutputs will call this method again if needed
            return;
        }
        if (!triggersLidAction()) {
            qCWarning(POWERDEVIL) << "Lid action was suppressed because an external monitor is present";
            return;
        }

        processAction(m_lidAction);
    } else {
        // When we restore the keyboard brightness before waking up, we shouldn't conflict
        // with dimdisplay or dpms also messing with the keyboard.
        if (m_oldKeyboardBrightness.has_value() && m_oldKeyboardBrightness > 0) {
            core()->keyboardBrightnessController()->setBrightness(m_oldKeyboardBrightness.value());
        }

        // In this case, let's send a wakeup event
        KIdleTime::instance()->simulateUserActivity();
    }
}

void HandleButtonEvents::processAction(PowerDevil::PowerButtonAction action)
{
    // Basically, we simply trigger other actions :)
    switch (action) {
    case PowerDevil::PowerButtonAction::TurnOffScreen:
        // Turn off screen
        triggerAction(u"DPMSControl"_s, u"TurnOff"_s);
        break;
    case PowerDevil::PowerButtonAction::ToggleScreenOnOff:
        // Toggle screen on/off
        triggerAction(u"DPMSControl"_s, u"ToggleOnOff"_s);
        break;
    default:
        triggerAction(u"SuspendSession"_s, qToUnderlying(action));
        break;
    }
}

void HandleButtonEvents::triggerAction(const QString &action, const QVariant &type)
{
    PowerDevil::Action *helperAction = core()->action(action);
    if (helperAction) {
        helperAction->trigger({
            {QStringLiteral("Type"), type},
            {QStringLiteral("Explicit"), true},
        });
    }
}

void HandleButtonEvents::triggerImpl(const QVariantMap & /*args*/)
{
}

bool HandleButtonEvents::loadAction(const PowerDevil::ProfileSettings &profileSettings)
{
    m_lidAction = static_cast<PowerDevil::PowerButtonAction>(profileSettings.lidAction());
    m_triggerLidActionWhenExternalMonitorPresent = !profileSettings.inhibitLidActionWhenExternalMonitorPresent();
    m_powerButtonAction = static_cast<PowerDevil::PowerButtonAction>(profileSettings.powerButtonAction());
    m_powerDownButtonAction = static_cast<PowerDevil::PowerButtonAction>(profileSettings.powerDownAction());

    constexpr auto NoAction = PowerDevil::PowerButtonAction::NoAction;
    if (m_lidAction == NoAction || m_powerButtonAction == NoAction || m_powerDownButtonAction == NoAction) {
        return false;
    }

    checkOutputs();

    return true;
}

int HandleButtonEvents::lidAction() const
{
    return qToUnderlying(m_lidAction);
}

bool HandleButtonEvents::triggersLidAction() const
{
    return m_triggerLidActionWhenExternalMonitorPresent || !m_externalMonitorPresent.value_or(false);
}

void HandleButtonEvents::powerOffButtonTriggered()
{
    processAction(m_powerButtonAction);
}

void HandleButtonEvents::powerDownButtonTriggered()
{
    processAction(m_powerDownButtonAction);
}

void HandleButtonEvents::hibernate()
{
    processAction(m_hibernateButtonAction);
}

void HandleButtonEvents::sleep()
{
    processAction(m_sleepButtonAction);
}

void HandleButtonEvents::checkOutputs()
{
    if (!m_screenConfiguration) {
        qCWarning(POWERDEVIL) << "Handle button events action could not check for screen configuration";
        return;
    }

    const bool old_triggersLidAction = triggersLidAction();
    const std::optional<bool> oldExternalMonitorPresent = m_externalMonitorPresent;

    const auto outputs = m_screenConfiguration->outputs();
    m_externalMonitorPresent = std::any_of(outputs.begin(), outputs.end(), [](const KScreen::OutputPtr &output) {
        return output->isConnected() && output->isEnabled() && output->type() != KScreen::Output::Panel && output->type() != KScreen::Output::Unknown;
    });

    if (old_triggersLidAction != triggersLidAction() || !oldExternalMonitorPresent.has_value()) {
        Q_EMIT triggersLidActionChanged(triggersLidAction());

        // when the lid is closed but we don't suspend because of an external monitor but we then
        // unplug said monitor, re-trigger the lid action (Bug 379265)
        if (triggersLidAction() && core()->lidController()->isLidClosed()) {
            qCDebug(POWERDEVIL) << "External monitor that kept us from suspending is gone and lid is closed, re-triggering lid action";
            onLidClosedChanged(true);
        }
    }
}

}

#include "moc_handlebuttonevents.cpp"
