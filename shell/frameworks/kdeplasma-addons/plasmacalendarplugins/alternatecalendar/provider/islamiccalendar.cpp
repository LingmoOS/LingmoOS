/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "islamiccalendar.h"

#include "icucalendar_p.h"
#include <unicode/numsys.h>

class IslamicCalendarProviderPrivate : public ICUCalendarPrivate
{
public:
    explicit IslamicCalendarProviderPrivate(CalendarSystem::System calendarSystem);

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
    icu::Locale m_arabicLocale;
    icu::Locale m_nativeLocale;
    bool m_isArabicNumberingSystem = false;
};

IslamicCalendarProviderPrivate::IslamicCalendarProviderPrivate(CalendarSystem::System calendarSystem)
    : ICUCalendarPrivate()
{
    if (U_FAILURE(m_errorCode)) {
        return; // Failed to create m_GregorianCalendar
    }

    // See https://github.com/unicode-org/cldr/blob/main/common/bcp47/number.xml for available number systems
    // See https://cldr.unicode.org/development/development-process/design-proposals/islamic-calendar-types for available Islamic caneldar types
    const QByteArray iso639Language = QLocale::languageToCode(QLocale::system().language()).toLatin1().toLower();
    const QByteArray iso3166Country = QLocale::territoryToCode(QLocale::system().territory()).toLatin1().toUpper();
    const char *keywords = nullptr;
    QLocale::Language typicalLanguage = QLocale::Arabic;
    QLocale::Territory typicalTerritory = QLocale::SaudiArabia;
    switch (calendarSystem) {
    case CalendarSystem::Jalali: {
        keywords = "calendar=persian;";
        typicalLanguage = QLocale::Persian;
        typicalTerritory = QLocale::Iran;
        break;
    }
    case CalendarSystem::Islamic: {
        keywords = "calendar=islamic;";
        break;
    }
    case CalendarSystem::IslamicCivil: {
        keywords = "calendar=islamic-civil;";
        break;
    }
    case CalendarSystem::IslamicUmalqura: {
        keywords = "calendar=islamic-umalqura;";
        break;
    }
    default:
        Q_UNREACHABLE();
    }

    m_nativeLocale = icu::Locale(iso639Language, iso3166Country, nullptr, keywords);
    std::unique_ptr<icu::NumberingSystem> numberingSystem(icu::NumberingSystem::createInstance(m_nativeLocale, m_errorCode));
    m_isArabicNumberingSystem = U_SUCCESS(m_errorCode) && QByteArrayView(numberingSystem->getName()).startsWith(QByteArrayView("arab"));
    if (!m_isArabicNumberingSystem) {
        m_arabicLocale = icu::Locale(QLocale::languageToCode(typicalLanguage).toLatin1().toLower(),
                                     QLocale::territoryToCode(typicalTerritory).toLatin1().toUpper(),
                                     nullptr,
                                     keywords);
    }

    m_calendar.reset(icu::Calendar::createInstance(m_nativeLocale, m_errorCode));
}

QString IslamicCalendarProviderPrivate::formattedDateString(const icu::UnicodeString &str) const
{
    Q_ASSERT(!m_isArabicNumberingSystem);
    UErrorCode errorCode = U_ZERO_ERROR;
    icu::UnicodeString dateString;
    icu::SimpleDateFormat formatter(str, m_arabicLocale, errorCode);
    formatter.setCalendar(*m_calendar);
    formatter.format(m_calendar->getTime(errorCode), dateString);

    return QStringView(dateString.getBuffer(), dateString.length()).toString();
}

QString IslamicCalendarProviderPrivate::formattedDateStringInNativeLanguage(const icu::UnicodeString &str) const
{
    UErrorCode errorCode = U_ZERO_ERROR;
    icu::UnicodeString dateString;
    icu::SimpleDateFormat formatter(str, m_nativeLocale, errorCode);
    formatter.setCalendar(*m_calendar);
    formatter.format(m_calendar->getTime(errorCode), dateString);

    return QStringView(dateString.getBuffer(), dateString.length()).toString();
}

QCalendar::YearMonthDay IslamicCalendarProviderPrivate::fromGregorian(const QDate &_date)
{
    if (U_FAILURE(m_errorCode) || !_date.isValid() || !setDate(_date)) {
        return {};
    }

    return date();
}

CalendarEvents::CalendarEventsPlugin::SubLabel IslamicCalendarProviderPrivate::subLabel(const QDate &date)
{
    auto sublabel = CalendarEvents::CalendarEventsPlugin::SubLabel{};

    if (U_FAILURE(m_errorCode) || !date.isValid() || !setDate(date)) {
        return sublabel;
    }

    sublabel.dayLabel = formattedDateStringInNativeLanguage("d");
    // Translated month names are available in https://github.com/unicode-org/icu/tree/main/icu4c/source/data/locales
    sublabel.label = m_isArabicNumberingSystem
        ? formattedDateStringInNativeLanguage("d MMMM yyyy")
        : i18ndc("plasma_calendar_alternatecalendar",
                 "@label %1 Day number %2 Month name in Islamic Calendar %3 Year number %4 Islamic calendar date in Arabic",
                 "%1 %2, %3 (%4)",
                 QString::number(day()),
                 formattedDateStringInNativeLanguage("MMMM"),
                 QString::number(year()),
                 formattedDateString("d MMMM yyyy"));
    sublabel.priority = CalendarEvents::CalendarEventsPlugin::SubLabelPriority::Low;

    return sublabel;
}

IslamicCalendarProvider::IslamicCalendarProvider(QObject *parent,
                                                 CalendarSystem::System calendarSystem,
                                                 const QDate &startDate,
                                                 const QDate &endDate,
                                                 int dateOffset)
    : AbstractCalendarProvider(parent, calendarSystem, startDate, endDate, dateOffset)
    , d(std::make_unique<IslamicCalendarProviderPrivate>(calendarSystem))
{
    Q_ASSERT(m_calendarSystem == CalendarSystem::Jalali || m_calendarSystem == CalendarSystem::Islamic || m_calendarSystem == CalendarSystem::IslamicCivil
             || m_calendarSystem == CalendarSystem::IslamicUmalqura);
}

IslamicCalendarProvider::~IslamicCalendarProvider()
{
}

QCalendar::YearMonthDay IslamicCalendarProvider::fromGregorian(const QDate &date) const
{
    return d->fromGregorian(date);
}

CalendarEvents::CalendarEventsPlugin::SubLabel IslamicCalendarProvider::subLabel(const QDate &date) const
{
    return d->subLabel(date);
}
