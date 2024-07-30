/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "hebrewcalendar.h"

#include "icucalendar_p.h"

class HebrewCalendarProviderPrivate : public ICUCalendarPrivate
{
public:
    explicit HebrewCalendarProviderPrivate();

    /**
     * For formatting, see the documentation of SimpleDateFormat:
     * https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1SimpleDateFormat.html#details
     */
    QString formattedDateString(const icu::UnicodeString &str) const;
    QString formattedDateStringInNativeLanguage(const icu::UnicodeString &str) const;

    QCalendar::YearMonthDay fromGregorian(const QDate &_date);
    CalendarEvents::CalendarEventsPlugin::SubLabel subLabel(const QDate &date);

private:
    // See https://unicode-org.github.io/icu/userguide/locale/#keywords for available keywords
    icu::Locale m_hebrewLocale;
    icu::Locale m_nativeLocale;
};

HebrewCalendarProviderPrivate::HebrewCalendarProviderPrivate()
    : ICUCalendarPrivate()
    , m_hebrewLocale(icu::Locale("he_IL", 0, 0, "calendar=hebrew;numbers=hebr"))
    , m_nativeLocale(icu::Locale(QLocale::system().name().toLatin1(), 0, 0, "calendar=hebrew;numbers=hebr"))
{
    if (U_FAILURE(m_errorCode)) {
        return; // Failed to create m_GregorianCalendar
    }

    m_calendar.reset(icu::Calendar::createInstance(icu::Locale("he_IL@calendar=hebrew"), m_errorCode));
}

QString HebrewCalendarProviderPrivate::formattedDateString(const icu::UnicodeString &str) const
{
    UErrorCode errorCode = U_ZERO_ERROR;
    icu::UnicodeString dateString;
    icu::SimpleDateFormat formatter(str, m_hebrewLocale, errorCode);
    formatter.setCalendar(*m_calendar);
    formatter.format(m_calendar->getTime(errorCode), dateString);

    return QStringView(dateString.getBuffer(), dateString.length()).toString();
}

QString HebrewCalendarProviderPrivate::formattedDateStringInNativeLanguage(const icu::UnicodeString &str) const
{
    UErrorCode errorCode = U_ZERO_ERROR;
    icu::UnicodeString dateString;
    icu::SimpleDateFormat formatter(str, m_nativeLocale, errorCode);
    formatter.setCalendar(*m_calendar);
    formatter.format(m_calendar->getTime(errorCode), dateString);

    return QStringView(dateString.getBuffer(), dateString.length()).toString();
}

QCalendar::YearMonthDay HebrewCalendarProviderPrivate::fromGregorian(const QDate &_date)
{
    if (U_FAILURE(m_errorCode) || !_date.isValid() || !setDate(_date)) {
        return {};
    }

    return date();
}

CalendarEvents::CalendarEventsPlugin::SubLabel HebrewCalendarProviderPrivate::subLabel(const QDate &date)
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (U_FAILURE(m_errorCode) || !date.isValid() || !setDate(date)) {
        return sublabel;
    }

    const bool isLocaleHebrew = QLocale::system().language() == QLocale::Hebrew;

    sublabel.dayLabel = isLocaleHebrew ? formattedDateString("d") : QString::number(day());
    const QString hebrewDateString = formattedDateString("d בMMMM y"); // See https://unicode-org.github.io/cldr/ldml/tr35-dates.html
    // Translated month names are available in https://github.com/unicode-org/icu/tree/main/icu4c/source/data/locales
    sublabel.label = isLocaleHebrew ? hebrewDateString
                                    : i18ndc("plasma_calendar_alternatecalendar",
                                             "%1 Day number %2 Translated month name in Hebrew/Jewish calendar %3 Year number %4 Full date in Hebrew",
                                             "%1 %2, %3 (%4)",
                                             QString::number(day()),
                                             formattedDateStringInNativeLanguage("MMMM"),
                                             QString::number(year()),
                                             hebrewDateString);
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    return sublabel;
}

HebrewCalendarProvider::HebrewCalendarProvider(QObject *parent, CalendarSystem::System calendarSystem, const QDate &startDate, const QDate &endDate)
    : AbstractCalendarProvider(parent, calendarSystem, startDate, endDate)
    , d(std::make_unique<HebrewCalendarProviderPrivate>())
{
    Q_ASSERT(m_calendarSystem == CalendarSystem::Hebrew);
}

HebrewCalendarProvider::~HebrewCalendarProvider()
{
}

QCalendar::YearMonthDay HebrewCalendarProvider::fromGregorian(const QDate &date) const
{
    return d->fromGregorian(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel HebrewCalendarProvider::subLabel(const QDate &date) const
{
    return d->subLabel(date);
}
