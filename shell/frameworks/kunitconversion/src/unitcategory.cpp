/*
 *   SPDX-FileCopyrightText: 2007-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "unitcategory.h"
#include "unit_p.h"
#include "unitcategory_p.h"

#include <KLocalizedString>

#include <QNetworkReply>

#include <QNetworkAccessManager>
#include <QStandardPaths>

namespace KUnitConversion
{
UnitCategoryPrivate::UnitCategoryPrivate()
    : m_id(InvalidCategory)
{
}

UnitCategoryPrivate::UnitCategoryPrivate(CategoryId id, const QString &name, const QString &description)
    : m_id(id)
    , m_name(name)
    , m_description(description)
{
}

UnitCategoryPrivate::~UnitCategoryPrivate()
{
}

UnitCategoryPrivate *UnitCategoryPrivate::clone()
{
    return new UnitCategoryPrivate(*this);
}

bool UnitCategoryPrivate::operator==(const UnitCategoryPrivate &other) const
{
    return (m_id == other.m_id);
}

bool UnitCategoryPrivate::operator!=(const UnitCategoryPrivate &other) const
{
    return !(*this == other);
}

QNetworkAccessManager *UnitCategoryPrivate::nam()
{
    static std::unique_ptr<QNetworkAccessManager> s_nam;
    if (!s_nam) {
        s_nam = std::make_unique<QNetworkAccessManager>();
        s_nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        s_nam->setStrictTransportSecurityEnabled(true);
        s_nam->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/org.kde.unitconversion/hsts/"));
    }
    return s_nam.get();
}

Value UnitCategoryPrivate::convert(const Value &value, const Unit &toUnit)
{
    qreal v = value.unit().toDefault(value.number());
    return Value(toUnit.fromDefault(v), toUnit);
}

UnitCategory::UnitCategory()
    : d(nullptr)
{
}

UnitCategory::UnitCategory(UnitCategoryPrivate *dd)
    : d(dd)
{
}

UnitCategory::UnitCategory(const UnitCategory &other)
    : d(other.d)
{
}

UnitCategory::~UnitCategory()
{
}

UnitCategory &UnitCategory::operator=(const UnitCategory &other)
{
    d = other.d;
    return *this;
}

UnitCategory &UnitCategory::operator=(UnitCategory &&other)
{
    d.swap(other.d);
    return *this;
}

bool UnitCategory::operator==(const UnitCategory &other) const
{
    if (d && other.d) {
        return (*d == *other.d);
    } else {
        return (d == other.d);
    }
}

bool UnitCategory::operator!=(const UnitCategory &other) const
{
    if (d && other.d) {
        return (*d != *other.d);
    } else {
        return (d != other.d);
    }
}

bool UnitCategory::isNull() const
{
    return !d;
}

CategoryId UnitCategory::id() const
{
    if (d) {
        return d->m_id;
    }
    return InvalidCategory;
}

QList<Unit> UnitCategory::units() const
{
    if (d) {
        return d->m_units;
    }
    return QList<Unit>();
}

QList<Unit> UnitCategory::mostCommonUnits() const
{
    if (d) {
        return d->m_mostCommonUnits;
    }
    return QList<Unit>();
}

QStringList UnitCategory::allUnits() const
{
    if (d) {
        return d->m_unitMap.keys();
    }
    return QStringList();
}

bool UnitCategory::hasUnit(const QString &unit) const
{
    if (d) {
        return d->m_unitMap.contains(unit);
    }
    return false;
}

Value UnitCategory::convert(const Value &value, const QString &toUnit) const
{
    if (d && (toUnit.isEmpty() || d->m_unitMap.contains(toUnit)) && value.unit().isValid()) {
        Unit to = toUnit.isEmpty() ? defaultUnit() : d->m_unitMap[toUnit];
        return convert(value, to);
    }
    return Value();
}

Value UnitCategory::convert(const Value &value, UnitId toUnit) const
{
    if (d && d->m_idMap.contains(toUnit) && value.unit().isValid()) {
        return convert(value, d->m_idMap[toUnit]);
    }
    return Value();
}

Value UnitCategory::convert(const Value &value, const Unit &toUnit) const
{
    if (d && !toUnit.isNull()) {
        return d->convert(value, toUnit);
    }
    return Value();
}

Unit UnitCategory::unit(const QString &s) const
{
    if (d) {
        return d->m_unitMap.value(s);
    }
    return Unit();
}

Unit UnitCategory::unit(UnitId unitId) const
{
    if (d && d->m_idMap.contains(unitId)) {
        return d->m_idMap[unitId];
    }
    return Unit();
}

QString UnitCategory::name() const
{
    if (d) {
        return d->m_name;
    }
    return QString();
}

Unit UnitCategory::defaultUnit() const
{
    if (d) {
        return d->m_defaultUnit;
    }
    return Unit();
}

QString UnitCategory::description() const
{
    if (d) {
        return d->m_description;
    }
    return QString();
}

void UnitCategoryPrivate::addDefaultUnit(const Unit &unit)
{
    addCommonUnit(unit);
    m_defaultUnit = unit;
}

void UnitCategoryPrivate::addCommonUnit(const Unit &unit)
{
    addUnit(unit);
    m_mostCommonUnits.append(unit);
}

void UnitCategoryPrivate::addUnit(const Unit &unit)
{
    // ### this is emulating a weak_ptr to break a reference cycle between Unit and UnitCategory
    // ### even without that, this is slicing the polymorphic part of UnitCategory
    // this only works by chance as apart from the ctors those parts contain no logic or data it seems
    unit.d->m_category = this;
    const QStringList list = unit.d->m_matchString.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    Q_ASSERT_X(!list.isEmpty(),
               "UnitCategoryPrivate::addUnit",
               QLatin1String("Empty match string for unit symbol: '%1' '%2' - fix translation?").arg(unit.symbol(), unit.description()).toUtf8().constData());

    for (const QString &name : list) {
        m_unitMap[name] = unit;
    }
    m_idMap[unit.id()] = unit;
    m_units.append(unit);
}

bool UnitCategory::hasOnlineConversionTable() const
{
    return d->hasOnlineConversionTable();
}

UpdateJob* UnitCategory::syncConversionTable(std::chrono::seconds updateSkipPeriod)
{
    return d->syncConversionTable(updateSkipPeriod);
}


UpdateJob::UpdateJob(QNetworkReply *reply)
    : d(reply)
{
    connect(d, &QNetworkReply::finished, this, &UpdateJob::finished);
    connect(d, &QNetworkReply::finished, this, &QObject::deleteLater);
}

UpdateJob::~UpdateJob() = default;

}

#include "moc_unitcategory.cpp"
