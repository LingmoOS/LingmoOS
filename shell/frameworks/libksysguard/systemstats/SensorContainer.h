/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <memory>

#include <QObject>

#include "SensorPlugin.h"

#include "systemstats_export.h"

namespace KSysGuard
{
class SensorObject;

/**
 * Represents a collection of similar sensors.
 * For example: a SensorContainer could represent all CPUs or represent all disks
 */
class SYSTEMSTATS_EXPORT SensorContainer : public QObject
{
    Q_OBJECT
public:
    explicit SensorContainer(const QString &id, const QString &name, SensorPlugin *parent);
    ~SensorContainer() override;

    /**
     * A computer readable ID of this group of sensors
     */
    QString id() const;
    /**
     * A human readable name, used for selection
     */
    QString name() const;

    QList<SensorObject *> objects();
    SensorObject *object(const QString &id) const;

    /**
     * Add an object to the container.
     *
     * It will be exposed to clients as a subitem of this container.
     *
     * \param object The SensorObject to add.
     */
    void addObject(SensorObject *object);

    /**
     * Remove an object from the container.
     *
     * It will no longer be available to clients.
     *
     * \param object The SensorObject to remove.
     */
    void removeObject(SensorObject *object);

Q_SIGNALS:
    /**
     * Emitted when an object has been added
     */
    void objectAdded(SensorObject *object);
    /**
     * Emitted after an object has been removed
     * it may not be valid at this time
     */
    void objectRemoved(SensorObject *object);

private:
    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard
