/*
    SPDX-FileCopyrightText: 2019 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorDataModel.h"

#include <chrono>
#include <optional>

#include <QMetaEnum>

#include "formatter/Formatter.h"
#include "systemstats/SensorInfo.h"

#include "SensorDaemonInterface_p.h"
#include "sensors_logging.h"

using namespace KSysGuard;

namespace chrono = std::chrono;

class Q_DECL_HIDDEN SensorDataModel::Private
{
public:
    Private(SensorDataModel *qq)
        : q(qq)
    {
    }

    void sensorsChanged();
    void addSensor(const QString &id);
    void removeSensor(const QString &id);

    QStringList requestedSensors;

    QStringList sensors;
    QStringList objects;

    QHash<QString, SensorInfo> sensorInfos;
    QHash<QString, QVariant> sensorData;
    QVariantMap sensorColors;
    QVariantMap sensorLabels;

    bool usedByQml = false;
    bool componentComplete = false;
    bool loaded = false;
    bool enabled = true;

    std::optional<qreal> minimum;
    std::optional<qreal> maximum;

    std::optional<int> updateRateLimit;
    QHash<int, std::chrono::steady_clock::time_point> lastUpdateTimes;

private:
    SensorDataModel *q;
};

SensorDataModel::SensorDataModel(const QStringList &sensorIds, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private(this))
{
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::sensorAdded, this, &SensorDataModel::onSensorAdded);
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::sensorRemoved, this, &SensorDataModel::onSensorRemoved);
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::metaDataChanged, this, &SensorDataModel::onMetaDataChanged);
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::valueChanged, this, &SensorDataModel::onValueChanged);

    // Empty string is used for entries that do not specify a wildcard object
    d->objects << QStringLiteral("");

    setSensors(sensorIds);
}

SensorDataModel::~SensorDataModel()
{
}

QHash<int, QByteArray> SensorDataModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant SensorDataModel::data(const QModelIndex &index, int role) const
{
    const bool check = checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::DoNotUseParent);
    if (!check) {
        return QVariant();
    }

    auto sensor = d->sensors.at(index.column());
    auto info = d->sensorInfos.value(sensor);
    auto data = d->sensorData.value(sensor);

    switch (role) {
    case Qt::DisplayRole:
    case FormattedValue:
        return Formatter::formatValue(data, info.unit);
    case Value:
        return data;
    case Unit:
        return info.unit;
    case Name:
        return d->sensorLabels.value(sensor, info.name);
    case ShortName: {
        auto it = d->sensorLabels.constFind(sensor);
        if (it != d->sensorLabels.constEnd()) {
            return *it;
        }
        if (info.shortName.isEmpty()) {
            return info.name;
        }
        return info.shortName;
    }
    case Description:
        return info.description;
    case Minimum:
        return info.min;
    case Maximum:
        return info.max;
    case Type:
        return info.variantType;
    case SensorId:
        return sensor;
    case Color:
        if (!d->sensorColors.empty()) {
            return d->sensorColors.value(sensor);
        }
        break;
    case UpdateInterval:
        // TODO: Make this dynamic once the backend supports it.
        return BackendUpdateInterval;
    default:
        break;
    }

    return QVariant();
}

QVariant SensorDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        return QVariant();
    }

    if (section < 0 || section >= d->sensors.size()) {
        return QVariant();
    }

    auto sensor = d->sensors.at(section);
    auto info = d->sensorInfos.value(sensor);

    switch (role) {
    case Qt::DisplayRole:
    case ShortName:
        if (info.shortName.isEmpty()) {
            return info.name;
        }
        return info.shortName;
    case Name:
        return info.name;
    case SensorId:
        return sensor;
    case Unit:
        return info.unit;
    case Description:
        return info.description;
    case Minimum:
        return info.min;
    case Maximum:
        return info.max;
    case Type:
        return info.variantType;
    case UpdateInterval:
        // TODO: Make this dynamic once the backend supports it.
        return BackendUpdateInterval;
    default:
        break;
    }

    return QVariant();
}

int SensorDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->objects.count();
}

int SensorDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->sensorInfos.count();
}

qreal SensorDataModel::minimum() const
{
    if (d->sensors.isEmpty()) {
        return 0;
    }

    if (d->minimum.has_value()) {
        return d->minimum.value();
    }

    auto result = std::min_element(d->sensorInfos.cbegin(), d->sensorInfos.cend(), [](const SensorInfo &first, const SensorInfo &second) {
        return first.min < second.min;
    });
    if (result == d->sensorInfos.cend()) {
        d->minimum = 0.0;
    } else {
        d->minimum = (*result).min;
    }
    return d->minimum.value();
}

qreal SensorDataModel::maximum() const
{
    if (d->sensors.isEmpty()) {
        return 0;
    }

    if (d->maximum.has_value()) {
        return d->maximum.value();
    }

    auto result = std::max_element(d->sensorInfos.cbegin(), d->sensorInfos.cend(), [](const SensorInfo &first, const SensorInfo &second) {
        return first.max < second.max;
    });
    if (result == d->sensorInfos.cend()) {
        d->maximum = 0.0;
    } else {
        d->maximum = (*result).max;
    }
    return d->maximum.value();
}

QStringList SensorDataModel::sensors() const
{
    return d->requestedSensors;
}

void SensorDataModel::setSensors(const QStringList &sensorIds)
{
    if (d->requestedSensors == sensorIds) {
        return;
    }

    d->requestedSensors = sensorIds;

    if (!d->usedByQml || d->componentComplete) {
        d->sensorsChanged();
    }
    Q_EMIT readyChanged();
    Q_EMIT sensorsChanged();
}

bool SensorDataModel::enabled() const
{
    return d->enabled;
}

void SensorDataModel::setEnabled(bool newEnabled)
{
    if (newEnabled == d->enabled) {
        return;
    }

    d->enabled = newEnabled;
    if (d->enabled) {
        SensorDaemonInterface::instance()->subscribe(d->sensorInfos.keys());
        SensorDaemonInterface::instance()->requestMetaData(d->sensorInfos.keys());
    } else {
        SensorDaemonInterface::instance()->unsubscribe(d->sensorInfos.keys());
    }

    Q_EMIT enabledChanged();
}

QVariantMap SensorDataModel::sensorColors() const
{
    return d->sensorColors;
}

void SensorDataModel::setSensorColors(const QVariantMap &sensorColors)
{
    if (sensorColors == d->sensorColors) {
        return;
    }
    d->sensorColors = sensorColors;
    Q_EMIT sensorColorsChanged();
    Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Color});
}

QVariantMap SensorDataModel::sensorLabels() const
{
    return d->sensorLabels;
}

void SensorDataModel::setSensorLabels(const QVariantMap &sensorLabels)
{
    if (sensorLabels == d->sensorLabels) {
        return;
    }
    d->sensorLabels = sensorLabels;
    Q_EMIT sensorLabelsChanged();
    Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Name, ShortName});
}

int SensorDataModel::updateRateLimit() const
{
    return d->updateRateLimit.value_or(-1);
}

void SensorDataModel::setUpdateRateLimit(int newUpdateRateLimit)
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
    d->lastUpdateTimes.clear();
    Q_EMIT updateRateLimitChanged();
}

void KSysGuard::SensorDataModel::resetUpdateRateLimit()
{
    setUpdateRateLimit(-1);
}

bool KSysGuard::SensorDataModel::isReady() const
{
    return d->sensors.size() == d->sensorInfos.size();
}

void SensorDataModel::addSensor(const QString &sensorId)
{
    d->addSensor(sensorId);
}

void SensorDataModel::removeSensor(const QString &sensorId)
{
    d->removeSensor(sensorId);
}

int KSysGuard::SensorDataModel::column(const QString &sensorId) const
{
    return d->sensors.indexOf(sensorId);
}

void KSysGuard::SensorDataModel::classBegin()
{
    d->usedByQml = true;
}

void KSysGuard::SensorDataModel::componentComplete()
{
    d->componentComplete = true;

    d->sensorsChanged();

    Q_EMIT sensorsChanged();
}

void SensorDataModel::Private::addSensor(const QString &id)
{
    if (!requestedSensors.contains(id)) {
        return;
    }

    qCDebug(LIBKSYSGUARD_SENSORS) << "Add Sensor" << id;

    sensors.append(id);
    SensorDaemonInterface::instance()->subscribe(id);
    SensorDaemonInterface::instance()->requestMetaData(id);
}

void SensorDataModel::Private::removeSensor(const QString &id)
{
    const int col = sensors.indexOf(id);
    if (col == -1) {
        return;
    }

    q->beginRemoveColumns(QModelIndex(), col, col);

    sensors.removeAt(col);
    sensorInfos.remove(id);
    sensorData.remove(id);

    q->endRemoveColumns();
}

void SensorDataModel::onSensorAdded(const QString &sensorId)
{
    if (!d->enabled) {
        return;
    }

    d->addSensor(sensorId);
}

void SensorDataModel::onSensorRemoved(const QString &sensorId)
{
    if (!d->enabled) {
        return;
    }

    d->removeSensor(sensorId);
}

void SensorDataModel::onMetaDataChanged(const QString &sensorId, const SensorInfo &info)
{
    if (!d->enabled) {
        return;
    }

    auto column = d->sensors.indexOf(sensorId);
    if (column == -1) {
        return;
    }

    qCDebug(LIBKSYSGUARD_SENSORS) << "Received metadata change for" << sensorId;

    d->minimum.reset();
    d->maximum.reset();

    // Simple case: Just an update for a sensor's metadata
    if (d->sensorInfos.contains(sensorId)) {
        d->sensorInfos[sensorId] = info;
        Q_EMIT dataChanged(index(0, column), index(0, column), {Qt::DisplayRole, Name, ShortName, Description, Unit, Minimum, Maximum, Type, FormattedValue});
        Q_EMIT sensorMetaDataChanged();
        return;
    }

    // Otherwise, it's a new sensor that was added

    // Ensure we do not insert columns that are out of range.
    while (d->sensorInfos.count() + 1 <= column && column > 0) {
        column--;
    }

    beginInsertColumns(QModelIndex{}, column, column);
    d->sensorInfos[sensorId] = info;
    d->sensorData[sensorId] = QVariant{};
    endInsertColumns();

    SensorDaemonInterface::instance()->requestValue(sensorId);
    Q_EMIT sensorMetaDataChanged();

    if (isReady()) {
        Q_EMIT readyChanged();
    }
}

void SensorDataModel::onValueChanged(const QString &sensorId, const QVariant &value)
{
    const auto column = d->sensors.indexOf(sensorId);
    if (column == -1 || !d->enabled) {
        return;
    }

    if (d->updateRateLimit && d->sensorData[sensorId].isValid()) {
        auto updateRateLimit = chrono::steady_clock::duration(chrono::milliseconds(d->updateRateLimit.value()));
        auto now = chrono::steady_clock::now();
        if (d->lastUpdateTimes.contains(column) && now - d->lastUpdateTimes.value(column) < updateRateLimit) {
            return;
        } else {
            d->lastUpdateTimes[column] = now;
        }
    }

    d->sensorData[sensorId] = value;
    Q_EMIT dataChanged(index(0, column), index(0, column), {Qt::DisplayRole, Value, FormattedValue});
}

void SensorDataModel::Private::sensorsChanged()
{
    q->beginResetModel();

    SensorDaemonInterface::instance()->unsubscribe(sensors);

    sensors.clear();
    sensorData.clear();
    sensorInfos.clear();
    lastUpdateTimes.clear();

    sensors = requestedSensors;

    SensorDaemonInterface::instance()->subscribe(requestedSensors);
    SensorDaemonInterface::instance()->requestMetaData(requestedSensors);

    q->endResetModel();
}
