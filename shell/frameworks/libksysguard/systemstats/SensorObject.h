/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <memory>

#include <QObject>

#include "SensorPlugin.h"
#include "SensorProperty.h"

#include "systemstats_export.h"

namespace KSysGuard
{
class SensorContainer;
class SensorObject;

/**
 * Represents a physical or virtual object for example
 * A CPU core, or a disk
 */
class SYSTEMSTATS_EXPORT SensorObject : public QObject
{
    Q_OBJECT
public:
    explicit SensorObject(const QString &id, const QString &name, SensorContainer *parent = nullptr);
    explicit SensorObject(const QString &id, SensorContainer *parent = nullptr);
    ~SensorObject() override;

    QString id() const;
    QString path() const;
    QString name() const;

    void setName(const QString &newName);
    void setParentContainer(SensorContainer *parent);

    QList<SensorProperty *> sensors() const;
    SensorProperty *sensor(const QString &sensorId) const;

    void addProperty(SensorProperty *property);

    bool isSubscribed() const;

Q_SIGNALS:
    /**
     * Emitted when a client subscribes to one or more of the underlying properties of this object
     */
    void subscribedChanged(bool);

    /**
     * Emitted just before deletion
     * The object is still valid at this point
     */
    void aboutToBeRemoved();

    /**
     * Emitted whenever the object's name changes.
     */
    void nameChanged();

private:
    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard
