/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "brightnesscontrol.h"

#include <PowerDevilProfileSettings.h>
#include <brightnesscontroladaptor.h>
#include <brightnessosdwidget.h>
#include <powerdevil_debug.h>
#include <powerdevilcore.h>

#include <QAction>
#include <QDebug>

#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(PowerDevil::BundledActions::BrightnessControl, "powerdevilbrightnesscontrolaction.json")

namespace PowerDevil::BundledActions
{
BrightnessControl::BrightnessControl(QObject *parent)
    : Action(parent)
{
    // DBus
    new BrightnessControlAdaptor(this);

    connect(core()->screenBrightnessController(),
            &ScreenBrightnessController::legacyBrightnessInfoChanged,
            this,
            &PowerDevil::BundledActions::BrightnessControl::onBrightnessChangedFromController);

    KActionCollection *actionCollection = new KActionCollection(this);
    actionCollection->setComponentDisplayName(i18nc("Name for powerdevil shortcuts category", "Power Management"));

    QAction *globalAction = actionCollection->addAction(QLatin1String("Increase Screen Brightness"));
    globalAction->setText(i18nc("@action:inmenu Global shortcut", "Increase Screen Brightness"));
    KGlobalAccel::setGlobalShortcut(globalAction, Qt::Key_MonBrightnessUp);
    connect(globalAction, &QAction::triggered, this, &BrightnessControl::increaseBrightness);

    globalAction = actionCollection->addAction(QLatin1String("Increase Screen Brightness Small"));
    globalAction->setText(i18nc("@action:inmenu Global shortcut", "Increase Screen Brightness by 1%"));
    KGlobalAccel::setGlobalShortcut(globalAction, Qt::ShiftModifier | Qt::Key_MonBrightnessUp);
    connect(globalAction, &QAction::triggered, this, &BrightnessControl::increaseBrightnessSmall);

    globalAction = actionCollection->addAction(QLatin1String("Decrease Screen Brightness"));
    globalAction->setText(i18nc("@action:inmenu Global shortcut", "Decrease Screen Brightness"));
    KGlobalAccel::setGlobalShortcut(globalAction, Qt::Key_MonBrightnessDown);
    connect(globalAction, &QAction::triggered, this, &BrightnessControl::decreaseBrightness);

    globalAction = actionCollection->addAction(QLatin1String("Decrease Screen Brightness Small"));
    globalAction->setText(i18nc("@action:inmenu Global shortcut", "Decrease Screen Brightness by 1%"));
    KGlobalAccel::setGlobalShortcut(globalAction, Qt::ShiftModifier | Qt::Key_MonBrightnessDown);
    connect(globalAction, &QAction::triggered, this, &BrightnessControl::decreaseBrightnessSmall);
}

void BrightnessControl::onProfileLoad(const QString &previousProfile, const QString &newProfile)
{
    const int absoluteBrightnessValue = qRound(m_defaultValue / 100.0 * brightnessMax());

    // if the current profile is more conservative than the previous one and the
    // current brightness is lower than the new profile
    if (((newProfile == QLatin1String("Battery") && previousProfile == QLatin1String("AC"))
         || (newProfile == QLatin1String("LowBattery") && (previousProfile == QLatin1String("AC") || previousProfile == QLatin1String("Battery"))))
        && absoluteBrightnessValue > brightness()) {
        // We don't want to change anything here
        qCDebug(POWERDEVIL) << "Not changing brightness, the current one is lower and the profile is more conservative";
    } else if (absoluteBrightnessValue >= 0) {
        core()->screenBrightnessController()->setBrightness(absoluteBrightnessValue);
    }
}

void BrightnessControl::triggerImpl(const QVariantMap & /*args*/)
{
}

bool BrightnessControl::isSupported()
{
    return core()->screenBrightnessController()->isSupported();
}

bool BrightnessControl::loadAction(const PowerDevil::ProfileSettings &profileSettings)
{
    if (!profileSettings.useProfileSpecificDisplayBrightness()) {
        return false;
    }

    m_defaultValue = profileSettings.displayBrightness();
    return true;
}

void BrightnessControl::onBrightnessChangedFromController(const BrightnessLogic::BrightnessInfo &info, ScreenBrightnessController::IndicatorHint hint)
{
    if (info.value != m_lastBrightnessInfo.value) {
        Q_EMIT brightnessChanged(info.value);
    }
    if (info.valueMax != m_lastBrightnessInfo.valueMax) {
        Q_EMIT brightnessMaxChanged(info.valueMax);
    }
    if (info.valueMin != m_lastBrightnessInfo.valueMin) {
        Q_EMIT brightnessMinChanged(info.valueMin);
    }

    m_lastBrightnessInfo = info;

    if (hint == ScreenBrightnessController::ShowIndicator) {
        BrightnessOSDWidget::show(brightnessPercent(m_lastBrightnessInfo.value));
    }
}

int BrightnessControl::brightness() const
{
    return core()->screenBrightnessController()->brightness();
}

int BrightnessControl::brightnessMax() const
{
    return core()->screenBrightnessController()->maxBrightness();
}

int BrightnessControl::brightnessMin() const
{
    return core()->screenBrightnessController()->minBrightness();
}

int BrightnessControl::knownSafeBrightnessMin() const
{
    // Theoretically we should provide a change signal for this property, because it can change as
    // display connections change, even if it's constant per individual display. In practice,
    // this won't change as long as laptop screens don't get added or removed at runtime.
    // Which shouldn't happen.
    return core()->screenBrightnessController()->knownSafeMinBrightness();
}

void BrightnessControl::setBrightness(int value)
{
    core()->screenBrightnessController()->setBrightness(value, ScreenBrightnessController::ShowIndicator);
}

void BrightnessControl::setBrightnessSilent(int value)
{
    core()->screenBrightnessController()->setBrightness(value, ScreenBrightnessController::SuppressIndicator);
}

void BrightnessControl::increaseBrightness()
{
    core()->screenBrightnessController()->adjustBrightnessStep(BrightnessLogic::Increase, ScreenBrightnessController::ShowIndicator);
}

void BrightnessControl::increaseBrightnessSmall()
{
    core()->screenBrightnessController()->adjustBrightnessStep(BrightnessLogic::IncreaseSmall, ScreenBrightnessController::ShowIndicator);
}

void BrightnessControl::decreaseBrightness()
{
    core()->screenBrightnessController()->adjustBrightnessStep(BrightnessLogic::Decrease, ScreenBrightnessController::ShowIndicator);
}

void BrightnessControl::decreaseBrightnessSmall()
{
    core()->screenBrightnessController()->adjustBrightnessStep(BrightnessLogic::DecreaseSmall, ScreenBrightnessController::ShowIndicator);
}

int BrightnessControl::brightnessSteps() const
{
    return core()->screenBrightnessController()->brightnessSteps();
}

int BrightnessControl::brightnessPercent(float value) const
{
    const float maxBrightness = brightnessMax();
    if (maxBrightness <= 0) {
        return 0;
    }

    return qRound(value / maxBrightness * 100);
}

}

#include "brightnesscontrol.moc"

#include "moc_brightnesscontrol.cpp"
