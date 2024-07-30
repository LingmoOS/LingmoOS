/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "kscreen_export.h"
#include "types.h"

#include <QObject>
#include <QString>

namespace KScreen
{
class Config;

/**
 * Abstract class for backends.
 */
class KSCREEN_EXPORT AbstractBackend : public QObject
{
    Q_OBJECT

public:
    ~AbstractBackend() override
    {
    }

    /**
     * This is where the backend should perform all initialization. This method
     * is always called right after the backend is created.
     *
     * Default implementation does nothing.
     *
     * @p arguments Optional arguments passed by caller. Used mostly for unit-testing.
     */
    virtual void init(const QVariantMap &arguments);

    /**
     * Returns a user-friendly name of the backend.
     */
    virtual QString name() const = 0;

    /**
     * Returns the name of the DBus service that should be used for this backend.
     *
     * Each backend must have an unique service name (usually something like
     * org.kde.KScreen.Backend.%backendName%) to allow multiple different backends
     * running concurrently.
     */
    virtual QString serviceName() const = 0;

    /**
     * Returns a new Config object, holding Screen, Output objects, etc.
     *
     * @return Config object for the system.
     */
    virtual KScreen::ConfigPtr config() const = 0;

    /**
     * Apply a config object to the system.
     *
     * @param config Configuration to apply
     */
    virtual void setConfig(const KScreen::ConfigPtr &config) = 0;

    /**
     * Returns whether the backend is in valid state.
     *
     * Backends should use this to tell BackendLauncher whether they are capable
     * of operating on the current platform.
     */
    virtual bool isValid() const = 0;

    /**
     * Returns encoded EDID data for given output
     *
     * Default implementation does nothing and returns null QByteArray. Backends
     * that don't support EDID don't have to reimplement this method.
     *
     * @param outputd ID of output to return EDID data for
     */
    virtual QByteArray edid(int outputId) const;

Q_SIGNALS:
    /**
     * Emitted when backend detects a change in configuration
     *
     * It's OK to emit this signal for every single change. The emissions are aggregated
     * in the backend launcher, so that the backend does not spam DBus and client
     * applications.
     *
     * @param config New configuration
     */
    void configChanged(const KScreen::ConfigPtr &config);
};

} // namespace KScreen
