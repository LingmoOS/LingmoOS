/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2010 Alejandro Fiestas <alex@eyeos.org>
    SPDX-FileCopyrightText: 2010-2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-only

*/

#include "screenbrightnesscontroller.h"

#include "backlightbrightness.h"
#include "ddcutildetector.h"
#include "displaybrightness.h"
#include "kwinbrightness.h"
#include "externalbrightnesscontrol.h"

#include <brightnessosdwidget.h>
#include <powerdevil_debug.h>

#include <QDebug>
#include <QPropertyAnimation>
#include <ranges>

#include <algorithm> // std::ranges::find_if

ScreenBrightnessController::ScreenBrightnessController()
    : QObject()
    , m_detectors({
          {
              .detector = new KWinDisplayDetector(this),
              .debugName = "kwin brightness control",
              .displayIdPrefix = "kwin:",
          },
          {
              .detector = new BacklightDetector(this),
              .debugName = "internal display backlight",
              .displayIdPrefix = "backlight:",
          },
          {
              .detector = new DDCutilDetector(this),
              .debugName = "libddcutil",
              .displayIdPrefix = "ddc:",
          },
      })
    , m_externalBrightnessController(std::make_unique<ExternalBrightnessController>())
{
}

ScreenBrightnessController::~ScreenBrightnessController()
{
}

void ScreenBrightnessController::detectDisplays()
{
    qCDebug(POWERDEVIL) << "Trying to detect displays for brightness control...";
    m_finishedDetectingCount = 0;

    for (const DetectorInfo &detectorInfo : m_detectors) {
        DisplayBrightnessDetector *detector = detectorInfo.detector;
        disconnect(detector, nullptr, this, nullptr);

        connect(detector, &DisplayBrightnessDetector::detectionFinished, this, [this, detector]() {
            disconnect(detector, &DisplayBrightnessDetector::detectionFinished, this, nullptr);

            if (++m_finishedDetectingCount; m_finishedDetectingCount == m_detectors.size()) {
                onDetectorDisplaysChanged();
                Q_EMIT detectionFinished();
            }
            connect(detector, &DisplayBrightnessDetector::displaysChanged, this, &ScreenBrightnessController::onDetectorDisplaysChanged);
        });
        detector->detect();
    }
}

bool ScreenBrightnessController::isSupported() const
{
    return !m_sortedDisplayIds.isEmpty();
}

QStringList ScreenBrightnessController::displayIds() const
{
    return m_sortedDisplayIds;
}

void ScreenBrightnessController::onDisplayDestroyed(QObject *obj)
{
    for (auto it = m_displaysById.begin(); it != m_displaysById.end(); ++it) {
        if (it->display == obj) {
            // we'll do the proper removal in onDetectorDisplaysChanged() which should come
            // right afterwards, just don't call it anymore including through QObject::disconnect()
            it->zombie = true;
        }
    }
}

