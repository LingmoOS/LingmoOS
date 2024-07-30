/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorObject.h"

#include "SensorContainer.h"

#include "systemstats_logging.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN SensorObject::Private
{
public:
    SensorContainer *parent = nullptr;
    QString id;
    QString name;
    QHash<QString, SensorProperty *> sensors;
};

SensorObject::SensorObject(const QString &id, SensorContainer *parent)
    : SensorObject(id, QString(), parent)
{
}

SensorObject::SensorObject(const QString &id, const QString &name, SensorContainer *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->parent = parent;
    d->id = id;
    d->name = name;

    if (parent) {
        QMetaObject::invokeMethod(
            parent,
            [this, parent] {
                parent->addObject(this);
            },
            Qt::QueuedConnection);
    }
}

SensorObject::~SensorObject() = default;

QString SensorObject::id() const
{
    return d->id;
}

QString SensorObject::name() const
{
    return d->name;
}

QString SensorObject::path() const
{
    if (!d->parent) {
        return QString{};
    }

    return d->parent->id() % QLatin1Char('/') % d->id;
}

void SensorObject::setName(const QString &newName)
{
    if (newName == d->name) {
        return;
    }

    d->name = newName;
    Q_EMIT nameChanged();
}

void SensorObject::setParentContainer(SensorContainer *parent)
{
    d->parent = parent;
}

QList<SensorProperty *> SensorObject::sensors() const
{
    return d->sensors.values();
}

SensorProperty *SensorObject::sensor(const QString &sensorId) const
{
    return d->sensors.value(sensorId);
}

void SensorObject::addProperty(SensorProperty *property)
{
    if (d->sensors.contains(property->id())) {
        qCWarning(SYSTEMSTATS) << "Add property" << property->id() << "to object" << id()
                               << "that already contains a property with that ID, overwriting the old property";
    }

    d->sensors[property->id()] = property;

    connect(property, &SensorProperty::subscribedChanged, this, [this]() {
        uint count = std::count_if(d->sensors.constBegin(), d->sensors.constEnd(), [](const SensorProperty *prop) {
            return prop->isSubscribed();
        });
        if (count == 1) {
            Q_EMIT subscribedChanged(true);
        } else if (count == 0) {
            Q_EMIT subscribedChanged(false);
        }
    });
}

bool SensorObject::isSubscribed() const
{
    return std::any_of(d->sensors.constBegin(), d->sensors.constEnd(), [](const SensorProperty *prop) {
        return prop->isSubscribed();
    });
}
