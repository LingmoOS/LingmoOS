/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SensorUnitModel.h"

#include <array>
#include <optional>

#include <QScopeGuard>

#include "formatter/Formatter.h"
#include "formatter/Unit.h"
#include "systemstats/SensorInfo.h"

#include "SensorDaemonInterface_p.h"

using namespace KSysGuard;

struct UnitInfo {
    Unit unit = UnitNone;
    QString symbol;
    qreal multiplier;
};

class Q_DECL_HIDDEN SensorUnitModel::Private
{
public:
    bool insertUnits(const std::array<Unit, 6> &from, Unit start);

    QStringList requestedSensors;
    QSet<QString> processedSensors;
    QMap<Unit, UnitInfo> units;
};

SensorUnitModel::SensorUnitModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
    connect(SensorDaemonInterface::instance(), &SensorDaemonInterface::metaDataChanged, this, &SensorUnitModel::metaDataChanged);
}

SensorUnitModel::~SensorUnitModel() = default;

QStringList SensorUnitModel::sensors() const
{
    return d->requestedSensors;
}

void SensorUnitModel::setSensors(const QStringList &newSensors)
{
    if (newSensors == d->requestedSensors) {
        return;
    }

    beginResetModel();

    d->requestedSensors = newSensors;
    d->processedSensors.clear();
    d->units.clear();

    endResetModel();

    if (d->requestedSensors.size() > 0) {
        SensorDaemonInterface::instance()->requestMetaData(d->requestedSensors);
    }

    Q_EMIT sensorsChanged();
    Q_EMIT readyChanged();
}

bool SensorUnitModel::ready() const
{
    return d->requestedSensors.size() == d->processedSensors.size();
}

QHash<int, QByteArray> SensorUnitModel::roleNames() const
{
    static const QHash<int, QByteArray> roleNames = {
        {UnitRole, "unit"},
        {SymbolRole, "symbol"},
        {MultiplierRole, "multiplier"},
    };
    return roleNames;
}

int SensorUnitModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->units.size();
}

QVariant SensorUnitModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::DoNotUseParent)) {
        return QVariant{};
    }

    auto itr = d->units.begin() + index.row();
    auto entry = itr.value();

    switch (role) {
    case UnitRole:
        return entry.unit;
    case SymbolRole:
        return entry.symbol;
    case MultiplierRole:
        return entry.multiplier;
    }

    return QVariant{};
}

void SensorUnitModel::metaDataChanged(const QString &id, const SensorInfo &info)
{
    if (!d->requestedSensors.contains(id) || d->processedSensors.contains(id)) {
        return;
    }

    d->processedSensors.insert(id);

    auto unit = info.unit;
    if (unit == UnitInvalid || unit == UnitNone) {
        return;
    }

    beginResetModel();

    auto guard = qScopeGuard([this, &id]() {
        endResetModel();
        d->processedSensors.insert(id);
        Q_EMIT readyChanged();
    });

    static const std::array<Unit, 6> bytes = {
        UnitByte,
        UnitKiloByte,
        UnitMegaByte,
        UnitGigaByte,
        UnitTeraByte,
        UnitPetaByte,
    };
    if (d->insertUnits(bytes, unit)) {
        return;
    }

    static const std::array<Unit, 6> rates = {
        UnitByteRate,
        UnitKiloByteRate,
        UnitMegaByteRate,
        UnitGigaByteRate,
        UnitTeraByteRate,
        UnitPetaByteRate,
    };
    if (d->insertUnits(rates, unit)) {
        return;
    }

    static const std::array<Unit, 6> frequencies = {
        UnitHertz,
        UnitKiloHertz,
        UnitMegaHertz,
        UnitGigaHertz,
        UnitTeraHertz,
        UnitPetaHertz,
    };
    if (d->insertUnits(frequencies, unit)) {
        return;
    }

    UnitInfo unitInfo;
    unitInfo.unit = unit;
    unitInfo.symbol = Formatter::symbol(unit);
    unitInfo.multiplier = 1.0;

    d->units.insert(unit, unitInfo);
}

bool SensorUnitModel::Private::insertUnits(const std::array<Unit, 6> &from, Unit start)
{
    auto itr = std::find(from.begin(), from.end(), start);
    if (itr == from.end()) {
        return false;
    }

    auto baseUnit = from.at(0);

    for (; itr != from.end(); ++itr) {
        UnitInfo info;
        info.unit = *itr;
        info.symbol = Formatter::symbol(*itr);
        info.multiplier = Formatter::scaleDownFactor(1.0, start, static_cast<MetricPrefix>((*itr) - baseUnit));
        units.insert(*itr, info);
    }

    return true;
}
