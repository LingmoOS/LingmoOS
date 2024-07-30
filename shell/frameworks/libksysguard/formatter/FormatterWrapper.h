/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QFont>
#include <QObject>
#include <QVariant>

#include "Unit.h"

namespace KSysGuard
{
/**
 * Tiny helper class to make Formatter usable from QML.
 *
 * An instance of this class will be exposed as a Singleton object to QML. It
 * allows formatting of values from the QML side.
 *
 * This effectively wraps Formatter::formatValue, removing the FormatOptions flag
 * that I couldn't get to work.
 *
 * It is accessible as `Formatter` inside the `org.kde.ksysguard.formatter` package
 * @see Formatter
 */
class FormatterWrapper : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Formatter)
    QML_SINGLETON

public:
    Q_INVOKABLE QString formatValue(const QVariant &value, KSysGuard::Unit unit, KSysGuard::MetricPrefix targetPrefix = MetricPrefixAutoAdjust);

    Q_INVOKABLE QString formatValueShowNull(const QVariant &value, KSysGuard::Unit unit, KSysGuard::MetricPrefix targetPrefix = MetricPrefixAutoAdjust);

    Q_INVOKABLE QString formatValueWithPrecision(const QVariant &value, KSysGuard::Unit unit, int precision = 1);

    Q_INVOKABLE qreal maximumLength(KSysGuard::Unit unit, const QFont &font);
};

}
