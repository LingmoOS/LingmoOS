/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2013 Alex Merry <alex.merry@kdemail.net>
    SPDX-FileCopyrightText: 2013 John Layt <jlayt@kde.org>
    SPDX-FileCopyrightText: 2010 Michael Leupold <lemma@confuego.org>
    SPDX-FileCopyrightText: 2009 Michael Pyne <mpyne@kde.org>
    SPDX-FileCopyrightText: 2008 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KFORMAT_H
#define KFORMAT_H

#include <kcoreaddons_export.h>

#include <QLocale>
#include <QSharedPointer>
#include <QString>

class QDate;
class QDateTime;

class KFormatPrivate;

/**
 * \file kformat.h
 */

/*
   The code in this class was copied from the old KLocale and modified
   by John Layt (and also Alex Merry) in the KDELIBS 4 to KDE
   Frameworks 5 transition in 2013.

   Albert Astals Cid is the original author of formatSpelloutDuration()
   originally named KLocale::prettyFormatDuration().

   Michael Pyne is the original author of formatByteSize().

   Michael Leupold is the original author of formatRelativeDate(()
   originally part of KFormat::formatDate().
*/

/**
 * @class KFormat kformat.h KFormat
 *
 * KFormat provides support for formatting numbers and datetimes in
 * formats that are not supported by QLocale.
 *
 * @author John Layt <jlayt@kde.org>,
 *         Michael Pyne <mpyne@kde.org>,
 *         Albert Astals Cid <aacid@kde.org>,
 *
 * @short Class for formatting numbers and datetimes.
 * @since 5.0
 */
class KCOREADDONS_EXPORT KFormat final
{
    Q_GADGET

public:
    /**
     * These binary units are used in KDE by the formatByteSize()
     * function.
     *
     * NOTE: There are several different units standards:
     * 1) SI  (i.e. metric), powers-of-10.
     * 2) IEC, powers-of-2, with specific units KiB, MiB, etc.
     * 3) JEDEC, powers-of-2, used for solid state memory sizing which
     *    is why you see flash cards labels as e.g. 4GB.  These (ab)use
     *    the metric units.  Although JEDEC only defines KB, MB, GB, if
     *    JEDEC is selected all units will be powers-of-2 with metric
     *    prefixes for clarity in the event of sizes larger than 1024 GB.
     *
     * Although 3 different dialects are possible this enum only uses
     * metric names since adding all 3 different names of essentially the same
     * unit would be pointless.  Use BinaryUnitDialect to control the exact
     * units returned.
     *
     * @see BinaryUnitDialect
     * @see formatByteSize
     */
    enum BinarySizeUnits {
        /// Auto-choose a unit such that the result is in the range [0, 1000 or 1024)
        DefaultBinaryUnits = -1,

        // The first real unit must be 0 for the current implementation!
        UnitByte, ///<  B         1 byte
        UnitKiloByte, ///<  KiB/KB/kB 1024/1000 bytes.
        UnitMegaByte, ///<  MiB/MB/MB 2^20/10^06 bytes.
        UnitGigaByte, ///<  GiB/GB/GB 2^30/10^09 bytes.
        UnitTeraByte, ///<  TiB/TB/TB 2^40/10^12 bytes.
        UnitPetaByte, ///<  PiB/PB/PB 2^50/10^15 bytes.
        UnitExaByte, ///<  EiB/EB/EB 2^60/10^18 bytes.
        UnitZettaByte, ///<  ZiB/ZB/ZB 2^70/10^21 bytes.
        UnitYottaByte, ///<  YiB/YB/YB 2^80/10^24 bytes.
        UnitLastUnit = UnitYottaByte,
    };

    /**
     * These units are used in KDE by the formatValue() function.
     *
     * @see formatValue
     * @since 5.49
     */
    enum class Unit {
        Other,
        Bit, ///< "bit"
        Byte, ///< "B"
        Meter, ///< "m"
        Hertz, ///< "Hz"
    };

    /**
     * These prefixes are used in KDE by the formatValue()
     * function.
     *
     * IEC prefixes are only defined for integral units of information, e.g.
     * bits and bytes.
     *
     * @see BinarySizeUnits
     * @see formatValue
     * @since 5.49
     */
    enum class UnitPrefix {
        /// Auto-choose a unit such that the result is in the range [0, 1000 or 1024)
        AutoAdjust = -128,

