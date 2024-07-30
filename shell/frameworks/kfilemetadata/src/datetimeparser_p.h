/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2024 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kfilemetadata_debug.h"

#include <QDateTime>
#include <QString>
#include <QTimeZone>

#include <array>

namespace KFileMetaData
{
namespace Parser
{

inline QDateTime dateTimeFromString(const QString &dateString)
{
    if (auto dateTime = QDateTime::fromString(dateString, Qt::ISODate); dateTime.isValid()) {
        return dateTime;
    }

    std::array<QString, 13> formats{
        QStringLiteral("yyyy-MM-dd"),
        QStringLiteral("dd-MM-yyyy"),
        QStringLiteral("yyyy-MM"),
        QStringLiteral("MM-yyyy"),
        QStringLiteral("yyyy.MM.dd"),
        QStringLiteral("dd.MM.yyyy"),
        QStringLiteral("dd MMMM yyyy"),
        QStringLiteral("MM.yyyy"),
        QStringLiteral("yyyy.MM"),
        QStringLiteral("yyyy"),
        QStringLiteral("yy"),
        QStringLiteral("dddd d MMM yyyy h':'mm':'ss AP"),
        QStringLiteral("yyyy:MM:dd hh:mm:ss"),
    };

    for (const auto& formatString : formats) {
        if (auto dateTime = QDateTime::fromString(dateString, formatString); dateTime.isValid()) {
            dateTime.setTimeZone(QTimeZone::UTC);
            return dateTime;
        }
    }

    if (auto dateTime = QLocale().toDateTime(dateString, QLocale::ShortFormat); dateTime.isValid()) {
        dateTime.setTimeZone(QTimeZone::UTC);
        return dateTime;
    }

    if (auto dateTime = QLocale().toDateTime(dateString, QLocale::LongFormat); dateTime.isValid()) {
        dateTime.setTimeZone(QTimeZone::UTC);
        return dateTime;
    }

    qCWarning(KFILEMETADATA_LOG) << "Could not determine correct datetime format from:" << dateString;
    return {};
}

} // namespace Parser
} // namespace KFileMetaData