void ScreenBrightnessController::onDetectorDisplaysChanged()
{
    // don't clear m_displaysById, we'll diff its items for proper signal invocation
    m_sortedDisplayIds.clear();
    QStringList legacyDisplayIds;
    QStringList addedDisplayIds;
    QStringList brightnessChangedDisplayIds;

    QList<DisplayBrightness *> forExternalControl;

    // for backwards compatibility with legacy API clients, set the same brightness to all displays
    // of the first detector - e.g. to only the backlight display, or to all external DDC monitors
    DisplayBrightnessDetector *firstSupportedDetector = nullptr;

    // add new displays
    for (const DetectorInfo &detectorInfo : m_detectors) {
        QList<DisplayBrightness *> detectorDisplays = detectorInfo.detector->displays();

        if (!detectorDisplays.isEmpty()) {
            if (firstSupportedDetector == nullptr) {
                firstSupportedDetector = detectorInfo.detector;
            }
            qCDebug(POWERDEVIL) << "Using" << detectorInfo.debugName << "for brightness controls.";
        }
        for (DisplayBrightness *display : std::as_const(detectorDisplays)) {
            const QString displayId = QString::fromLocal8Bit(detectorInfo.displayIdPrefix) + display->id();
            bool replacing = false;

            if (const auto it = m_displaysById.constFind(displayId); it != m_displaysById.constEnd()) {
                if (it->display != display) { // same displayId, different object: start from scratch
                    if (!it->zombie) {
                        disconnect(it->display, nullptr, this, nullptr);
                    }
                    const PowerDevil::BrightnessLogic::BrightnessInfo previous = it->brightnessLogic.info();

                    if (previous.value != display->brightness() || previous.valueMax != display->maxBrightness()) {
                        brightnessChangedDisplayIds.append(displayId); // in case the new object has different values
                    }
                    replacing = true;
                }
            }
            if (replacing || !m_displaysById.contains(displayId)) {
                if (m_externalBrightnessController->isActive() && !dynamic_cast<KWinDisplayDetector *>(detectorInfo.detector)) {
                    // control over this display should be given to KWin
                    forExternalControl.push_back(display);
                    continue;
                }
                connect(display, &QObject::destroyed, this, &ScreenBrightnessController::onDisplayDestroyed);
                connect(display, &DisplayBrightness::externalBrightnessChangeObserved, this, &ScreenBrightnessController::onExternalBrightnessChangeObserved);

                auto &info = m_displaysById[displayId] = DisplayInfo{
                    .display = display,
                    .detector = detectorInfo.detector,
                };
                info.brightnessLogic.setValueRange(display->knownSafeMinBrightness(), display->maxBrightness());
                info.brightnessLogic.setValue(display->brightness());

                if (!replacing) {
                    addedDisplayIds.append(displayId);
                }
            }
            m_sortedDisplayIds.append(displayId);

            if (detectorInfo.detector == firstSupportedDetector) {
                legacyDisplayIds.append(displayId);
            }
        }
    }

    m_externalBrightnessController->setDisplays(forExternalControl);

    // remove displays that were in the list before, but disappeared after the update
    for (const QString &displayId : m_displaysById.keys()) {
        if (!m_sortedDisplayIds.contains(displayId)) {
            const auto it = m_displaysById.constFind(displayId);
            if (!it->zombie) {
                disconnect(it->display, nullptr, this, nullptr);
            }
            m_displaysById.erase(it);
            Q_EMIT displayRemoved(displayId);
        }
    }

    // now that display structures are all up to date, emit signals about new and changed displays
    for (const QString &displayId : std::as_const(addedDisplayIds)) {
        Q_EMIT displayAdded(displayId);
    }
    for (const QString &displayId : std::as_const(brightnessChangedDisplayIds)) {
        const auto it = m_displaysById.constFind(displayId);
        qCDebug(POWERDEVIL) << "Screen brightness of display" << displayId << "after detection/reconfiguration:" << it->brightnessLogic.info().value;
        Q_EMIT brightnessChanged(displayId, it->brightnessLogic.info(), SuppressIndicator);
    }

    const QString previousFirstDisplayId = m_legacyDisplayIds.value(0, QString());

    if (m_legacyDisplayIds != legacyDisplayIds) {
        m_legacyDisplayIds = legacyDisplayIds;
        Q_EMIT legacyDisplayIdsChanged(m_legacyDisplayIds);
    }

    if (!isSupported()) {
        qCDebug(POWERDEVIL) << "No suitable displays detected. Brightness controls are unsupported in this configuration.";
        return;
    }

    // legacy API needs to emit a brightness change signal for the first display, regardless of
    // whether the new first display was newly added or an existing display moved up to index 0
    const QString &newFirstDisplayId = m_legacyDisplayIds.first();
    if (newFirstDisplayId != previousFirstDisplayId || brightnessChangedDisplayIds.contains(newFirstDisplayId)) {
        const auto it = m_displaysById.constFind(newFirstDisplayId);
        qCDebug(POWERDEVIL) << "Screen brightness of first display after detection/reconfiguration:" << it->brightnessLogic.info().value;
        Q_EMIT legacyBrightnessInfoChanged(it->brightnessLogic.info(), SuppressIndicator);
    }
}

int ScreenBrightnessController::knownSafeMinBrightness(const QString &displayId) const
{
    if (const auto it = m_displaysById.constFind(displayId); it != m_displaysById.constEnd() && !it->zombie) {
        int result = it->display->knownSafeMinBrightness();
        qCDebug(POWERDEVIL) << "Screen knownSafeMinBrightness of" << displayId << "is" << result;
        return result;
    }
    qCWarning(POWERDEVIL) << "Query screen knownSafeMinBrightness failed: no display with id" << displayId;
    return 0;
}

int ScreenBrightnessController::minBrightness(const QString &displayId) const
{
    if (const auto it = m_displaysById.constFind(displayId); it != m_displaysById.constEnd()) {
        int result = it->brightnessLogic.info().valueMin;
        qCDebug(POWERDEVIL) << "Screen minBrightness of" << displayId << "is" << result;
        return result;
    }
    qCWarning(POWERDEVIL) << "Query screen minBrightness failed: no display with id" << displayId;
    return 0;
}

int ScreenBrightnessController::maxBrightness(const QString &displayId) const
{
    if (const auto it = m_displaysById.constFind(displayId); it != m_displaysById.constEnd()) {
        int result = it->brightnessLogic.info().valueMax;
        qCDebug(POWERDEVIL) << "Screen maxBrightness of" << displayId << "is" << result;
        return result;
    }
    qCWarning(POWERDEVIL) << "Query screen maxBrightness failed: no display with id" << displayId;
    return 0;
}

int ScreenBrightnessController::brightness(const QString &displayId) const
{
    if (const auto it = m_displaysById.constFind(displayId); it != m_displaysById.constEnd()) {
        int result = it->brightnessLogic.info().value;
        qCDebug(POWERDEVIL) << "Screen brightness of" << displayId << "is" << result;
        return result;
    }
    qCWarning(POWERDEVIL) << "Query screen brightness failed: no display with id" << displayId;
    return 0;
}