        Yocto = 0, ///<  --/-/y  10^-24
        Zepto, ///<  --/-/z  10^-21
        Atto, ///<  --/-/a  10^-18
        Femto, ///<  --/-/f  10^-15
        Pico, ///<  --/-/p  10^-12
        Nano, ///<  --/-/n  10^-9
        Micro, ///<  --/-/µ  10^-6
        Milli, ///<  --/-/m  10^-3
        Centi, ///<  --/-/c  0.01
        Deci, ///<  --/-/d  0.1
        Unity, ///<  ""      1
        Deca, ///<  --/-/da 10
        Hecto, ///<  --/-/h  100
        Kilo, ///<  Ki/K/k  1024/1000
        Mega, ///<  Mi/M/M  2^20/10^06
        Giga, ///<  Gi/G/G  2^30/10^09
        Tera, ///<  Ti/T/T  2^40/10^12
        Peta, ///<  Pi/P/P  2^50/10^15
        Exa, ///<  Ei/E/E  2^60/10^18
        Zetta, ///<  Zi/Z/Z  2^70/10^21
        Yotta, ///<  Yi/Y/Y  2^80/10^24
    };

    /**
     * This enum chooses what dialect is used for binary units.
     *
     * Note: Although JEDEC abuses the metric prefixes and can therefore be
     * confusing, it has been used to describe *memory* sizes for quite some time
     * and programs should therefore use either Default, JEDEC, or IEC 60027-2
     * for memory sizes.
     *
     * On the other hand network transmission rates are typically in metric so
     * Default, Metric, or IEC (which is unambiguous) should be chosen.
     *
     * Normally choosing DefaultBinaryDialect is the best option as that uses
     * the user's selection for units.  If the user has not selected a preference,
     * IECBinaryDialect will typically be used.
     *
     * @see BinarySizeUnits
     * @see formatByteSize
     */
    enum BinaryUnitDialect {
        DefaultBinaryDialect = -1, ///< Used if no specific preference
        IECBinaryDialect, ///< KiB, MiB, etc. 2^(10*n)
        JEDECBinaryDialect, ///< KB, MB, etc. 2^(10*n)
        MetricBinaryDialect, ///< SI Units, kB, MB, etc. 10^(3*n)
        LastBinaryDialect = MetricBinaryDialect,
    };

    /**
     * Format flags for formatDuration()
     * @see DurationFormatOptions
     */
    enum DurationFormatOption {
        DefaultDuration = 0x0, ///< Default formatting in localized 1:23:45 format
        InitialDuration = 0x1, ///< Default formatting in localized 1h23m45s format
        ShowMilliseconds = 0x2, ///< Include milliseconds in format, e.g. 1:23:45.678
        HideSeconds = 0x4, ///< Hide the seconds, e.g. 1:23 or 1h23m, overrides ShowMilliseconds
        FoldHours = 0x8, ///< Fold the hours into the minutes, e.g. 83:45 or 83m45s, overrides HideSeconds
    };
    /**
     * Stores a combination of #DurationFormatOption values.
     */
    Q_DECLARE_FLAGS(DurationFormatOptions, DurationFormatOption)
    Q_FLAG(DurationFormatOption)

    /**
     * Constructs a KFormat.
     *
     * @param locale the locale to use, defaults to the system locale
     */
    explicit KFormat(const QLocale &locale = QLocale());

    /**
     * Copy constructor
     */
    KFormat(const KFormat &other);

    KFormat &operator=(const KFormat &other);

    /**
     * Destructor
     */
    ~KFormat();

