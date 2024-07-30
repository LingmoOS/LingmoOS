/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *   SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dpms.h"
#include <KScreenDpms/Dpms>

#include <PowerDevilProfileSettings.h>
#include <kwinkscreenhelpereffect.h>
#include <powerdevil_debug.h>
#include <powerdevilcore.h>

#include <QAction>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QGuiApplication>
#include <QTimer>
#include <private/qtx11extras_p.h>

#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>
#include <Kirigami/Platform/TabletModeWatcher>

K_PLUGIN_CLASS_WITH_JSON(PowerDevil::BundledActions::DPMS, "powerdevildpmsaction.json")

using namespace std::chrono_literals;
using namespace Qt::StringLiterals;

// always make sure to allow meaningful interaction and for lock screens, time for authentication methods
static constexpr std::chrono::milliseconds s_minIdleTimeoutWhenUnlocked = 30s;
static constexpr std::chrono::milliseconds s_minIdleTimeoutWhenLocked = 10s;
static constexpr std::chrono::milliseconds s_minIdleTimeoutWhenActivatingLock = 100ms;

namespace PowerDevil
{
namespace BundledActions
{
DPMS::DPMS(QObject *parent)
    : Action(parent)
    , m_dpms(new KScreen::Dpms)
{
    setRequiredPolicies(PowerDevil::PolicyAgent::ChangeScreenSettings);

    // On Wayland, KWin takes care of performing the effect properly
    if (QX11Info::isPlatformX11()) {
        auto fadeEffect = new PowerDevil::KWinKScreenHelperEffect(this);
        connect(this, &DPMS::startFade, fadeEffect, &PowerDevil::KWinKScreenHelperEffect::start);
        connect(this, &DPMS::stopFade, fadeEffect, &PowerDevil::KWinKScreenHelperEffect::stop);
        connect(fadeEffect, &PowerDevil::KWinKScreenHelperEffect::fadedOut, this, &DPMS::turnOffOnIdleTimeout);
    } else {
        // short-cut without fadeEffect in between timeout and actual action (compositor handles fading)
        connect(this, &DPMS::startFade, this, &DPMS::turnOffOnIdleTimeout);
    }

    // Listen to the policy agent
    auto policyAgent = PowerDevil::PolicyAgent::instance();
    connect(policyAgent, &PowerDevil::PolicyAgent::unavailablePoliciesChanged, this, &DPMS::onUnavailablePoliciesChanged);
    connect(policyAgent, &PowerDevil::PolicyAgent::screenLockerActiveChanged, this, &DPMS::onScreenLockerActiveChanged);

    m_inhibitScreen = policyAgent->unavailablePolicies() & PowerDevil::PolicyAgent::ChangeScreenSettings;

    connect(core()->suspendController(), &SuspendController::aboutToSuspend, this, &DPMS::onAboutToSuspend);
    connect(core()->suspendController(), &SuspendController::resumeFromSuspend, this, &DPMS::onResumeFromSuspend);

    KActionCollection *actionCollection = new KActionCollection(this);
    actionCollection->setComponentDisplayName(i18nc("Name for powerdevil shortcuts category", "Power Management"));

    QAction *globalAction = actionCollection->addAction(QLatin1String("Turn Off Screen"));
    globalAction->setText(i18nc("@action:inmenu Global shortcut", "Turn Off Screen"));
    connect(globalAction, &QAction::triggered, this, [this] {
        if (m_lockBeforeTurnOff) {
            lockScreen();
        }
        m_dpms->switchMode(KScreen::Dpms::Off);
    });

    auto powerButtonMode = [globalAction](bool isTablet) {
        if (!isTablet) {
            KGlobalAccel::self()->setGlobalShortcut(globalAction, QList<QKeySequence>());
        } else {
            KGlobalAccel::self()->setGlobalShortcut(globalAction, Qt::Key_PowerOff);
        }
    };
    auto interface = Kirigami::Platform::TabletModeWatcher::self();
    connect(interface, &Kirigami::Platform::TabletModeWatcher::tabletModeChanged, globalAction, powerButtonMode);
    powerButtonMode(interface->isTabletMode());
}

DPMS::~DPMS() = default;

bool DPMS::isSupported()
{
    return m_dpms->isSupported();
}

void DPMS::onWakeupFromIdle()
{
    if (m_isActivatingLock) {
        // Use the longer minimum timeout whenever we see user interaction, e.g. for allowing the user to type
        registerStandardIdleTimeout();
    }
    Q_EMIT stopFade(); // only actively used in X11

    if (m_oldKeyboardBrightness > 0) {
        setKeyboardBrightnessHelper(m_oldKeyboardBrightness);
        m_oldKeyboardBrightness = 0;
    }
}

void DPMS::onIdleTimeout(std::chrono::milliseconds /*timeout*/)
{
    // Do not inhibit anything even if idleTimeout reaches because we are inhibit
    if (!m_inhibitScreen && isSupported()) {
        qCDebug(POWERDEVIL) << "DPMS: starting to fade out";
        Q_EMIT startFade();
    } else {
        qCDebug(POWERDEVIL) << "DPMS: inhibited (or unsupported), not turning off display";
    }
}

void DPMS::turnOffOnIdleTimeout()
{
    qCDebug(POWERDEVIL) << "DPMS: triggered on idle timeout, turning off display and keyboard backlight";

    const int keyboardBrightness = core()->keyboardBrightnessController()->brightness();
    if (keyboardBrightness > 0) {
        m_oldKeyboardBrightness = keyboardBrightness;
        setKeyboardBrightnessHelper(0);
    }
    if (isSupported()) {
        m_dpms->switchMode(KScreen::Dpms::Off);
    }
}

void DPMS::setKeyboardBrightnessHelper(int brightness)
{
    trigger({{u"KeyboardBrightness"_s, QVariant::fromValue(brightness)}});
}

void DPMS::triggerImpl(const QVariantMap &args)
{
    QString type = args.value(QStringLiteral("Type")).toString();
    qCDebug(POWERDEVIL) << "DPMS: triggered from externally, type:" << (type.isEmpty() ? u"TurnOn"_s : type);

    QString KEYBOARD_BRIGHTNESS = QStringLiteral("KeyboardBrightness");
    if (args.contains(KEYBOARD_BRIGHTNESS)) {
        core()->keyboardBrightnessController()->setBrightness(args.value(KEYBOARD_BRIGHTNESS).toInt());
        return;
    }

    if (!isSupported()) {
        return;
    }
    if (m_lockBeforeTurnOff && (type == u"TurnOff" || type == u"ToggleOnOff")) {
        lockScreen();
    }
    KScreen::Dpms::Mode level = KScreen::Dpms::Mode::On;
    if (type == QLatin1String("ToggleOnOff")) {
        level = KScreen::Dpms::Toggle;
    } else if (type == QLatin1String("TurnOff")) {
        level = KScreen::Dpms::Off;
    } else if (type == QLatin1String("Standby")) {
        level = KScreen::Dpms::Standby;
    } else if (type == QLatin1String("Suspend")) {
        level = KScreen::Dpms::Suspend;
    } else {
        level = KScreen::Dpms::On;
    }
    m_dpms->switchMode(level);
}

bool DPMS::loadAction(const PowerDevil::ProfileSettings &profileSettings)
{
    m_idleTimeoutWhenUnlocked = std::chrono::seconds(profileSettings.turnOffDisplayIdleTimeoutSec());
    m_idleTimeoutWhenLocked = std::chrono::seconds(profileSettings.turnOffDisplayIdleTimeoutWhenLockedSec());
    m_lockBeforeTurnOff = profileSettings.lockBeforeTurnOffDisplay();

    if (!profileSettings.turnOffDisplayWhenIdle() || m_idleTimeoutWhenUnlocked < 0ms) {
        onProfileUnload();
        return false;
    }

    m_idleTimeoutWhenUnlocked = std::max(m_idleTimeoutWhenUnlocked, s_minIdleTimeoutWhenUnlocked);

    if (m_idleTimeoutWhenLocked < 0ms) {
        m_idleTimeoutWhenLocked = m_idleTimeoutWhenUnlocked;
    }
    // Allow (almost) immediate turn-off upon entering the lockscreen, but not when already locked
    m_idleTimeoutWhenActivatingLock = std::max(m_idleTimeoutWhenLocked, s_minIdleTimeoutWhenActivatingLock);
    m_idleTimeoutWhenLocked = std::max(m_idleTimeoutWhenLocked, s_minIdleTimeoutWhenLocked);

    registerStandardIdleTimeout();
    return true;
}

void DPMS::onProfileUnload()
{
    m_idleTimeoutWhenUnlocked = std::chrono::milliseconds(-1);
}

void DPMS::onUnavailablePoliciesChanged(PowerDevil::PolicyAgent::RequiredPolicies policies)
{
    m_inhibitScreen = policies & PowerDevil::PolicyAgent::ChangeScreenSettings;
}

void DPMS::registerStandardIdleTimeout()
{
    unregisterIdleTimeouts();
    m_isActivatingLock = false;

    if (m_idleTimeoutWhenUnlocked < 0ms) { // action created, but not loaded
        return;
    }

    if (PowerDevil::PolicyAgent::instance()->screenLockerActive()) {
        qCDebug(POWERDEVIL) << "DPMS: registering idle timeout (screen already locked) after" << m_idleTimeoutWhenLocked;
        registerIdleTimeout(m_idleTimeoutWhenLocked);
    } else {
        qCDebug(POWERDEVIL) << "DPMS: registering idle timeout after" << m_idleTimeoutWhenUnlocked;
        registerIdleTimeout(m_idleTimeoutWhenUnlocked);
    }
}

void DPMS::onScreenLockerActiveChanged(bool active)
{
    unregisterIdleTimeouts();
    m_isActivatingLock = active && !m_isAboutToSuspend;

    if (m_idleTimeoutWhenUnlocked < 0ms) { // action created, but not loaded
        return;
    }

    if (m_isActivatingLock) {
        // entering lockscreen - fast display turn-off if the config calls for it
        qCDebug(POWERDEVIL) << "DPMS: registering idle timeout (screen lock activating) after" << m_idleTimeoutWhenActivatingLock;
        registerIdleTimeout(m_idleTimeoutWhenActivatingLock);
        // if we're locking but about to suspend, wait until onResumeFromSuspend() to register the timeout
    } else if (!active) {
        // restoring normal idleTimeout
        qCDebug(POWERDEVIL) << "DPMS: registering idle timeout (screen unlocked) after" << m_idleTimeoutWhenUnlocked;
        registerIdleTimeout(m_idleTimeoutWhenUnlocked);
    }
}

void DPMS::onAboutToSuspend()
{
    m_isAboutToSuspend = true;
    unregisterIdleTimeouts();
}

void DPMS::onResumeFromSuspend()
{
    m_isAboutToSuspend = false;
    registerStandardIdleTimeout();
}

static std::chrono::milliseconds dimAnimationTime()
{
    // See kscreen.kcfg from kwin
    return std::chrono::milliseconds(KSharedConfig::openConfig(u"kwinrc"_s)->group(u"Effect-Kscreen"_s).readEntry("Duration", 250));
}

void DPMS::lockScreen()
{
    // We need to delay locking until the screen has dimmed, otherwise it looks all clunky
    QTimer::singleShot(dimAnimationTime(), this, [] {
        const QDBusMessage msg =
            QDBusMessage::createMethodCall(u"org.freedesktop.ScreenSaver"_s, u"/ScreenSaver"_s, u"org.freedesktop.ScreenSaver"_s, u"Lock"_s);
        QDBusConnection::sessionBus().asyncCall(msg);
    });
}

}
}
#include "dpms.moc"

#include "moc_dpms.cpp"
