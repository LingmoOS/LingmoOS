/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "indiancalendar.h"

class IndianCalendarProviderPrivate : public ICUCalendarPrivate
{
public:
    explicit IndianCalendarProviderPrivate();

    /**
     * For formatting, see the documentation of SimpleDateFormat:
     * https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1SimpleDateFormat.html#details
     */
    QString formattedDateStringInNativeLanguage(const icu::UnicodeString &str) const;

    QCalendar::YearMonthDay fromGregorian(const QDate &_date);
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date);

private:
    // Translated month names are available in https://github.com/unicode-org/icu/tree/main/icu4c/source/data/locales
    icu::Locale m_nativeLocale;
};

IndianCalendarProviderPrivate::IndianCalendarProviderPrivate()
    : ICUCalendarPrivate()
    , m_nativeLocale(icu::Locale(QLocale::system().name().toLatin1(), 0, 0, "calendar=indian;"))
{
    if (U_FAILURE(m_errorCode)) {
        return; // Failed to create m_GregorianCalendar
    }

    m_calendar.reset(icu::Calendar::createInstance("en_US@calendar=indian", m_errorCode));
}

QString IndianCalendarProviderPrivate::formattedDateStringInNativeLanguage(const icu::UnicodeString &str) const
{
    UErrorCode errorCode = U_ZERO_ERROR;
    icu::UnicodeString dateString;
    icu::SimpleDateFormat formatter(str, m_nativeLocale, errorCode);
    formatter.setCalendar(*m_calendar);
    formatter.format(m_calendar->getTime(errorCode), dateString);

    return QStringView(dateString.getBuffer(), dateString.length()).toString();
}

QCalendar::YearMonthDay IndianCalendarProviderPrivate::fromGregorian(const QDate &_date)
{
    if (U_FAILURE(m_errorCode) || !_date.isValid() || !setDate(_date)) {
        return {};
    }

    return date();
}

CalendarEvents::CalendarEventsPlugin::SubLabel IndianCalendarProviderPrivate::subLabel(const QDate &date)
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (U_FAILURE(m_errorCode) || !date.isValid() || !setDate(date)) {
        return sublabel;
    }

    sublabel.dayLabel = QString::number(day());
    sublabel.label = i18ndc("plasma_calendar_alternatecalendar",
                            "@label %1 day %2 month name in India National Calendar %3 year",
                            "%1 %2, %3",
                            sublabel.dayLabel,
                            formattedDateStringInNativeLanguage("MMMM"),
                            QString::number(year()));
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    return sublabel;
}

IndianCalendarProvider::IndianCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate)
    : AbstractCalendarProvider(parent, calendarSystem, startDate, endDate)
    , d(std::make_unique<IndianCalendarProviderPrivate>())
{
    Q_ASSERT(m_calendarSystem == CalendarSystem::Indian);
}

IndianCalendarProvider::~IndianCalendarProvider()
{
}

QCalendar::YearMonthDay IndianCalendarProvider::fromGregorian(const QDate &date) const
{
    return d->fromGregorian(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel IndianCalendarProvider::subLabel(const QDate &date) const
{
    return d->subLabel(date);
}
