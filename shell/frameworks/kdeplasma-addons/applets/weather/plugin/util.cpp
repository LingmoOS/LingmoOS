/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "util.h"

// KF
#include <KLocalizedString>
#include <KUnitConversion/Value>
// Qt
#include <QIcon>
#include <QLocale>
#include <QPixmap>
// Std
#include <cmath>

template<typename T>
T clampValue(T value, int decimals)
{
    const T mul = std::pow(static_cast<T>(10), decimals);
    return int(value * mul) / mul;
}

Util::Util(QObject *parent)
    : QObject(parent)
{
}

KUnitConversion::Converter Util::m_converter;

QString Util::existingWeatherIconName(const QString &iconName) const
{
    const bool isValid = !iconName.isEmpty() && QIcon::hasThemeIcon(iconName);
    return isValid ? iconName : QStringLiteral("weather-not-available");
}

QString Util::temperatureToDisplayString(int displayUnitType, double value, int valueUnitType, bool rounded, bool degreesOnly) const
{
    KUnitConversion::Value v(value, static_cast<KUnitConversion::UnitId>(valueUnitType));
    v = v.convertTo(static_cast<KUnitConversion::UnitId>(displayUnitType));

    const QString unit = degreesOnly ? i18nc("Degree, unit symbol", "°") : v.unit().symbol();

    if (rounded) {
        int tempNumber = qRound(v.number());
        return i18nc("temperature unitsymbol", "%1 %2", tempNumber, unit);
    }

    const QString formattedTemp = QLocale().toString(clampValue(v.number(), 1), 'f', 1);
    return i18nc("temperature unitsymbol", "%1 %2", formattedTemp, unit);
}

QString Util::valueToDisplayString(int displayUnitType, double value, int valueUnitType, int precision) const
{
    KUnitConversion::Value v(value, static_cast<KUnitConversion::UnitId>(valueUnitType));
    v = v.convertTo(static_cast<KUnitConversion::UnitId>(displayUnitType));

    // TODO: fix KUnitConversion to do locale encoded values and use that
    const QString formattedValue = QLocale().toString(clampValue(v.number(), precision), 'f', precision);
    return i18nc("value unitsymbol", "%1 %2", formattedValue, v.unit().symbol());
}

QString Util::percentToDisplayString(double value) const
{
    const QString formattedPercentValue = QLocale().toString(clampValue(value, 0), 'f', 0);
    return i18nc("value percentsymbol", "%1 %", formattedPercentValue);
}

QString Util::nameFromUnitId(KUnitConversion::UnitId unitId)
{
    const KUnitConversion::Unit unit = m_converter.unit(unitId);
    QString unitDescription = i18nc("@item %1 is a unit description and %2 its unit symbol", "%1 (%2)", unit.description(), unit.symbol());
    return unitDescription;
}