    /**
     * Converts @p size from bytes to the appropriate string representation
     * using the binary unit dialect @p dialect and the specific units @p units.
     *
     * Example:
     * @code
     * QString metric, iec, jedec, small;
     * metric = formatByteSize(1000, 1, KFormat::MetricBinaryDialect, KFormat::UnitKiloByte);
     * iec    = formatByteSize(1024, 1, KFormat::IECBinaryDialect, KFormat::UnitKiloByte);
     * jedec  = formatByteSize(1024, 1, KFormat::JEDECBinaryDialect, KFormat::UnitKiloByte);
     * small  = formatByteSize(100);
     * // metric == "1.0 kB", iec == "1.0 KiB", jedec == "1.0 KB", small == "100 B"
     * @endcode
     *
     * @param size size in bytes
     * @param precision number of places after the decimal point to use.  KDE uses
     *        1 by default so when in doubt use 1.  Whenever KFormat::UnitByte is used
     *        (either explicitly or autoselected from KFormat::DefaultBinaryUnits),
     *        the fractional part is always omitted.
     * @param dialect binary unit standard to use.  Use DefaultBinaryDialect to
     *        use the localized user selection unless you need to use a specific
     *        unit type (such as displaying a flash memory size in JEDEC).
     * @param units specific unit size to use in result.  Use
     *        DefaultBinaryUnits to automatically select a unit that will return
     *        a sanely-sized number.
     * @return converted size as a translated string including the units.
     *         E.g. "1.23 KiB", "2 GB" (JEDEC), "4.2 kB" (Metric).
     * @see BinarySizeUnits
     * @see BinaryUnitDialect
     */

    QString formatByteSize(double size,
                           int precision = 1,
                           KFormat::BinaryUnitDialect dialect = KFormat::DefaultBinaryDialect,
                           KFormat::BinarySizeUnits units = KFormat::DefaultBinaryUnits) const;

    /**
     * Given a number of milliseconds, converts that to a string containing
     * the localized equivalent, e.g. 1:23:45
     *
     * @param msecs Time duration in milliseconds
     * @param options options to use in the duration format
     * @return converted duration as a string - e.g. "1:23:45" "1h23m"
     */

    QString formatDuration(quint64 msecs, KFormat::DurationFormatOptions options = KFormat::DefaultDuration) const;

    /**
     * Given a number of milliseconds, converts that to a string containing
     * the localized equivalent to the requested decimal places.
     *
     * e.g. given formatDuration(60000), returns "1.0 minutes"
     *
     * @param msecs Time duration in milliseconds
     * @param decimalPlaces Decimal places to round off to, defaults to 2
     * @return converted duration as a string - e.g. "5.5 seconds" "23.0 minutes"
     */

    QString formatDecimalDuration(quint64 msecs, int decimalPlaces = 2) const;

    /**
     * Given a number of milliseconds, converts that to a spell-out string containing
     * the localized equivalent.
     *
     * e.g. given formatSpelloutDuration(60001) returns "1 minute"
     *      given formatSpelloutDuration(62005) returns "1 minute and 2 seconds"
     *      given formatSpelloutDuration(90060000) returns "1 day and 1 hour"
     *
     * @param msecs Time duration in milliseconds
     * @return converted duration as a string.
     *         Units not interesting to the user, for example seconds or minutes when the first
     *         unit is day, are not returned because they are irrelevant. The same applies for
     *         seconds when the first unit is hour.
     */
    QString formatSpelloutDuration(quint64 msecs) const;

    /**
     * Returns a string formatted to a relative date style.
     *
     * If the @p date falls within one week before or after the current date
     * then a relative date string will be returned, such as:
     * * Yesterday
     * * Today
     * * Tomorrow
     * * Last Tuesday
     * * Next Wednesday
     *
     * If the @p date falls outside this period then the @p format is used.
     *
     * @param date the date to be formatted
     * @param format the date format to use
     *
     * @return the date as a string
     */
    QString formatRelativeDate(const QDate &date, QLocale::FormatType format) const;

    /**
     * Returns a string formatted to a relative datetime style.
     *
     * If the @p dateTime falls within one week before or after the current date
     * then a relative date string will be returned, such as:
     * * Yesterday at 3:00pm
     * * Today at 3:00pm
     * * Tomorrow at 3:00pm
     * * Last Tuesday at 3:00pm
     * * Next Wednesday at 3:00pm
     *
     * If the @p dateTime falls within one hour of the current time.
     * Then a shorter version is displayed:
     *  * Just a moment ago (for within the same minute)
     *  * 15 minutes ago
     *
     * If the @p dateTime falls outside this period then the date is rendered as:
     *  * Monday, 7 September, 2021 at 7:00 PM : date formatted @p format + " at " + time formatted with @p format
     *
     * With @p format LongFormat, time format used is set to ShortFormat (to omit timezone and seconds).
     *
     * First character is capitalized.
     *
     * @param dateTime the date to be formatted
     * @param format the date format to use
     *
     * @return the date as a string
     */
    QString formatRelativeDateTime(const QDateTime &dateTime, QLocale::FormatType format) const;

    /**
     * Converts @p value to the appropriate string representation
     *
     * Example:
     * @code
     * // sets formatted to "1.0 kbit"
     * auto formatted = format.formatValue(1000, KFormat::Unit::Bit, 1, KFormat::UnitPrefix::Kilo);
     * @endcode
     *
     * @param value value to be formatted
     * @param precision number of places after the decimal point to use.  KDE uses
     *        1 by default so when in doubt use 1.
     * @param unit unit to use in result.
     * @param prefix specific prefix to use in result.  Use UnitPrefix::AutoAdjust
     *        to automatically select an appropriate prefix.
     * @param dialect prefix standard to use.  Use DefaultBinaryDialect to
     *        use the localized user selection unless you need to use a specific
     *        unit type. Only meaningful for KFormat::Unit::Byte, and ignored for
     *        all other units.
     * @return converted size as a translated string including prefix and unit.
     *         E.g. "1.23 KiB", "2 GB" (JEDEC), "4.2 kB" (Metric), "1.2 kbit".
     * @see Unit
     * @see UnitPrefix
     * @see BinaryUnitDialect
     * @since 5.49
     */
    QString formatValue(double value,
                        KFormat::Unit unit,
                        int precision = 1,
                        KFormat::UnitPrefix prefix = KFormat::UnitPrefix::AutoAdjust,
                        KFormat::BinaryUnitDialect dialect = KFormat::DefaultBinaryDialect) const;

    /**
     * Converts @p value to the appropriate string representation
     *
     * Example:
     * @code
     * QString bits, slow, fast;
     * // sets bits to "1.0 kbit", slow to "1.0 kbit/s" and fast to "12.3 Mbit/s".
     * bits = format.formatValue(1000, QStringLiteral("bit"), 1, KFormat::UnitPrefix::Kilo);
     * slow = format.formatValue(1000, QStringLiteral("bit/s");
     * fast = format.formatValue(12.3e6, QStringLiteral("bit/s");
     * @endcode
     *
     * @param value value to be formatted
     * @param precision number of places after the decimal point to use.  KDE uses
     *        1 by default so when in doubt use 1.
     * @param unit unit to use in result.
     * @param prefix specific prefix to use in result.  Use UnitPrefix::AutoAdjust
     *        to automatically select an appropriate prefix.
     * @return converted size as a translated string including prefix and unit.
     *         E.g. "1.2 kbit", "2.4 kB", "12.3 Mbit/s"
     * @see UnitPrefix
     * @since 5.49
     */
    QString formatValue(double value, const QString &unit, int precision = 1, KFormat::UnitPrefix prefix = KFormat::UnitPrefix::AutoAdjust) const;
    /**
     * Converts @p value to the appropriate string representation.
     *
     * Example:
     * @code
     * QString iec, jedec, metric;
     * // Sets iec to "1.0 KiB/s", jedec to "1.0 KB/s" and metric to "1.0 kB/s"
     * iec = format.formatValue(1024, QStringLiteral("B/s"), 1, KFormat::UnitPrefix::AutoAdjust, KFormat::IECBinaryDialect);
     * jedec = format.formatValue(1024, QStringLiteral("B/s"), 1, KFormat::UnitPrefix::AutoAdjust, KFormat::JEDECBinaryDialect);
     * metric = format.formatValue(1000, QStringLiteral("B/s"), 1, KFormat::UnitPrefix::AutoAdjust, KFormat::MetricBinaryDialect);
     * @endcode
     *
     * @param value value to be formatted
     * @param precision number of places after the decimal point to use. 1 is used by default; when
     *        in doubt use 1
     * @param unit unit to use in result
     * @param prefix specific prefix to use in result. Use UnitPrefix::AutoAdjust
     *        to automatically select an appropriate prefix
     * @param dialect prefix standard to use. Use DefaultBinaryDialect to
     *        use the localized user selection unless you need to use a specific
     *        unit type
     * @return converted size as a translated string including prefix and unit.
     *         E.g. "1.2 kbit", "2.4 kB", "12.3 Mbit/s"
     * @see UnitPrefix
     * @since 5.74
     */
    QString formatValue(double value, const QString &unit, int precision, KFormat::UnitPrefix prefix, KFormat::BinaryUnitDialect dialect) const;

private:
    QSharedDataPointer<KFormatPrivate> d;
};

#endif // KFORMAT_H
