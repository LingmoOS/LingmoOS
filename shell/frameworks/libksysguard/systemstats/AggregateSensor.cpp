/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "AggregateSensor.h"

#include "SensorContainer.h"
#include <QTimer>

using namespace KSysGuard;

// Add two QVariants together.
//
// This will try to add two QVariants together based on the type of first. This
// will try to convert first and second to a common type, add them, then convert
// back to the type of first.
//
// If any conversion fails or there is no way to add two types, first will be
// returned.
QVariant addVariants(const QVariant &first, const QVariant &second)
{
    auto result = QVariant{};

    bool convertFirst = false;
    bool convertSecond = false;

    auto type = first.typeId();
    switch (type) {
    case QMetaType::Char:
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
        result = first.toLongLong(&convertFirst) + second.toLongLong(&convertSecond);
        break;
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        result = first.toULongLong(&convertFirst) + second.toULongLong(&convertSecond);
        break;
    case QMetaType::Float:
    case QMetaType::Double:
        result = first.toDouble(&convertFirst) + second.toDouble(&convertSecond);
        break;
    default:
        return first;
    }

    if (!convertFirst || !convertSecond) {
        return first;
    }

    if (!result.convert(QMetaType(type))) {
        return first;
    }

    return result;
}

class Q_DECL_HIDDEN AggregateSensor::Private
{
public:
    QRegularExpression matchObjects;
    QString matchProperty;
    SensorHash sensors;
    bool dataChangeQueued = false;
    int dataCompressionDuration = 100;
    SensorContainer *subsystem = nullptr;

    AggregateFunction aggregateFunction;
    FilterFunction filterFunction;
};

QVariant AggregateSensor::SensorIterator::operator*() const
{
    return m_it.value()->value();
}

AggregateSensor::SensorIterator &AggregateSensor::SensorIterator::operator++()
{
    do {
        ++m_it;
    } while (!(m_it == m_end || m_it.value()));
    return *this;
}

AggregateSensor::SensorIterator AggregateSensor::SensorIterator::operator++(int)
{
    AggregateSensor::SensorIterator tmp = *this;
    operator++();
    return tmp;
}

bool AggregateSensor::SensorIterator::operator==(const SensorIterator &other) const
{
    return m_it == other.m_it;
}

bool AggregateSensor::SensorIterator::operator!=(const SensorIterator &other) const
{
    return m_it != other.m_it;
}

AggregateSensor::AggregateSensor(SensorObject *provider, const QString &id, const QString &name)
    : AggregateSensor(provider, id, name, QVariant{})
{
}

AggregateSensor::AggregateSensor(SensorObject *provider, const QString &id, const QString &name, const QVariant &initialValue)
    : SensorProperty(id, name, initialValue, provider)
    , d(std::make_unique<Private>())
{
    d->subsystem = qobject_cast<SensorContainer *>(provider->parent());
    setAggregateFunction(addVariants);
    connect(d->subsystem, &SensorContainer::objectAdded, this, &AggregateSensor::updateSensors);
    connect(d->subsystem, &SensorContainer::objectRemoved, this, &AggregateSensor::updateSensors);

    d->filterFunction = [](const SensorProperty *) {
        return true;
    };
}

AggregateSensor::~AggregateSensor() = default;

QRegularExpression AggregateSensor::matchSensors() const
{
    return d->matchObjects;
}

QVariant AggregateSensor::value() const
{
    if (d->sensors.isEmpty()) {
        return QVariant();
    }

    auto it = d->sensors.constBegin();
    while (!it.value() && it != d->sensors.constEnd()) {
        it++;
    }

    if (it == d->sensors.constEnd()) {
        return QVariant{};
    }

    auto begin = SensorIterator(it, d->sensors.constEnd());
    const auto end = SensorIterator(d->sensors.constEnd(), d->sensors.constEnd());
    auto result = d->aggregateFunction(begin, end);

    return result;
}

void AggregateSensor::subscribe()
{
    bool wasSubscribed = SensorProperty::isSubscribed();
    SensorProperty::subscribe();
    if (!wasSubscribed && isSubscribed()) {
        for (auto sensor : std::as_const(d->sensors)) {
            if (sensor) {
                sensor->subscribe();
            }
        }
    }
}

void AggregateSensor::unsubscribe()
{
    bool wasSubscribed = SensorProperty::isSubscribed();
    SensorProperty::unsubscribe();
    if (wasSubscribed && !isSubscribed()) {
        for (auto sensor : std::as_const(d->sensors)) {
            if (sensor) {
                sensor->unsubscribe();
            }
        }
    }
}

void AggregateSensor::setMatchSensors(const QRegularExpression &objectIds, const QString &propertyName)
{
    if (objectIds == d->matchObjects && propertyName == d->matchProperty) {
        return;
    }

    d->matchProperty = propertyName;
    d->matchObjects = objectIds;
    updateSensors();
}

