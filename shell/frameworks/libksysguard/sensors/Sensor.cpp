/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "Sensor.h"

#include <chrono>
#include <optional>

#include <QEvent>

#include "formatter/Formatter.h"
#include "systemstats/SensorInfo.h"

#include "Sensor.h"
#include "SensorDaemonInterface_p.h"
#include "SensorQuery.h"

using namespace KSysGuard;
namespace chrono = std::chrono;

class Q_DECL_HIDDEN Sensor::Private
{
public:
    SensorInfo sensorInfo;

    Sensor::Status status = Sensor::Status::Unknown;
    QVariant value;

    bool usedByQml = false;
    bool componentComplete = false;

    QString pendingId;
    QString id;

    bool enabled = true;

    std::optional<int> updateRateLimit;
    chrono::steady_clock::time_point lastUpdate;
};

Sensor::Sensor(QObject *parent)
    : Sensor(QString{}, parent)
{
}

Sensor::Sensor(const QString &id, QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    connect(this, &Sensor::statusChanged, this, &Sensor::valueChanged);
    connect(this, &Sensor::statusChanged, this, &Sensor::metaDataChanged);
    connect(this, &Sensor::enabledChanged, this, &Sensor::onEnabledChanged);

    setSensorId(id);
}

Sensor::Sensor(const SensorQuery &query, int index, QObject *parent)
    : Sensor(QString{}, parent)
{
    if (index >= 0 && index < query.result().size()) {
        auto result = query.result().at(index);
        d->id = result.first;
        onMetaDataChanged(d->id, result.second);
        onEnabledChanged();
    }
}

bool Sensor::event(QEvent *event)
{
    if (event->type() == QEvent::ParentAboutToChange && parent()) {
        parent()->disconnect(this);
    } else if (event->type() == QEvent::ParentChange && parent()) {
        if (parent()->metaObject()->indexOfSignal("enabledChanged()") != -1) {
            connect(parent(), SIGNAL(enabledChanged()), this, SIGNAL(enabledChanged()));
        }
    }

    return QObject::event(event);
}

Sensor::~Sensor()
{
    SensorDaemonInterface::instance()->unsubscribe(d->id);
}

QString Sensor::sensorId() const
{
    return d->id;
}

void Sensor::setSensorId(const QString &id)
{
    if (id == d->id) {
        return;
    }

    if (d->usedByQml && !d->componentComplete) {
        d->pendingId = id;
        return;
    }

    d->id = id;
    d->status = Sensor::Status::Loading;

    if (!id.isEmpty()) {
        SensorDaemonInterface::instance()->requestMetaData(id);
        connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::metaDataChanged, this, &Sensor::onMetaDataChanged, Qt::UniqueConnection);
    }

    if (enabled()) {
        SensorDaemonInterface::instance()->subscribe(id);
        SensorDaemonInterface::instance()->requestValue(id);
        connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::valueChanged, this, &Sensor::onValueChanged, Qt::UniqueConnection);
    }

    Q_EMIT sensorIdChanged();
    Q_EMIT statusChanged();
}

Sensor::Status Sensor::status() const
{
    return d->status;
}

QString Sensor::name() const
{
    return d->sensorInfo.name;
}

QString Sensor::shortName() const
{
    if (d->sensorInfo.shortName.isEmpty()) {
        return d->sensorInfo.name;
    }

    return d->sensorInfo.shortName;
}

QString Sensor::description() const
{
    return d->sensorInfo.description;
}

Unit Sensor::unit() const
{
    return d->sensorInfo.unit;
}

qreal Sensor::minimum() const
{
    return d->sensorInfo.min;
}

qreal Sensor::maximum() const
{
    return d->sensorInfo.max;
}

QVariant::Type Sensor::type() const
{
    return d->sensorInfo.variantType;
}

QVariant Sensor::value() const
{
    if (!d->value.isValid()) {
        return QVariant{d->sensorInfo.variantType};
    }
    return d->value;
}

