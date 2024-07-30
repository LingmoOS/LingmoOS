/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "FaceLoader.h"
#include <QQmlEngine>

using namespace KSysGuard;

class Q_DECL_HIDDEN FaceLoader::Private
{
public:
    Private(FaceLoader *qq)
        : q(qq)
    {
    }
    void setupController();

    FaceLoader *q;

    SensorFaceController *parentController = nullptr;
    SensorFaceController *controller = nullptr;

    QString groupName;

    bool complete = false;

    QJsonArray sensors;
    QString faceId;
    QVariantMap colors;
    QVariantMap labels;
    bool readOnly = true;
    bool showTitle = false;
};

FaceLoader::FaceLoader(QObject *parent)
    : QObject(parent)
    , d(new Private{this})
{
}

FaceLoader::~FaceLoader() = default;

SensorFaceController *FaceLoader::parentController() const
{
    return d->parentController;
}

void FaceLoader::setParentController(SensorFaceController *newParentController)
{
    if (newParentController == d->parentController) {
        return;
    }

    if (d->parentController) {
        d->parentController->disconnect(this);
    }

    if (d->controller) {
        d->controller->deleteLater();
    }

    d->parentController = newParentController;

    d->setupController();

    Q_EMIT parentControllerChanged();
}

QString FaceLoader::faceId() const
{
    return d->faceId;
}

void FaceLoader::setFaceId(const QString &newFaceId)
{
    if (newFaceId == d->faceId) {
        return;
    }

    d->faceId = newFaceId;
    if (d->controller) {
        d->controller->setFaceId(d->faceId);
    }

    Q_EMIT faceIdChanged();
}

QString FaceLoader::groupName() const
{
    return d->groupName;
}

void FaceLoader::setGroupName(const QString &newGroupName)
{
    if (newGroupName == d->groupName) {
        return;
    }

    d->groupName = newGroupName;

    d->setupController();

    Q_EMIT groupNameChanged();
}

QJsonArray FaceLoader::sensors() const
{
    return d->sensors;
}

void FaceLoader::setSensors(const QJsonArray &newSensors)
{
    if (newSensors == d->sensors) {
        return;
    }

    d->sensors = newSensors;

    if (d->controller) {
        d->controller->setHighPrioritySensorIds(d->sensors);
    }

    Q_EMIT sensorsChanged();
}

QVariantMap FaceLoader::colors() const
{
    return d->colors;
}

void FaceLoader::setColors(const QVariantMap &newColors)
{
    if (newColors == d->colors) {
        return;
    }

    d->colors = newColors;
    if (d->controller) {
        d->controller->setSensorColors(d->colors);
    }
    Q_EMIT colorsChanged();
}

QVariantMap FaceLoader::labels() const
{
    return d->labels;
}

void FaceLoader::setLabels(const QVariantMap &newLabels)
{
    if (newLabels == d->labels) {
        return;
    }

    d->labels = newLabels;
    if (d->controller) {
        d->controller->setSensorLabels(d->labels);
    }
    Q_EMIT labelsChanged();
}

bool FaceLoader::readOnly() const
{
    return d->readOnly;
}

void FaceLoader::setReadOnly(bool newReadOnly)
{
    if (newReadOnly == d->readOnly) {
        return;
    }

    d->readOnly = newReadOnly;
    if (d->controller) {
        d->controller->setShouldSync(!d->readOnly);
    }
    Q_EMIT readOnlyChanged();
}

SensorFaceController *FaceLoader::controller() const
{
    return d->controller;
}

void FaceLoader::reload()
{
    d->controller->reloadFaceConfiguration();
}

void FaceLoader::classBegin()
{
}

void FaceLoader::componentComplete()
{
    d->complete = true;
    d->setupController();
}

void FaceLoader::Private::setupController()
{
    if (!parentController || groupName.isEmpty() || !complete) {
        return;
    }

    auto configGroup = parentController->configGroup().group(groupName);
    controller = new SensorFaceController(configGroup, qmlEngine(q), new QQmlEngine(q));
    controller->setShouldSync(readOnly);
    controller->setHighPrioritySensorIds(sensors);
    controller->setSensorColors(colors);
    controller->setSensorLabels(labels);
    controller->setShowTitle(showTitle);
    controller->setFaceId(faceId);

    Q_EMIT q->controllerChanged();
}
