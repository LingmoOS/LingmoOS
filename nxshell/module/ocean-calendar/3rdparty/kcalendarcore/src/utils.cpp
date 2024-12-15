/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2017 Daniel Vrátil <dvratil@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "utils_p.h"

#include <QDataStream>
#include <QTimeZone>

// To remain backwards compatible we need to (de)serialize QDateTime the way KDateTime
// was (de)serialized
void KCalendarCore::serializeQDateTimeAsKDateTime(QDataStream &out, const QDateTime &dt)
{
    out << dt.date() << dt.time();
    switch (dt.timeSpec()) {
    case Qt::UTC:
        out << static_cast<quint8>('u');
        break;
    case Qt::OffsetFromUTC:
        out << static_cast<quint8>('o') << dt.offsetFromUtc();
        break;
    case Qt::TimeZone:
        serializeQTimeZoneAsSpec(out, dt.timeZone());
        break;
    case Qt::LocalTime:
        out << static_cast<quint8>('c');
        break;
    }
    const bool isDateOnly = dt.date().isValid() && !dt.time().isValid();
    out << quint8(isDateOnly ? 0x01 : 0x00);
}

void KCalendarCore::deserializeKDateTimeAsQDateTime(QDataStream &in, QDateTime &dt)
{
    QDate date;
    QTime time;
    quint8 ts, flags;

    in >> date >> time >> ts;
    switch (static_cast<uchar>(ts)) {
    case 'u':
        dt = QDateTime(date, time, Qt::UTC);
        break;
    case 'o': {
        int offset;
        in >> offset;
        dt = QDateTime(date, time, Qt::OffsetFromUTC, offset);
        break;
    }
    case 'z': {
        QString tzid;
        in >> tzid;
        dt = QDateTime(date, time, QTimeZone(tzid.toUtf8()));
        break;
    }
    case 'c':
        dt = QDateTime(date, time, Qt::LocalTime);
        break;
    }

    // unused, we don't have a special handling for date-only QDateTime
    in >> flags;
}

void KCalendarCore::serializeQTimeZoneAsSpec(QDataStream &out, const QTimeZone &tz)
{
    out << static_cast<quint8>('z') << (tz.isValid() ? QString::fromUtf8(tz.id()) : QString());
}

void KCalendarCore::deserializeSpecAsQTimeZone(QDataStream &in, QTimeZone &tz)
{
    quint8 ts;
    in >> ts;
    switch (static_cast<uchar>(ts)) {
    case 'u':
        tz = QTimeZone::utc();
        return;
    case 'o': {
        int offset;
        in >> offset;
        tz = QTimeZone(offset);
        return;
    }
    case 'z': {
        QString tzid;
        in >> tzid;
        tz = QTimeZone(tzid.toUtf8());
        return;
    }
    case 'c':
        tz = QTimeZone::systemTimeZone();
        break;
    }
}

void KCalendarCore::serializeQDateTimeList(QDataStream &out, const QList<QDateTime> &list)
{
    out << list.size();
    for (const auto &i : list) {
        serializeQDateTimeAsKDateTime(out, i);
    }
}

void KCalendarCore::deserializeQDateTimeList(QDataStream &in, QList<QDateTime> &list)
{
    int size;
    in >> size;
    list.clear();
    list.reserve(size);
    for (int i = 0; i < size; ++i) {
        QDateTime dt;
        deserializeKDateTimeAsQDateTime(in, dt);
        list << dt;
    }
}