AggregateSensor::AggregateFunction AggregateSensor::aggregateFunction() const
{
    return d->aggregateFunction;
}

void AggregateSensor::setAggregateFunction(const std::function<QVariant(QVariant, QVariant)> &newAggregateFunction)
{
    auto aggregateFunction = [newAggregateFunction](AggregateSensor::SensorIterator begin, const AggregateSensor::SensorIterator end) -> QVariant {
        auto &it = begin;
        QVariant result = *begin;
        ++it;
        for (; it != end; ++it) {
            result = newAggregateFunction(result, *it);
        }
        return result;
    };

    d->aggregateFunction = aggregateFunction;
}

void AggregateSensor::setAggregateFunction(const AggregateFunction &newAggregateFunction)
{
    d->aggregateFunction = newAggregateFunction;
}

AggregateSensor::FilterFunction AggregateSensor::filterFunction() const
{
    return d->filterFunction;
}

void AggregateSensor::setFilterFunction(const FilterFunction &function)
{
    d->filterFunction = function;
    updateSensors();
}

void AggregateSensor::addSensor(SensorProperty *sensor)
{
    if (!sensor || sensor->path() == path() || d->sensors.contains(sensor->path())) {
        return;
    }

    if (!d->filterFunction(sensor)) {
        return;
    }

    if (isSubscribed()) {
        sensor->subscribe();
    }

    connect(sensor, &SensorProperty::valueChanged, this, [this, sensor]() {
        sensorDataChanged(sensor);
    });
    d->sensors.insert(sensor->path(), sensor);
}

void AggregateSensor::removeSensor(const QString &sensorPath)
{
    auto sensor = d->sensors.take(sensorPath);
    sensor->disconnect(this);
    if (isSubscribed()) {
        sensor->unsubscribe();
    }
}

int AggregateSensor::matchCount() const
{
    return d->sensors.size();
}

void AggregateSensor::updateSensors()
{
    if (!d->matchObjects.isValid()) {
        return;
    }

    auto itr = d->sensors.begin();
    while (itr != d->sensors.end()) {
        if (!itr.value()) {
            itr = d->sensors.erase(itr);
        } else if (itr.value()->parent() && itr.value()->parent()->parent() != d->subsystem) {
            itr.value()->disconnect(this);
            if (isSubscribed()) {
                itr.value()->unsubscribe();
            }
            itr = d->sensors.erase(itr);
        } else if (!d->filterFunction(itr.value())) {
            itr.value()->disconnect(this);
            if (isSubscribed()) {
                itr.value()->unsubscribe();
            }
            itr = d->sensors.erase(itr);
        } else {
            ++itr;
        }
    }

    for (auto obj : d->subsystem->objects()) {
        if (d->matchObjects.match(obj->id()).hasMatch()) {
            auto sensor = obj->sensor(d->matchProperty);
            if (sensor) {
                addSensor(sensor);
            }
        }
    }

    delayedEmitDataChanged();
}

void AggregateSensor::sensorDataChanged(SensorProperty *sensor)
{
    Q_UNUSED(sensor)
    delayedEmitDataChanged();
}

void AggregateSensor::delayedEmitDataChanged()
{
    if (!d->dataChangeQueued) {
        d->dataChangeQueued = true;
        QTimer::singleShot(d->dataCompressionDuration, [this]() {
            Q_EMIT valueChanged();
            d->dataChangeQueued = false;
        });
    }
}

class Q_DECL_HIDDEN PercentageSensor::Private
{
public:
    SensorProperty *sensor = nullptr;
};

PercentageSensor::PercentageSensor(SensorObject *provider, const QString &id, const QString &name)
    : SensorProperty(id, name, provider)
    , d(std::make_unique<Private>())
{
    setUnit(KSysGuard::UnitPercent);
    setMax(100);
}

PercentageSensor::~PercentageSensor() = default;

void PercentageSensor::setBaseSensor(SensorProperty *property)
{
    d->sensor = property;
    connect(property, &SensorProperty::valueChanged, this, &PercentageSensor::valueChanged);
    connect(property, &SensorProperty::sensorInfoChanged, this, &PercentageSensor::valueChanged);
}

QVariant PercentageSensor::value() const
{
    if (!d->sensor) {
        return QVariant();
    }
    QVariant value = d->sensor->value();
    if (!value.isValid()) {
        return QVariant();
    }
    return (value.toReal() / d->sensor->info().max) * 100.0;
}

void PercentageSensor::subscribe()
{
    d->sensor->subscribe();
}

void PercentageSensor::unsubscribe()
{
    d->sensor->unsubscribe();
}
