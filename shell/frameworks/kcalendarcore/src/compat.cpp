/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and defines
  classes for managing compatibility between different calendar formats.

  @brief
  Classes that provide compatibility to older or "broken" calendar formats.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "compat_p.h"
#include "incidence.h"

#include "kcalendarcore_debug.h"

#include <QDate>
#include <QRegularExpression>
#include <QString>

using namespace KCalendarCore;

Compat *CompatFactory::createCompat(const QString &productId, const QString &implementationVersion)
{
    Compat *compat = nullptr;

    int korg = productId.indexOf(QLatin1String("KOrganizer"));
    int outl9 = productId.indexOf(QLatin1String("Outlook 9.0"));

    if (korg >= 0) {
        int versionStart = productId.indexOf(QLatin1Char(' '), korg);
        if (versionStart >= 0) {
            int versionStop = productId.indexOf(QRegularExpression(QStringLiteral("[ /]")), versionStart + 1);
            if (versionStop >= 0) {
                QString version = productId.mid(versionStart + 1, versionStop - versionStart - 1);

                int versionNum = version.section(QLatin1Char('.'), 0, 0).toInt() * 10000 + version.section(QLatin1Char('.'), 1, 1).toInt() * 100
                    + version.section(QLatin1Char('.'), 2, 2).toInt();
                int releaseStop = productId.indexOf(QLatin1Char('/'), versionStop);
                QString release;
                if (releaseStop > versionStop) {
                    release = productId.mid(versionStop + 1, releaseStop - versionStop - 1);
                }
                if (versionNum < 30100) {
                    compat = new CompatPre31;
                } else if (versionNum < 30200) {
                    compat = new CompatPre32;
                } else if (versionNum == 30200 && release == QLatin1String("pre")) {
                    qCDebug(KCALCORE_LOG) << "Generating compat for KOrganizer 3.2 pre";
                    compat = new Compat32PrereleaseVersions;
                } else if (versionNum < 30400) {
                    compat = new CompatPre34;
                } else if (versionNum < 30500) {
                    compat = new CompatPre35;
                }
            }
        }
    } else if (outl9 >= 0) {
        qCDebug(KCALCORE_LOG) << "Generating compat for Outlook < 2000 (Outlook 9.0)";
        compat = new CompatOutlook9;
    }
    if (!compat) {
        compat = new Compat;
    }
    // Older implementations lacked the implementation version,
    // so apply this fix if it is a file from kontact and the version is missing.
    if (implementationVersion.isEmpty()
        && (productId.contains(QLatin1String("libkcal")) || productId.contains(QLatin1String("KOrganizer")) || productId.contains(QLatin1String("KAlarm")))) {
        compat = new CompatPre410(compat);
    }

    return compat;
}

Compat::~Compat() = default;

void Compat::fixEmptySummary(const Incidence::Ptr &incidence)
{
    // some stupid vCal exporters ignore the standard and use Description
    // instead of Summary for the default field. Correct for this: Copy the
    // first line of the description to the summary (if summary is just one
    // line, move it)
    if (incidence->summary().isEmpty() && !(incidence->description().isEmpty())) {
        QString oldDescription = incidence->description().trimmed();
        QString newSummary(oldDescription);
        newSummary.remove(QRegularExpression(QStringLiteral("\n.*")));
        incidence->setSummary(newSummary);
        if (oldDescription == newSummary) {
            incidence->setDescription(QLatin1String(""));
        }
    }
}

void Compat::fixAlarms(const Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence);
}

void Compat::fixFloatingEnd(QDate &date)
{
    Q_UNUSED(date);
}

void Compat::fixRecurrence(const Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence);
    // Prevent use of compatibility mode during subsequent changes by the application
    // incidence->recurrence()->setCompatVersion();
}

int Compat::fixPriority(int priority)
{
    return priority;
}

bool Compat::useTimeZoneShift() const
{
    return true;
}

void Compat::setCreatedToDtStamp(const Incidence::Ptr &incidence, const QDateTime &dtstamp)
{
    Q_UNUSED(incidence);
    Q_UNUSED(dtstamp);
}

CompatDecorator::CompatDecorator(Compat *compat)
    : m_compat(compat)
{
}

CompatDecorator::~CompatDecorator() = default;

void CompatDecorator::fixEmptySummary(const Incidence::Ptr &incidence)
{
    m_compat->fixEmptySummary(incidence);
}

void CompatDecorator::fixAlarms(const Incidence::Ptr &incidence)
{
    m_compat->fixAlarms(incidence);
}

void CompatDecorator::fixFloatingEnd(QDate &date)
{
    m_compat->fixFloatingEnd(date);
}

void CompatDecorator::fixRecurrence(const Incidence::Ptr &incidence)
{
    m_compat->fixRecurrence(incidence);
}

