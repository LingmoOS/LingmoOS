/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorContainer.h"

#include "SensorObject.h"

#include "systemstats_logging.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN SensorContainer::Private
{
public:
    QString id;
    QString name;
    QHash<QString, SensorObject *> sensorObjects;
};

SensorContainer::SensorContainer(const QString &id, const QString &name, SensorPlugin *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->id = id;
    d->name = name;
    parent->addContainer(this);
}

SensorContainer::~SensorContainer() = default;

QString SensorContainer::id() const
{
    return d->id;
}

QString SensorContainer::name() const
{
    return d->name;
}

QList<SensorObject *> SensorContainer::objects()
{
    return d->sensorObjects.values();
}

SensorObject *SensorContainer::object(const QString &id) const
{
    return d->sensorObjects.value(id);
}

void SensorContainer::addObject(SensorObject *object)
{
    object->setParentContainer(this);

    const QString objectId = object->id();
    if (d->sensorObjects.contains(objectId)) {
        qCWarning(SYSTEMSTATS) << "Tried to add SensorObject with id" << objectId << "to container" << id()
                               << "that already contains an object with that ID, ignoring.";
        return;
    }

    d->sensorObjects[objectId] = object;
    Q_EMIT objectAdded(object);

    connect(object, &SensorObject::aboutToBeRemoved, this, [this, object]() {
        removeObject(object);
    });
}

void SensorContainer::removeObject(SensorObject *object)
{
    if (!d->sensorObjects.contains(object->id())) {
        return;
    }

    object->setParentContainer(nullptr);
    d->sensorObjects.remove(object->id());
    Q_EMIT objectRemoved(object);
}
