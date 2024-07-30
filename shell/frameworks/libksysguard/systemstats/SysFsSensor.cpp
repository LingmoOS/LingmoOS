/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SysFsSensor.h"

#include <QFile>

using namespace KSysGuard;

class Q_DECL_HIDDEN SysFsSensor::Private
{
public:
    QString path;
    std::function<QVariant(const QByteArray &)> convertFunction;
};

SysFsSensor::SysFsSensor(const QString &id, const QString &path, SensorObject *parent)
    : SysFsSensor(id, path, QVariant{}, parent)
{
}

SysFsSensor::SysFsSensor(const QString &id, const QString &path, const QVariant &initialValue, SensorObject *parent)
    : SensorProperty(id, id, initialValue, parent)
    , d(std::make_unique<Private>())
{
    d->path = path;

    d->convertFunction = [](const QByteArray &input) {
        return std::atoll(input);
    };
}

SysFsSensor::~SysFsSensor() = default;

void SysFsSensor::setConvertFunction(const std::function<QVariant(const QByteArray &)> &function)
{
    d->convertFunction = function;
}

void SysFsSensor::update()
{
    if (!isSubscribed()) {
        return;
    }

    QFile file(d->path);
    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    auto value = file.readAll();
    setValue(d->convertFunction(value));
}
