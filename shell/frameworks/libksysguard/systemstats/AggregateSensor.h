/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemsta@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <functional>
#include <iterator>
#include <memory>

#include <QList>
#include <QPointer>
#include <QRegularExpression>
#include <QVariant>

#include "SensorObject.h"
#include "SensorPlugin.h"
#include "SensorProperty.h"

#include "systemstats_export.h"

namespace KSysGuard
{

using SensorHash = QHash<QString, QPointer<SensorProperty>>;

/**
 * @todo write docs
 */
class SYSTEMSTATS_EXPORT AggregateSensor : public SensorProperty
{
    Q_OBJECT

public:
    class SensorIterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = QVariant;
        using difference_type = SensorHash::size_type;
        using pointer = void;
        using reference = QVariant &;

        SensorIterator(const SensorIterator &) = default;
        SensorIterator &operator=(const SensorIterator &) = default;
        ~SensorIterator() = default;

        SensorIterator(SensorHash::const_iterator begin, const SensorHash::const_iterator end)
            : m_it(begin)
            , m_end(end)
        {
        }

        value_type operator*() const;
        SensorIterator &operator++();
        SensorIterator operator++(int);
        bool operator==(const SensorIterator &other) const;
        bool operator!=(const SensorIterator &other) const;

    private:
        SensorHash::const_iterator m_it;
        const SensorHash::const_iterator m_end;
    };

    using AggregateFunction = std::function<QVariant(SensorIterator, const SensorIterator)>;
    using FilterFunction = std::function<bool(const SensorProperty *)>;

    AggregateSensor(SensorObject *provider, const QString &id, const QString &name);
    AggregateSensor(SensorObject *provider, const QString &id, const QString &name, const QVariant &initialValue);
    ~AggregateSensor() override;

    QVariant value() const override;
    void subscribe() override;
    void unsubscribe() override;

    QRegularExpression matchSensors() const;
    void setMatchSensors(const QRegularExpression &objectMatch, const QString &propertyId);
    AggregateFunction aggregateFunction() const;
    void setAggregateFunction(const std::function<QVariant(QVariant, QVariant)> &function);
    void setAggregateFunction(const AggregateFunction &function);
    FilterFunction filterFunction() const;
    void setFilterFunction(const FilterFunction &function);

    void addSensor(SensorProperty *sensor);
    void removeSensor(const QString &sensorPath);

    int matchCount() const;

private:
    void updateSensors();
    void sensorDataChanged(SensorProperty *sensor);
    void delayedEmitDataChanged();

    class Private;
    const std::unique_ptr<Private> d;
};

class SYSTEMSTATS_EXPORT PercentageSensor : public SensorProperty
{
    Q_OBJECT
public:
    PercentageSensor(SensorObject *provider, const QString &id, const QString &name);
    ~PercentageSensor() override;

    QVariant value() const override;
    void subscribe() override;
    void unsubscribe() override;

    void setBaseSensor(SensorProperty *sensor);

private:
    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard
