/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

/**
 * Interface for accessing & manipulating brightness of a single display.
 */
class DisplayBrightness : public QObject
{
    Q_OBJECT

public:
    explicit DisplayBrightness(QObject *parent = nullptr);

    /**
     * A string that uniquely identifies this display within the scope of its associated detector.
     */
    virtual QString id() const = 0;

    /**
     * A human-readable name for this display.
     */
    virtual QString label() const = 0;

    /**
     * The minimum practical brightness value that still results in contents being visible.
     *
     * Some screens fully turn off when set to 0. Brightness adjustments should not be used to
     * turn screens off, use DPMS interfaces for this instead. Setting brightness to values in
     * the range [0, knownSafeMinBrightness()) might still keep contents visible, but should
     * only be done with informed user consent.
     */
    virtual int knownSafeMinBrightness() const = 0;

    /**
     * The maximum brightness value that the user is allowed to set.
     */
    virtual int maxBrightness() const = 0;

    /**
     * The latest brightness value that was set or observed for this display.
     *
     * It's acceptable if this class does not pick up on brightness changes by external actors.
     */
    virtual int brightness() const = 0;
    virtual void setBrightness(int brightness) = 0;

    /**
     * @returns if this object controls the brightness of an internal panel
     */
    virtual bool isInternal() const;

    /**
     * @returns the EDID data associated with the display this object controls the brightness of
     *          or std::nullopt, if that's unknown
     */
    virtual std::optional<QByteArray> edidData() const;

Q_SIGNALS:
    void externalBrightnessChangeObserved(DisplayBrightness *self, int brightness);
};

/**
 * Interface for accessing & manipulating brightness of a single screen.
 */
class DisplayBrightnessDetector : public QObject
{
    Q_OBJECT

public:
    explicit DisplayBrightnessDetector(QObject *parent = nullptr);

    /**
     * Detect displays that support brightness operations. Emits detectionFinished() once completed.
     * Can be called repeatedly if needed.
     */
    virtual void detect() = 0;

    /**
     * Retrieve a list of displays that support brightness operations.
     * Call detect() and wait for the detectionFinished() signal before using this.
     *
     * @return true after detectionFinished() is emitted, if a brightness-adjustable display is available.
     *         false otherwise.
     */
    virtual QList<DisplayBrightness *> displays() const = 0;

Q_SIGNALS:
    void detectionFinished(bool isSupported);
    void displaysChanged();
};
