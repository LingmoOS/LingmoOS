/*
    This file is part of the Nepomuk KDE project.
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "timelinetools.h"
#include "kio_timeline_debug.h"

#include <QRegularExpression>
#include <QUrlQuery>

namespace
{
QDate applyRelativeDateModificators(const QDate& date, const QMap<QString, QString>& modificators)
{
    QDate newDate(date);
    const QString dayKey = QStringLiteral("relDays");
    const QString weekKey = QStringLiteral("relWeeks");
    const QString monthKey = QStringLiteral("relMonths");
    const QString yearKey = QStringLiteral("relYears");
    bool ok = false;

    if (modificators.contains(yearKey)) {
        int relYears = modificators[yearKey].toInt(&ok);
        if (ok) {
            newDate = newDate.addYears(relYears);
        }
    }
    if (modificators.contains(monthKey)) {
        int relMonths = modificators[monthKey].toInt(&ok);
        if (ok) {
            newDate = newDate.addMonths(relMonths);
        }
    }
    if (modificators.contains(weekKey)) {
        int relWeeks = modificators[weekKey].toInt(&ok);
        if (ok) {
            newDate = newDate.addDays(relWeeks * 7); // we assume weeks have 7 days everywhere. QDate seems to make that assumption too, should be OK.
        }
    }
    if (modificators.contains(dayKey)) {
        int relDays = modificators[dayKey].toInt(&ok);
        if (ok) {
            newDate = newDate.addDays(relDays);
        }
    }
    return newDate;
}
}

QUrl Baloo::canonicalizeTimelineUrl(const QUrl& url) {
    QUrl newUrl = url;
    QString path = url.path();
    if (path.contains(QLatin1String("//"))) {
        QStringList sections = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        path = QLatin1Char('/') + sections.join(QLatin1Char('/'));
        newUrl.setPath(path);
    }
    if ((path.size() > 1) && path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
        newUrl.setPath(path);
    }
    if (!path.startsWith(QLatin1Char('/'))) {
        path = QLatin1Char('/') + path;
        newUrl.setPath(path);
    }
    return newUrl;
}

Baloo::TimelineFolderType Baloo::parseTimelineUrl(const QUrl& url, QDate* date, QString* filename)
{
    qCDebug(KIO_TIMELINE) << url;

    static const QRegularExpression s_dateRegexp(
                    QRegularExpression::anchoredPattern(QStringLiteral("\\d{4}-\\d{2}(?:-(\\d{2}))?")));

    // reset
    *date = QDate();

    QString path = url.path();
    if (path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }

    if (path.isEmpty()) {
        qCDebug(KIO_TIMELINE) << url << "is root folder";
        return RootFolder;
    } else if (path.startsWith(QLatin1String("/today"))) {
        *date = QDate::currentDate();
        if (filename) {
            *filename = path.mid(7);
        }
        qCDebug(KIO_TIMELINE) << url << "is today folder:" << *date;
        return DayFolder;
    } else if (path == QLatin1String("/calendar")) {
        qCDebug(KIO_TIMELINE) << url << "is calendar folder";
        return CalendarFolder;
    } else {
        QStringList sections = path.split(QStringLiteral("/"), Qt::SkipEmptyParts);
        QString dateString;
        QRegularExpressionMatch match = s_dateRegexp.match(sections.last());
        if (match.hasMatch()) {
            dateString = sections.last();
        } else if (sections.count() > 1
                   && (match = s_dateRegexp.match(sections[sections.count() - 2])).hasMatch()) {
            dateString = sections[sections.count() - 2];
            if (filename) {
                *filename = sections.last();
            }
        } else {
            qCWarning(KIO_TIMELINE) << url << "COULD NOT PARSE";
            return NoFolder;
        }

        if (match.captured(1).isEmpty()) {
            // no day -> month listing
            qCDebug(KIO_TIMELINE) << "parsing " << dateString;
            *date = QDate::fromString(dateString, QStringLiteral("yyyy-MM"));
            qCDebug(KIO_TIMELINE) << url << "is month folder:" << date->month() << date->year();
            if (date->month() > 0 && date->year() > 0) {
                return MonthFolder;
            }
        } else {
            qCDebug(KIO_TIMELINE) << "parsing " << dateString;
            typedef QPair<QString, QString> StringPair;
            QUrlQuery query(url);
            const QList<StringPair> queryItems = query.queryItems();
            QMap<QString, QString> map;
            for (const StringPair& pair : queryItems) {
                map.insert(pair.first, pair.second);
            }

            *date = applyRelativeDateModificators(QDate::fromString(dateString, QStringLiteral("yyyy-MM-dd")), map);
            // only in day folders we can have filenames
            qCDebug(KIO_TIMELINE) << url << "is day folder:" << *date;
            if (date->isValid()) {
                return DayFolder;
            }
        }
    }

    return NoFolder;
}
