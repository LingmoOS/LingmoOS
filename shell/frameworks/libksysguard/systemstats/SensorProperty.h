/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include "SensorInfo.h"
#include "systemstats_export.h"

namespace KSysGuard
{
class SensorObject;

/**
 * Represents a given value source with attached metadata
 * For example, current load for a given CPU core, or a disk capacity
 */
class SYSTEMSTATS_EXPORT SensorProperty : public QObject
{
    Q_OBJECT
public:
    explicit SensorProperty(const QString &id, SensorObject *parent);
    explicit SensorProperty(const QString &id, const QString &name, SensorObject *parent);

    /**
     * Construct a SensorProperty.
     *
     * \param id The unique ID of this SensorProperty.
     * \param name The user-visible name of this SensorProperty.
     * \param initialValue The value that is used when no other value has been set.
     * \param parent The SensorObject that contains this SensorProperty.
     *
     * \note If initialValue is valid, the value of this SensorProperty will be
     * reset to initialValue when the last client unsubscribes from this
     * SensorProperty.
     */
    explicit SensorProperty(const QString &id, const QString &name, const QVariant &initalValue, SensorObject *parent);

    ~SensorProperty() override;

    SensorObject *parentObject() const;

    SensorInfo info() const;

    /**
     * A computer readable ID of the property
     */
    QString id() const;

    /**
     * A deduced path based on the concatenated ID of ourselves + parent IDs
     */
    QString path() const;
    /**
     * A human reabable translated name of the property
     */
    void setName(const QString &name);
    void setShortName(const QString &name);
    void setPrefix(const QString &name);

    void setDescription(const QString &description);
    /**
     * Sets a hint describing the minimum value this value can be.
     * Values are not clipped, it is a hint for graphs.
     * When not relevant, leave unset
     */
    void setMin(qreal min);
    /**
     * Sets a hint describing the maximum value this value can be.
     * Values are not clipped, it is a hint for graphs.
     * When not relevant, leave unset
     */
    void setMax(qreal max);
    /**
     * Shorthand for setting the maximum value to that of another property
     * For example to mark the usedSpace of a disk to be the same as the disk capacity
     */
    void setMax(SensorProperty *other);
    void setUnit(KSysGuard::Unit unit);
    void setVariantType(QVariant::Type type);

    bool isSubscribed() const;

    /**
     * Called when a client requests to get continual updates from this property.
     */
    virtual void subscribe();
    /**
     * Called when a client disconnects or no longer wants updates for this property.
     */
    virtual void unsubscribe();
    /**
     * Returns the last value set for this property
     */
    virtual QVariant value() const;
    /**
     * Update the stored value for this property
     */
    void setValue(const QVariant &value);

    /**
     * Updates the value of this property if possible. The default implementation does nothing.
     */
    virtual void update()
    {
    }

Q_SIGNALS:
    /**
     * Emitted when the value changes
     * Clients should emit this manually if they implement value() themselves
     */
    void valueChanged();
    /**
     * Emitted when the metadata of a sensor changes.
     * min/max etc.
     */
    void sensorInfoChanged();
    /**
     * Emitted when we have our first subscription, or all subscriptions are gone
     */
    void subscribedChanged(bool);

private:
    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard
