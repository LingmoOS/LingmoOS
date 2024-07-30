/*  This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2013 Alex Merry <alex.merry@kdemail.net>
    SPDX-FileCopyrightText: 2013 John Layt <jlayt@kde.org>
    SPDX-FileCopyrightText: 2010 Michael Leupold <lemma@confuego.org>
    SPDX-FileCopyrightText: 2009 Michael Pyne <mpyne@kde.org>
    SPDX-FileCopyrightText: 2008 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kformatprivate_p.h"

KFormat::KFormat(const QLocale &locale)
    : d(new KFormatPrivate(locale))
{
}

KFormat::KFormat(const KFormat &other)
    : d(other.d)
{
}

KFormat &KFormat::operator=(const KFormat &other)
{
    d = other.d;
    return *this;
}

KFormat::~KFormat()
{
}

QString KFormat::formatByteSize(double size, int precision, KFormat::BinaryUnitDialect dialect, KFormat::BinarySizeUnits units) const
{
    return d->formatByteSize(size, precision, dialect, units);
}

QString KFormat::formatValue(double value, KFormat::Unit unit, int precision, KFormat::UnitPrefix prefix, KFormat::BinaryUnitDialect dialect) const
{
    return d->formatValue(value, unit, QString(), precision, prefix, dialect);
}

QString KFormat::formatValue(double value, const QString &unit, int precision, KFormat::UnitPrefix prefix) const
{
    return d->formatValue(value, KFormat::Unit::Other, unit, precision, prefix, MetricBinaryDialect);
}

// TODO KF6 Merge both methods
QString KFormat::formatValue(double value, const QString &unit, int precision, KFormat::UnitPrefix prefix, KFormat::BinaryUnitDialect dialect) const
{
    return d->formatValue(value, KFormat::Unit::Other, unit, precision, prefix, dialect);
}

QString KFormat::formatDuration(quint64 msecs, KFormat::DurationFormatOptions options) const
{
    return d->formatDuration(msecs, options);
}

QString KFormat::formatDecimalDuration(quint64 msecs, int decimalPlaces) const
{
    return d->formatDecimalDuration(msecs, decimalPlaces);
}

QString KFormat::formatSpelloutDuration(quint64 msecs) const
{
    return d->formatSpelloutDuration(msecs);
}

QString KFormat::formatRelativeDate(const QDate &date, QLocale::FormatType format) const
{
    return d->formatRelativeDate(date, format);
}

QString KFormat::formatRelativeDateTime(const QDateTime &dateTime, QLocale::FormatType format) const
{
    return d->formatRelativeDateTime(dateTime, format);
}

#include "moc_kformat.cpp"
