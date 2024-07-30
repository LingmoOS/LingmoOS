/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef UTIL_H
#define UTIL_H

// KF
#include <KUnitConversion/Converter>
// Qt
#include <QObject>

class Util : public QObject
{
    Q_OBJECT

public:
    explicit Util(QObject *parent = nullptr);

public:
    enum Status {
        Normal = 0,
        Connecting,
        NeedsConfiguration,
        Timeout,
    };
    Q_ENUM(Status);

public:
    /**
     * Returns the @p iconName if the current icon theme contains an icon with that name,
     * otherwise returns "weather-not-available" (expecting the icon theme to have that in any case).
     */
    Q_INVOKABLE QString existingWeatherIconName(const QString &iconName) const;

    Q_INVOKABLE QString temperatureToDisplayString(int displayUnitType, double value, int valueUnitType, bool rounded = false, bool degreesOnly = false) const;
    Q_INVOKABLE QString valueToDisplayString(int displayUnitType, double value, int valueUnitType, int precision = 0) const;
    Q_INVOKABLE QString percentToDisplayString(double value) const;

    static QString nameFromUnitId(KUnitConversion::UnitId unitId);

private:
    static KUnitConversion::Converter m_converter;
};

#endif