int CompatDecorator::fixPriority(int priority)
{
    return m_compat->fixPriority(priority);
}

bool CompatDecorator::useTimeZoneShift() const
{
    return m_compat->useTimeZoneShift();
}

void CompatDecorator::setCreatedToDtStamp(const Incidence::Ptr &incidence, const QDateTime &dtstamp)
{
    m_compat->setCreatedToDtStamp(incidence, dtstamp);
}

void CompatPre35::fixRecurrence(const Incidence::Ptr &incidence)
{
    Recurrence *recurrence = incidence->recurrence();
    if (recurrence) {
        QDateTime start(incidence->dtStart());
        // kde < 3.5 only had one rrule, so no need to loop over all RRULEs.
        RecurrenceRule *r = recurrence->defaultRRule();
        if (r && !r->dateMatchesRules(start)) {
            recurrence->addExDateTime(start);
        }
    }

    // Call base class method now that everything else is done
    Compat::fixRecurrence(incidence);
}

int CompatPre34::fixPriority(int priority)
{
    if (0 < priority && priority < 6) {
        // adjust 1->1, 2->3, 3->5, 4->7, 5->9
        return 2 * priority - 1;
    } else {
        return priority;
    }
}

void CompatPre32::fixRecurrence(const Incidence::Ptr &incidence)
{
    Recurrence *recurrence = incidence->recurrence();
    if (recurrence->recurs() && recurrence->duration() > 0) {
        recurrence->setDuration(recurrence->duration() + incidence->recurrence()->exDates().count());
    }
    // Call base class method now that everything else is done
    CompatPre35::fixRecurrence(incidence);
}

void CompatPre31::fixFloatingEnd(QDate &endDate)
{
    endDate = endDate.addDays(1);
}

void CompatPre31::fixRecurrence(const Incidence::Ptr &incidence)
{
    CompatPre32::fixRecurrence(incidence);

    Recurrence *recur = incidence->recurrence();
    RecurrenceRule *r = nullptr;
    if (recur) {
        r = recur->defaultRRule();
    }
    if (recur && r) {
        int duration = r->duration();
        if (duration > 0) {
            // Backwards compatibility for KDE < 3.1.
            // rDuration was set to the number of time periods to recur,
            // with week start always on a Monday.
            // Convert this to the number of occurrences.
            r->setDuration(-1);
            QDate end(r->startDt().date());
            bool doNothing = false;
            // # of periods:
            int tmp = (duration - 1) * r->frequency();
            switch (r->recurrenceType()) {
            case RecurrenceRule::rWeekly: {
                end = end.addDays(tmp * 7 + 7 - end.dayOfWeek());
                break;
            }
            case RecurrenceRule::rMonthly: {
                int month = end.month() - 1 + tmp;
                end.setDate(end.year() + month / 12, month % 12 + 1, 31);
                break;
            }
            case RecurrenceRule::rYearly: {
                end.setDate(end.year() + tmp, 12, 31);
                break;
            }
            default:
                doNothing = true;
                break;
            }
            if (!doNothing) {
                duration = r->durationTo(QDateTime(end, QTime(0, 0, 0), incidence->dtStart().timeZone()));
                r->setDuration(duration);
            }
        }

        /* addYearlyNum */
        // Dates were stored as day numbers, with a fiddle to take account of
        // leap years. Convert the day number to a month.
        QList<int> days = r->byYearDays();
        if (!days.isEmpty()) {
            QList<int> months = r->byMonths();
            for (int i = 0; i < months.size(); ++i) {
                int newmonth = QDate(r->startDt().date().year(), 1, 1).addDays(months.at(i) - 1).month();
                if (!months.contains(newmonth)) {
                    months.append(newmonth);
                }
            }

            r->setByMonths(months);
            days.clear();
            r->setByYearDays(days);
        }
    }
}

void CompatOutlook9::fixAlarms(const Incidence::Ptr &incidence)
{
    if (!incidence) {
        return;
    }

    const Alarm::List alarms = incidence->alarms();
    for (const Alarm::Ptr &al : alarms) {
        if (al && al->hasStartOffset()) {
            Duration offsetDuration = al->startOffset();
            int offs = offsetDuration.asSeconds();
            if (offs > 0) {
                offsetDuration = Duration(-offs);
            }
            al->setStartOffset(offsetDuration);
        }
    }
}

bool Compat32PrereleaseVersions::useTimeZoneShift() const
{
    return false;
}

CompatPre410::CompatPre410(Compat *decoratedCompat)
    : CompatDecorator(decoratedCompat)
{
}

void CompatPre410::setCreatedToDtStamp(const Incidence::Ptr &incidence, const QDateTime &dtstamp)
{
    if (dtstamp.isValid()) {
        incidence->setCreated(dtstamp);
    }
}
