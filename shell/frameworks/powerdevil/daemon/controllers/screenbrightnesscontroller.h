/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2010 Alejandro Fiestas <alex@eyeos.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-only

*/

#pragma once

#include <QHash>
#include <QObject>
#include <QStringList>

#include <utility> // std::pair

#include <powerdevilcore_export.h>

#include "displaybrightness.h"
#include "powerdevilscreenbrightnesslogic.h"

class BacklightDetector;
class DDCutilDetector;
class ExternalBrightnessController;

class POWERDEVILCORE_EXPORT ScreenBrightnessController : public QObject
{
    Q_OBJECT

public:
    enum BrightnessStepSize {
        RegularStepSize,
        SmallStepSize,
    };

    /// This hint will be ignored by ScreenBrightnessController itself, but forwarded to any
    /// signals emitted by the corresponding brightness setter call.
    enum IndicatorHint {
        ShowIndicator = 0,
        SuppressIndicator = 1,
    };

    explicit ScreenBrightnessController();
    ~ScreenBrightnessController() override;

    /**
     * Actively look for connected displays.
     *
     * This will emit detectionFinished signal once all candidates were checked, and after initial
     * detection, displayAdded/displayRemoved can be emitted whenever a change in display
     * connections is observed.
     */
    void detectDisplays();
    bool isSupported() const;

    /**
     * A list of string identifiers for displays that support brightness operations.
     *
     * Note that a displayId is not necessarily stable across reboots and hotplugging events.
     * The same display may receive a different displayId when it is removed and later re-added.
     */
    QStringList displayIds() const;

    int knownSafeMinBrightness(const QString &displayId) const;
    int minBrightness(const QString &displayId) const;
    int maxBrightness(const QString &displayId) const;
    int brightness(const QString &displayId) const;

    /**
     * Set display brightness for this @p displayId to the given @p value.
     *
     * The @p value will be clamped to the range within minBrightness(@p displayId) and
     * maxBrightness(@p displayId).
     *
     * @see brightnessChanged
     */
    void setBrightness(const QString &displayId, int value, IndicatorHint hint = SuppressIndicator);

    /**
     * Adjust display brightness for a predetermined set of displays by a step up or down the
     * brightness scale.
     *
     * The direction and size of each step are specified by @p adjustment. The exact behavior
     * of step movement and the set of affected displays are an implementation detail that may
     * change over time.
     *
     * @see brightnessChanged
     */
    void adjustBrightnessStep(PowerDevil::BrightnessLogic::StepAdjustmentAction adjustment, IndicatorHint hint = SuppressIndicator);

    int brightnessSteps(const QString &displayId) const;

    // legacy API without displayId parameter, kept for backward compatibility
    QStringList legacyDisplayIds() const;
    int knownSafeMinBrightness() const;
    int minBrightness() const;
    int maxBrightness() const;
    int brightness() const;
    void setBrightness(int value, IndicatorHint hint = SuppressIndicator);
    int brightnessSteps() const;

Q_SIGNALS:
    void detectionFinished();
    void displayAdded(const QString &displayId);
    void displayRemoved(const QString &displayId);
    void brightnessChanged(const QString &displayId, const PowerDevil::BrightnessLogic::BrightnessInfo &, IndicatorHint);

    // legacy API without displayId parameter, kept for backward compatibility.
    // include legacy prefix to avoid function overload errors when used in connect()
    void legacyDisplayIdsChanged(const QStringList &);
    void legacyBrightnessInfoChanged(const PowerDevil::BrightnessLogic::BrightnessInfo &, IndicatorHint);

private Q_SLOTS:
    void onDisplayDestroyed(QObject *);
    void onDetectorDisplaysChanged();
    void onExternalBrightnessChangeObserved(DisplayBrightness *display, int value);

private:
    struct DisplayInfo {
        DisplayBrightness *display = nullptr;
        DisplayBrightnessDetector *detector = nullptr;
        PowerDevil::ScreenBrightnessLogic brightnessLogic = {};
        bool zombie = false;
    };
    QStringList m_sortedDisplayIds;
    QStringList m_legacyDisplayIds;
    QHash<QString, DisplayInfo> m_displaysById;

    struct DetectorInfo {
        DisplayBrightnessDetector *detector = nullptr;
        const char *debugName = nullptr;
        const char *displayIdPrefix = nullptr;
    };
    QList<DetectorInfo> m_detectors;
    int m_finishedDetectingCount = 0;
    std::unique_ptr<ExternalBrightnessController> m_externalBrightnessController;
};
