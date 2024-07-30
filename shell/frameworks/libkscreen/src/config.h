/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "kscreen_export.h"
#include "output.h"
#include "screen.h"
#include "types.h"

#include <QHash>
#include <QMetaType>
#include <QObject>

#include <cstdint>
#include <optional>

namespace KScreen
{
class Output;

/**
 * Represents a (or the) screen configuration.
 *
 * This is the main class of KScreen, with it you can use
 * the static methods current() to get the systems config and
 * setConfig() to apply a config to the system.
 *
 * Also, you can instantiate an empty Config, this is usually done
 * to create a config (with the objective of setting it) from scratch
 * and for example unserialize a saved config to it.
 *
 */
class KSCREEN_EXPORT Config : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ScreenPtr screen READ screen)
    Q_PROPERTY(OutputList outputs READ outputs)

public:
    enum class ValidityFlag {
        None = 0x0,
        RequireAtLeastOneEnabledScreen = 0x1,
    };
    Q_ENUM(ValidityFlag)
    Q_DECLARE_FLAGS(ValidityFlags, ValidityFlag)

    /** This indicates which features the used backend supports.
     *
     * @see supportedFeatures
     * @since 5.7
     */
    enum class Feature {
        None = 0, ///< None of the mentioned features are supported.
        PrimaryDisplay = 1, ///< The backend knows about the concept of a primary display
        Writable = 1 << 1, ///< The backend supports setting the config, it's not read-only.
        PerOutputScaling = 1 << 2, ///< The backend supports scaling each output individually.
        OutputReplication = 1 << 3, ///< The backend supports replication of outputs.
        AutoRotation = 1 << 4, ///< The backend supports automatic rotation of outputs.
        TabletMode = 1 << 5, ///< The backend supports querying if a device is in tablet mode.
        SynchronousOutputChanges = 1 << 6, ///< The backend supports blocking until the output setting changes are applied
        XwaylandScales = 1 << 7, ///< The backend supports adapting Xwayland clients to a certain scale
    };
    Q_ENUM(Feature)
    Q_DECLARE_FLAGS(Features, Feature)

    /**
     * Validates that a config can be applied in the current system
     *
     * Each system has different constrains, this method will test
     * the given config with those constrains to see if it
     * can be applied.
     *
     * @arg config to be checked
     * @flags enable additional optional checks
     * @return true if the configuration can be applied, false if not.
     * @since 5.3.0
     */
    static bool canBeApplied(const ConfigPtr &config, ValidityFlags flags);

    /**
     * Validates that a config can be applied in the current system
     *
     * Each system has different constrains, this method will test
     * the given config with those constrains to see if it
     * can be applied.
     *
     * @arg config to be checked
     * @return true if the configuration can be applied, false if not.
     */
    static bool canBeApplied(const ConfigPtr &config);

    /**
     * Instantiate an empty config
     *
     * Usually you do not want to use this constructor since there are some
     * values that make no sense to set (for example you want the Screen of
     * the current systme).
     *
     * So usually what you do is call current() and then modify
     * whatever you need.
     */
    explicit Config();
    ~Config() override;

    /**
     * Duplicates the config
     *
     * @return a new Config instance with same property values
     */
    ConfigPtr clone() const;

    /**
     * Returns an identifying hash for this config in regards to its
     * connected outputs.
     *
     * The hash is calculated with a sorted combination of all
     * connected output hashes.
     *
     * @return sorted hash combination of all connected outputs
     * @since 5.15
     */
    QString connectedOutputsHash() const;

    ScreenPtr screen() const;
    void setScreen(const ScreenPtr &screen);

    OutputPtr output(int outputId) const;
    OutputList outputs() const;
    OutputList connectedOutputs() const;

    /**
     * Find primary output. Primary output is the output with priority 1. May be
     * null.
     */
    OutputPtr primaryOutput() const;
    /**
     * Setting output to be the primary one is equivalent to setting its
     * priority to 1.
     */
    void setPrimaryOutput(const OutputPtr &output);
    /**
     * Add an output to this configuration.
     *
     * This method does not ensure consistency of priorities, it is up to the
     * caller to perform necessary adjustments afterwards. The reason is that it
     * might be used in a loop (such as adding all outputs) where committing
     * intermediate states is undesirable.
     */
    void addOutput(const OutputPtr &output);
    /**
     * Remove an output with matching ID from this configuration.
     *
     * This method does not ensure consistency of priorities, it is up to the
     * caller to perform necessary adjustments afterwards. The reason is that it
     * might be used in a loop (such as removing all outputs) where committing
     * intermediate states is undesirable.
     */
    void removeOutput(int outputId);
    /**
     * Replace all existing outputs with the given ones.
     *
     * Unlike addOutput and removeOutput which operate on individual items
     * presumably in a loop, this method will call adjustPriorities() before
     * returning.
     */
    void setOutputs(const OutputList &outputs);

    /**
     * Set output's priority and call adjustPriorities() trying to retain
     * relative ordering of the output. Setting priority to zero with this
     * method will disable the output, otherwise the output will be enabled.
     */
    void setOutputPriority(const OutputPtr &output, uint32_t priority);

    void setOutputPriorities(QMap<OutputPtr, uint32_t> &priorities);

    /**
     * Ensure consistency and continuity of priorities.
     *
     * Most methods operating on outputs are doing so in loop, where committing
     * intermediate states is undesirable. This method restores the balance by
     * settings priority of all disabled outputs to 0, disabling all outputs
     * whose priority is 0 (i.e. it works in both directions), and sorting all
     * the remaining ones, such that they are numbered strictly sequentially
     * starting from 1.
     * @param keep The output, which priority should stay as close as possible
     * to its current one. It is not possible to guarantee, but the algorithm
     * will do its best to prioritize this output among others, if there happens
     * to be multiple ones with the same priority number.
     */
    void adjustPriorities(std::optional<OutputPtr> keep = std::nullopt);

    bool isValid() const;
    void setValid(bool valid);

    void apply(const ConfigPtr &other);

    /** Indicates features supported by the backend. This exists to allow the user
     * to find out which of the features offered by libkscreen are actually supported
     * by the backend. Not all backends are writable (QScreen, for example is
     * read-only, only XRandR, but not KWayland support the primary display, etc.).
     *
     * @return Flags for features that are supported for this config, determined by
     * the backend.
     * @see setSupportedFeatures
     * @since 5.7
     */
    Features supportedFeatures() const;

    /** Sets the features supported by this backend. This should not be called by the
     * user, but by the backend.
     *
     * @see supportedFeatures
     * @since 5.7
     */
    void setSupportedFeatures(const Features &features);

    /**
     * Indicates that the device supports switching between a default and a tablet mode. This is
     * common for convertibles.
     *
     * @return true when tablet mode is available, otherwise false
     * @see setTabletModeAvailable
     * @since 5.18
     */
    bool tabletModeAvailable() const;

    /** Sets if the device supports a tablet mode. This should not be called by the
     * user, but by the backend.
     *
     * @see tabletModeAvailable
     * @since 5.18
     */
    void setTabletModeAvailable(bool available);

    /**
     * Indicates that the device is currently in tablet mode.
     *
     * @return true when in tablet mode, otherwise false
     * @see setTabletModeEngaged
     * @since 5.18
     */
    bool tabletModeEngaged() const;

    /**
     * Sets if the device is currently in tablet mode. This should not be called by the
     * user, but by the backend.
     *
     * @see tabletModeEngaged
     * @since 5.18
     */
    void setTabletModeEngaged(bool engaged);

    QRect outputGeometryForOutput(const KScreen::Output &output) const;

    QSizeF logicalSizeForOutput(const KScreen::Output &output) const;

    /**
     * Returns the logical size of the output, converted to an integer.
     *
     * Takes the ceiling of non-integer sizes.
     */
    QSize logicalSizeForOutputInt(const KScreen::Output &output) const;

Q_SIGNALS:
    void outputAdded(const KScreen::OutputPtr &output);
    void outputRemoved(int outputId);
    void prioritiesChanged();

private:
    Q_DISABLE_COPY(Config)

    class Private;
    Private *const d;
};

} // KScreen namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(KScreen::Config::Features)

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::ConfigPtr &config);
