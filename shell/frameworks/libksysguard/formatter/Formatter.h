/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

// Own
#include "Unit.h"
#include "formatter_export.h"

// Qt
#include <QString>
#include <QVariant>

class KLocalizedString;

namespace KSysGuard
{
/**
 * This enum type is used to specify format options.
 */
enum FormatOption {
    FormatOptionNone = 0,
    FormatOptionAgo = 1 << 0,
    FormatOptionShowNull = 1 << 1,
};
Q_DECLARE_FLAGS(FormatOptions, FormatOption)

/**
 * A class for formatting sensor values
 * @see FormatterWrapper, for using it from Qml
 */
class FORMATTER_EXPORT Formatter
{
public:
    /**
     * Returns the scale factor suitable for display.
     *
     * @param value The maximum output value.
     * @param unit The unit of the value.
     * @param targetPrefix Preferred metric prefix.
     */
    static qreal scaleDownFactor(const QVariant &value, Unit unit, MetricPrefix targetPrefix = MetricPrefixAutoAdjust);

    /**
     * Returns localized string that is suitable for display.
     *
     * @param value The maximum output value.
     * @param unit The unit of the value.
     * @param targetPrefix Preferred metric prefix.
     */
    static KLocalizedString localizedString(const QVariant &value, Unit unit, MetricPrefix targetPrefix = MetricPrefixAutoAdjust);

    /**
     * Converts @p value to the appropriate displayable string.
     *
     * The returned string is localized.
     *
     * @param value The value to be converted.
     * @param unit The unit of the value.
     * @param targetPrefix Preferred metric prefix.
     * @param options
     * @param precision The precision to use. If < 0, use a default value.
     */
    static QString formatValue(const QVariant &value,
                               Unit unit,
                               MetricPrefix targetPrefix = MetricPrefixAutoAdjust,
                               FormatOptions options = FormatOptionNone,
                               int precision = -1);

    /**
     * Returns a symbol that corresponds to the given @p unit.
     *
     * The returned unit symbol is localized.
     */
    static QString symbol(Unit unit);

    /**
     * Return the maximum length of a formatted string for the specified unit and font.
     *
     * @param unit The unit to use.
     * @param font The font to use.
     */
    static qreal maximumLength(Unit unit, const QFont &font);
};

} // namespace KSysGuard

Q_DECLARE_OPERATORS_FOR_FLAGS(KSysGuard::FormatOptions)