void ScreenBrightnessController::setBrightness(const QString &displayId, int value, IndicatorHint hint)
{
    if (auto it = m_displaysById.find(displayId); it != m_displaysById.end() && !it->zombie) {
        const PowerDevil::BrightnessLogic::BrightnessInfo bi = it->brightnessLogic.info();
        const int boundedValue = qBound(bi.valueMin, value, bi.valueMax);

        qCDebug(POWERDEVIL) << "Set screen brightness of" << displayId << "to" << boundedValue << "/" << bi.valueMax;
        if (value != boundedValue) {
            qCDebug(POWERDEVIL) << "- clamped from" << value;
        }

        // notify only when the internally tracked brightness value is actually different
        if (bi.value != boundedValue) {
            it->brightnessLogic.setValue(boundedValue);
            Q_EMIT brightnessChanged(displayId, it->brightnessLogic.info(), hint);

            // legacy API without displayId parameter: notify only if the first supported display changed
            if (displayId == m_legacyDisplayIds.first()) {
                Q_EMIT legacyBrightnessInfoChanged(it->brightnessLogic.info(), hint);
            }
        }

        // but always call setBrightness() on the display, in case we're unaware of an external change
        it->display->setBrightness(boundedValue);
    } else {
        qCWarning(POWERDEVIL) << "Set screen brightness failed: no display with id" << displayId;
    }
}

void ScreenBrightnessController::adjustBrightnessStep(PowerDevil::BrightnessLogic::StepAdjustmentAction adjustment, IndicatorHint hint)
{
    if (!isSupported()) {
        qCWarning(POWERDEVIL) << "Adjust brightness step failed: no displays available to adjust";
        return;
    }

    if (const auto it = m_displaysById.constFind(m_sortedDisplayIds.first()); it != m_displaysById.constEnd()) {
        int firstDisplayBrightness = it->brightnessLogic.adjusted(adjustment);
        if (firstDisplayBrightness < 0) {
            return;
        }

        setBrightness(m_sortedDisplayIds.first(), firstDisplayBrightness, hint);
    } else {
        qCWarning(POWERDEVIL) << "Adjust brightness step failed: no display with id" << m_sortedDisplayIds.first();
    }
}

int ScreenBrightnessController::brightnessSteps(const QString &displayId) const
{
    if (const auto it = m_displaysById.constFind(displayId); it != m_displaysById.constEnd()) {
        return it->brightnessLogic.steps();
    }
    qCWarning(POWERDEVIL) << "Query screen brightnessSteps failed: no display with id" << displayId;
    return 1;
}

void ScreenBrightnessController::onExternalBrightnessChangeObserved(DisplayBrightness *display, int value)
{
    const auto it = std::ranges::find_if(m_displaysById, [display](const DisplayInfo &info) {
        return info.display == display;
    });
    if (it == m_displaysById.end()) {
        qCWarning(POWERDEVIL) << "External brightness change of untracked display" << display->id() << "to" << value << "/" << display->maxBrightness();
        return;
    }
    if (value == it->brightnessLogic.info().value) {
        qCDebug(POWERDEVIL) << "External brightness change of display" << it.key() << "ignored - same as previous value";
        return;
    }
    qCDebug(POWERDEVIL) << "External brightness change of display" << it.key() << "to" << value << "/" << it->brightnessLogic.info().valueMax;

    it->brightnessLogic.setValue(value);

    Q_EMIT brightnessChanged(it.key(), it->brightnessLogic.info(), SuppressIndicator);

    // legacy API without displayId parameter: notify only if the first supported display changed
    if (it.key() == m_legacyDisplayIds.first()) {
        Q_EMIT legacyBrightnessInfoChanged(it->brightnessLogic.info(), SuppressIndicator);
    }
}

//
// legacy API without displayId parameter

QStringList ScreenBrightnessController::legacyDisplayIds() const
{
    return m_legacyDisplayIds;
}

int ScreenBrightnessController::knownSafeMinBrightness() const
{
    return knownSafeMinBrightness(m_legacyDisplayIds.value(0, QString()));
}

int ScreenBrightnessController::minBrightness() const
{
    return minBrightness(m_legacyDisplayIds.value(0, QString()));
}

int ScreenBrightnessController::maxBrightness() const
{
    return maxBrightness(m_legacyDisplayIds.value(0, QString()));
}

int ScreenBrightnessController::brightness() const
{
    return brightness(m_legacyDisplayIds.value(0, QString()));
}

void ScreenBrightnessController::setBrightness(int value, IndicatorHint hint)
{
    if (m_legacyDisplayIds.isEmpty()) {
        qCWarning(POWERDEVIL) << "Set screen brightness failed: no supported display available";
    }
    for (const QString &displayId : std::as_const(m_legacyDisplayIds)) {
        setBrightness(displayId, value, hint);
    }
}

int ScreenBrightnessController::brightnessSteps() const
{
    return brightnessSteps(m_legacyDisplayIds.value(0, QString()));
}

#include "moc_screenbrightnesscontroller.cpp"
