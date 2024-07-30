/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorProperty.h"
#include "SensorObject.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN SensorProperty::Private
{
public:
    SensorObject *parent = nullptr;
    SensorInfo info;
    QString id;
    QString name;
    QString prefix;
    QVariant value;
    QVariant initialValue;
    int subscribers = 0;
};

SensorProperty::SensorProperty(const QString &id, SensorObject *parent)
    : SensorProperty(id, QString(), parent)
{
}

SensorProperty::SensorProperty(const QString &id, const QString &name, SensorObject *parent)
    : SensorProperty(id, name, QVariant(), parent)
{
}

SensorProperty::SensorProperty(const QString &id, const QString &name, const QVariant &initialValue, SensorObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->id = id;
    d->parent = parent;
    setName(name);
    d->initialValue = initialValue;
    if (initialValue.isValid()) {
        setValue(initialValue);
    }
    parent->addProperty(this);
}

SensorProperty::~SensorProperty()
{
}

SensorObject *KSysGuard::SensorProperty::parentObject() const
{
    return d->parent;
}

SensorInfo SensorProperty::info() const
{
    return d->info;
}

QString SensorProperty::id() const
{
    return d->id;
}

QString SensorProperty::path() const
{
    return d->parent->path() % QLatin1Char('/') % d->id;
}

void SensorProperty::setName(const QString &name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;
    d->info.name = d->prefix.isEmpty() ? d->name : d->prefix % QLatin1Char(' ') % d->name;
    Q_EMIT sensorInfoChanged();
}

void SensorProperty::setShortName(const QString &name)
{
    if (d->info.shortName == name) {
        return;
    }

    d->info.shortName = name;
    Q_EMIT sensorInfoChanged();
}

void SensorProperty::setPrefix(const QString &prefix)
{
    if (d->prefix == prefix) {
        return;
    }

    d->prefix = prefix;
    d->info.name = prefix.isEmpty() ? d->name : prefix % QLatin1Char(' ') % d->name;
    Q_EMIT sensorInfoChanged();
}

void SensorProperty::setDescription(const QString &description)
{
    if (d->info.description == description) {
        return;
    }

    d->info.description = description;
    Q_EMIT sensorInfoChanged();
}

void SensorProperty::setMin(qreal min)
{
    if (qFuzzyCompare(d->info.min, min)) {
        return;
    }

    d->info.min = min;
    Q_EMIT sensorInfoChanged();
}

void SensorProperty::setMax(qreal max)
{
    if (qFuzzyCompare(d->info.max, max)) {
        return;
    }

    d->info.max = max;
    Q_EMIT sensorInfoChanged();
}

void SensorProperty::setMax(SensorProperty *other)
{
    setMax(other->value().toReal());
    if (isSubscribed()) {
        other->subscribe();
    }
    connect(this, &SensorProperty::subscribedChanged, this, [this, other](bool isSubscribed) {
        if (isSubscribed) {
            other->subscribe();
            setMax(other->value().toReal());
        } else {
            other->unsubscribe();
        }
    });
    connect(other, &SensorProperty::valueChanged, this, [this, other]() {
        setMax(other->value().toReal());
    });
}

void SensorProperty::setUnit(KSysGuard::Unit unit)
{
    if (d->info.unit == unit) {
        return;
    }

    d->info.unit = unit;
    Q_EMIT sensorInfoChanged();
}

void SensorProperty::setVariantType(QVariant::Type type)
{
    if (d->info.variantType == type) {
        return;
    }

    d->info.variantType = type;
    Q_EMIT sensorInfoChanged();
}

bool SensorProperty::isSubscribed() const
{
    return d->subscribers > 0;
}

void SensorProperty::subscribe()
{
    d->subscribers++;
    if (d->subscribers == 1) {
        Q_EMIT subscribedChanged(true);
    }
}

void SensorProperty::unsubscribe()
{
    d->subscribers--;
    if (d->subscribers == 0) {
        if (d->initialValue.isValid()) {
            setValue(d->initialValue);
        }
        Q_EMIT subscribedChanged(false);
    }
}

QVariant SensorProperty::value() const
{
    return d->value;
}

void SensorProperty::setValue(const QVariant &value)
{
    d->value = value;
    Q_EMIT valueChanged();
}
