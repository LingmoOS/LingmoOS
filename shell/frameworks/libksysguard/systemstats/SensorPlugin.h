/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <memory>

#include <QObject>
#include <QVariant>

#include "systemstats_export.h"

namespace KSysGuard
{
class SensorPlugin;
class SensorContainer;

/**
 * Base class for plugins
 */
class SYSTEMSTATS_EXPORT SensorPlugin : public QObject
{
    Q_OBJECT
public:
    SensorPlugin(QObject *parent, const QVariantList &args);
    ~SensorPlugin() override;

    /**
      A list of all containers provided by this plugin
     */
    QList<SensorContainer *> containers() const;

    SensorContainer *container(const QString &id) const;

    /**
     * @brief providerName
     * @returns a non-user facing name of the plugin base
     */
    virtual QString providerName() const;

    /**
     * @brief
     * A hook called before an update will be sent to the user
     */
    virtual void update();

    /**
     * Registers an object as being available for stat retrieval.
     */
    void addContainer(SensorContainer *container);

private:
    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard
