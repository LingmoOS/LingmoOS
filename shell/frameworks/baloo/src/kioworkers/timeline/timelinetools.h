/*
    This file is part of the Nepomuk KDE project.
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOO_TIMELINE_TOOLS_H_
#define BALOO_TIMELINE_TOOLS_H_

class QString;

#include <QDate>
#include <QUrl>

namespace Baloo
{
/**
 * The hierarchy in timeline:/ looks as follows:
 *
 * /
 * |- /today
 * |
 * |- /calendar
 * |  |- /calendar/2010
 * |     |- /calendar/2010/january
 * |        |- /calendar/2010/january/2010-01-01
 * |  |- /calendar/2009
 * |
 * |- /months
 * |  |- /month/2010
 * |     |- /months/2010/january
 * |
 * |- /weeks
 */
enum TimelineFolderType {
    NoFolder = 0,    /// nothing
    RootFolder,      /// the root folder
    CalendarFolder,  /// the calendar folder listing all months
    MonthFolder,     /// a folder listing a month's days (m_date contains the month)
    DayFolder,       /// a folder listing a day (m_date); optionally m_filename is set
};

/**
 * Parse a timeline URL like timeline:/today and return the type of folder it
 * represents. If DayFolder is returned \p date is set to the date that should be listed.
 * Otherwise it is an invalid date. \p filename is optionally set to the name of the file
 * in the folder.
 */
TimelineFolderType parseTimelineUrl(const QUrl& url, QDate* date, QString* filename = nullptr);

/**
  * Remove any double slashes, remove any trailing slashes, and
  * add an initial slash after the scheme.
  */
QUrl canonicalizeTimelineUrl(const QUrl& url);

}

#endif