QString Sensor::formattedValue() const
{
    return Formatter::formatValue(value(), unit(), MetricPrefixAutoAdjust, FormatOptionShowNull);
}

bool Sensor::enabled() const
{
    if (d->enabled && parent()) {
        auto parentEnabled = parent()->property("enabled");
        if (parentEnabled.isValid()) {
            return parentEnabled.toBool();
        }
    }

    return d->enabled;
}

void Sensor::setEnabled(bool newEnabled)
{
    if (newEnabled == d->enabled) {
        return;
    }

    d->enabled = newEnabled;
    Q_EMIT enabledChanged();
}

uint Sensor::updateInterval() const
{
    // TODO: Make this dynamic once the backend supports that.
    return BackendUpdateInterval;
}

int Sensor::updateRateLimit() const
{
    return d->updateRateLimit.value_or(-1);
}

void Sensor::setUpdateRateLimit(int newUpdateRateLimit)
{
    // An update rate limit of 0 or less makes no sense, so treat it as clearing
    // the limit.
    if (newUpdateRateLimit <= 0) {
        if (!d->updateRateLimit) {
            return;
        }

        d->updateRateLimit.reset();
    } else {
        if (d->updateRateLimit && d->updateRateLimit.value() == newUpdateRateLimit) {
            return;
        }

        d->updateRateLimit = newUpdateRateLimit;
    }

    d->lastUpdate = chrono::steady_clock::now();
    Q_EMIT updateRateLimitChanged();
}

void KSysGuard::Sensor::resetUpdateRateLimit()
{
    setUpdateRateLimit(-1);
}

void Sensor::classBegin()
{
    d->usedByQml = true;
}

void Sensor::componentComplete()
{
    d->componentComplete = true;

    setSensorId(d->pendingId);

    if (parent() && parent()->metaObject()->indexOfSignal("enabledChanged()") != -1) {
        connect(parent(), SIGNAL(enabledChanged()), this, SIGNAL(enabledChanged()));
    }
}

void Sensor::onMetaDataChanged(const QString &sensorId, const SensorInfo &metaData)
{
    if (sensorId != d->id || !enabled()) {
        return;
    }

    d->sensorInfo = metaData;

    if (d->status == Sensor::Status::Loading) {
        d->status = Sensor::Status::Ready;
        Q_EMIT statusChanged();
    }

    Q_EMIT metaDataChanged();
}

void Sensor::onValueChanged(const QString &sensorId, const QVariant &value)
{
    if (sensorId != d->id || !enabled()) {
        return;
    }

    if (d->updateRateLimit && d->value.isValid()) {
        auto updateRateLimit = chrono::steady_clock::duration(chrono::milliseconds(d->updateRateLimit.value()));
        auto now = chrono::steady_clock::now();
        if (now - d->lastUpdate < updateRateLimit) {
            return;
        } else {
            d->lastUpdate = now;
        }
    }

    d->value = value;
    Q_EMIT valueChanged();
}

void Sensor::onEnabledChanged()
{
    if (enabled()) {
        connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::metaDataChanged, this, &Sensor::onMetaDataChanged, Qt::UniqueConnection);
        connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::valueChanged, this, &Sensor::onValueChanged, Qt::UniqueConnection);

        SensorDaemonInterface::instance()->subscribe(d->id);
        // Force an update of metadata and data, since that may have changed
        // while we were disabled.
        SensorDaemonInterface::instance()->requestMetaData(d->id);
        SensorDaemonInterface::instance()->requestValue(d->id);
    } else {
        disconnect(SensorDaemonInterface::instance(), &SensorDaemonInterface::metaDataChanged, this, &Sensor::onMetaDataChanged);
        disconnect(SensorDaemonInterface::instance(), &SensorDaemonInterface::valueChanged, this, &Sensor::onValueChanged);
        SensorDaemonInterface::instance()->unsubscribe(d->id);
    }
}
