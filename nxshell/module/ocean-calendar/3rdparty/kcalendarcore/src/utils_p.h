/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2017 Daniel Vrátil <dvratil@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALCORE_UTILS_H
#define KCALCORE_UTILS_H

#include <QDateTime>

class QDataStream;

namespace KCalendarCore {
/**
 * Helpers to retain backwards compatibility of binary serialization.
 */
void serializeQDateTimeAsKDateTime(QDataStream &out, const QDateTime &dt);
Q_CORE_EXPORT void deserializeKDateTimeAsQDateTime(QDataStream &in, QDateTime &dt);

void serializeQDateTimeList(QDataStream &out, const QList<QDateTime> &list);
void deserializeQDateTimeList(QDataStream &in, QList<QDateTime> &list);

void serializeQTimeZoneAsSpec(QDataStream &out, const QTimeZone &tz);
void deserializeSpecAsQTimeZone(QDataStream &in, QTimeZone &tz);

} // namespace KCalendarCore

#endif
