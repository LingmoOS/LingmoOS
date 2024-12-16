/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the ICalFormat class.

  @brief
  iCalendar format implementation: a layer of abstraction for libical.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/
#include "icalformat.h"
#include "calendar_p.h"
#include "icalformat_p.h"
#include "icaltimezones_p.h"
#include "memorycalendar.h"

#include <QFile>
#include <QSaveFile>
#include <QTimeZone>
#include <QDebug>

extern "C" {
#include "libical/ical.h"
#include "libical/icalmemory.h"
#include "libical/icalparser.h"
#include "libical/icalrestriction.h"
#include "libical/icalss.h"
}

using namespace KCalendarCore;

//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::ICalFormat::Private
{
public:
    Private(ICalFormat *parent)
        : mImpl(new ICalFormatImpl(parent))
        , mTimeZone(QTimeZone::utc())
    {
    }
    ~Private()
    {
        delete mImpl;
    }
    ICalFormatImpl *mImpl = nullptr;
    QTimeZone mTimeZone;
};
//@endcond

ICalFormat::ICalFormat()
    : d(new Private(this))
{
}

ICalFormat::~ICalFormat()
{
    icalmemory_free_ring();
    delete d;
}

bool ICalFormat::load(const Calendar::Ptr &calendar, const QString &fileName)
{
    qDebug() << fileName;

    clearException();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "load error: unable to open " << fileName;
        setException(new Exception(Exception::LoadError));
        return false;
    }
    const QByteArray text = file.readAll().trimmed();
    file.close();

    if (!text.isEmpty()) {
        if (!fromRawString(calendar, text, false, fileName)) {
            qWarning() << fileName << " is not a valid iCalendar file";
            setException(new Exception(Exception::ParseErrorIcal));
            return false;
        }
    }

    // Note: we consider empty files to be valid

    return true;
}

bool ICalFormat::save(const Calendar::Ptr &calendar, const QString &fileName)
{
    qDebug() << fileName;

    clearException();

    QString text = toString(calendar);
    if (text.isEmpty()) {
        return false;
    }

    // Write backup file
    const QString backupFile = fileName + QLatin1Char('~');
    QFile::remove(backupFile);
    QFile::copy(fileName, backupFile);

    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "file open error: " << file.errorString() << ";filename=" << fileName;
        setException(new Exception(Exception::SaveErrorOpenFile, QStringList(fileName)));

        return false;
    }

    // Convert to UTF8 and save
    QByteArray textUtf8 = text.toUtf8();
    file.write(textUtf8.data(), textUtf8.size());
    // QSaveFile doesn't report a write error when the device is full (see Qt
    // bug 75077), so check that the data can actually be written.
    if (!file.flush()) {
        qDebug() << "file write error (flush failed)";
        setException(new Exception(Exception::SaveErrorSaveFile, QStringList(fileName)));
        return false;
    }

    if (!file.commit()) {
        qDebug() << "file finalize error:" << file.errorString();
        setException(new Exception(Exception::SaveErrorSaveFile, QStringList(fileName)));

        return false;
    }

    return true;
}

bool ICalFormat::fromString(const Calendar::Ptr &cal, const QString &string, bool deleted, const QString &notebook)
{
    return fromRawString(cal, string.toUtf8(), deleted, notebook);
}

Incidence::Ptr ICalFormat::readIncidence(const QByteArray &string)
{
    // Let's defend const correctness until the very gates of hell^Wlibical
    icalcomponent *calendar = icalcomponent_new_from_string(const_cast<char *>(string.constData()));
    if (!calendar) {
        qCritical() << "parse error from icalcomponent_new_from_string. string=" << QString::fromLatin1(string);
        setException(new Exception(Exception::ParseErrorIcal));
        return Incidence::Ptr();
    }

    ICalTimeZoneCache tzCache;
    ICalTimeZoneParser parser(&tzCache);
    parser.parse(calendar);

    Incidence::Ptr incidence;
    if (icalcomponent_isa(calendar) == ICAL_VCALENDAR_COMPONENT) {
        incidence = d->mImpl->readOneIncidence(calendar, &tzCache);
    } else if (icalcomponent_isa(calendar) == ICAL_XROOT_COMPONENT) {
        icalcomponent *comp = icalcomponent_get_first_component(calendar, ICAL_VCALENDAR_COMPONENT);
        if (comp) {
            incidence = d->mImpl->readOneIncidence(comp, &tzCache);
        }
    }

    if (!incidence) {
        qDebug() << "No VCALENDAR component found";
        setException(new Exception(Exception::NoCalendar));
    }

    icalcomponent_free(calendar);
    icalmemory_free_ring();

    return incidence;
}

bool ICalFormat::fromRawString(const Calendar::Ptr &cal, const QByteArray &string, bool deleted, const QString &notebook)
{
    Q_UNUSED(notebook);
    // Get first VCALENDAR component.
    // TODO: Handle more than one VCALENDAR or non-VCALENDAR top components
    icalcomponent *calendar;

    // Let's defend const correctness until the very gates of hell^Wlibical
    calendar = icalcomponent_new_from_string(const_cast<char *>(string.constData()));
    if (!calendar) {
        qCritical() << "parse error from icalcomponent_new_from_string. string=" << QString::fromLatin1(string);
        setException(new Exception(Exception::ParseErrorIcal));
        return false;
    }

    bool success = true;

    if (icalcomponent_isa(calendar) == ICAL_XROOT_COMPONENT) {
        icalcomponent *comp;
        for (comp = icalcomponent_get_first_component(calendar, ICAL_VCALENDAR_COMPONENT); comp;
             comp = icalcomponent_get_next_component(calendar, ICAL_VCALENDAR_COMPONENT)) {
            // put all objects into their proper places
            if (!d->mImpl->populate(cal, comp, deleted)) {
                qCritical() << "Could not populate calendar";
                if (!exception()) {
                    setException(new Exception(Exception::ParseErrorKcal));
                }
                success = false;
            } else {
                setLoadedProductId(d->mImpl->loadedProductId());
            }
        }
    } else if (icalcomponent_isa(calendar) != ICAL_VCALENDAR_COMPONENT) {
        qDebug() << "No VCALENDAR component found";
        setException(new Exception(Exception::NoCalendar));
        success = false;
    } else {
        // put all objects into their proper places
        if (!d->mImpl->populate(cal, calendar, deleted)) {
            qDebug() << "Could not populate calendar";
            if (!exception()) {
                setException(new Exception(Exception::ParseErrorKcal));
            }
            success = false;
        } else {
            setLoadedProductId(d->mImpl->loadedProductId());
        }
    }

    icalcomponent_free(calendar);
    icalmemory_free_ring();

    return success;
}

Incidence::Ptr ICalFormat::fromString(const QString &string)
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(d->mTimeZone));
    fromString(cal, string);

    const Incidence::List list = cal->incidences();
    return !list.isEmpty() ? list.first() : Incidence::Ptr();
}

QString ICalFormat::toString(const Calendar::Ptr &cal, const QString &notebook, bool deleted)
{
    icalcomponent *calendar = d->mImpl->createCalendarComponent(cal);
    icalcomponent *component;

    QVector<QTimeZone> tzUsedList;
    TimeZoneEarliestDate earliestTz;

    // todos
    Todo::List todoList = deleted ? cal->deletedTodos() : cal->rawTodos();
    for (auto it = todoList.cbegin(), end = todoList.cend(); it != end; ++it) {
        if (!deleted || !cal->todo((*it)->uid(), (*it)->recurrenceId())) {
            // use existing ones, or really deleted ones
            if (notebook.isEmpty() || (!cal->notebook(*it).isEmpty() && notebook.endsWith(cal->notebook(*it)))) {
                component = d->mImpl->writeTodo(*it, &tzUsedList);
                icalcomponent_add_component(calendar, component);
                ICalTimeZoneParser::updateTzEarliestDate((*it), &earliestTz);
            }
        }
    }
    // events
    Event::List events = deleted ? cal->deletedEvents() : cal->rawEvents();
    for (auto it = events.cbegin(), end = events.cend(); it != end; ++it) {
        if (!deleted || !cal->event((*it)->uid(), (*it)->recurrenceId())) {
            // use existing ones, or really deleted ones
            if (notebook.isEmpty() || (!cal->notebook(*it).isEmpty() && notebook.endsWith(cal->notebook(*it)))) {
                component = d->mImpl->writeEvent(*it, &tzUsedList);
                icalcomponent_add_component(calendar, component);
                ICalTimeZoneParser::updateTzEarliestDate((*it), &earliestTz);
            }
        }
    }

    // journals
    Journal::List journals = deleted ? cal->deletedJournals() : cal->rawJournals();
    for (auto it = journals.cbegin(), end = journals.cend(); it != end; ++it) {
        if (!deleted || !cal->journal((*it)->uid(), (*it)->recurrenceId())) {
            // use existing ones, or really deleted ones
            if (notebook.isEmpty() || (!cal->notebook(*it).isEmpty() && notebook.endsWith(cal->notebook(*it)))) {
                component = d->mImpl->writeJournal(*it, &tzUsedList);
                icalcomponent_add_component(calendar, component);
                ICalTimeZoneParser::updateTzEarliestDate((*it), &earliestTz);
            }
        }
    }

    // time zones
    if (todoList.isEmpty() && events.isEmpty() && journals.isEmpty()) {
        // no incidences means no used timezones, use all timezones
        // this will export a calendar having only timezone definitions
        tzUsedList = cal->d->mTimeZones;
    }
    for (const auto &qtz : qAsConst(tzUsedList)) {
        if (qtz != QTimeZone::utc()) {
            icaltimezone *tz = ICalTimeZoneParser::icaltimezoneFromQTimeZone(qtz, earliestTz[qtz]);
            if (!tz) {
                qCritical() << "bad time zone";
            } else {
                component = icalcomponent_new_clone(icaltimezone_get_component(tz));
                icalcomponent_add_component(calendar, component);
                icaltimezone_free(tz, 1);
            }
        }
    }

    char *const componentString = icalcomponent_as_ical_string_r(calendar);
    const QString &text = QString::fromUtf8(componentString);
    free(componentString);

    icalcomponent_free(calendar);
    icalmemory_free_ring();

    if (text.isEmpty()) {
        setException(new Exception(Exception::LibICalError));
    }

    return text;
}

QString ICalFormat::toICalString(const Incidence::Ptr &incidence)
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(d->mTimeZone));
    cal->addIncidence(Incidence::Ptr(incidence->clone()));
    return toString(cal.staticCast<Calendar>());
}

QString ICalFormat::toString(const Incidence::Ptr &incidence)
{
    return QString::fromUtf8(toRawString(incidence));
}

QByteArray ICalFormat::toRawString(const Incidence::Ptr &incidence)
{
    TimeZoneList tzUsedList;

    icalcomponent *component = d->mImpl->writeIncidence(incidence, iTIPRequest, &tzUsedList);

    QByteArray text = icalcomponent_as_ical_string(component);

    TimeZoneEarliestDate earliestTzDt;
    ICalTimeZoneParser::updateTzEarliestDate(incidence, &earliestTzDt);

    // time zones
    for (const auto &qtz : qAsConst(tzUsedList)) {
        if (qtz != QTimeZone::utc()) {
            icaltimezone *tz = ICalTimeZoneParser::icaltimezoneFromQTimeZone(qtz, earliestTzDt[qtz]);
            if (!tz) {
                qCritical() << "bad time zone";
            } else {
                icalcomponent *tzcomponent = icaltimezone_get_component(tz);
                icalcomponent_add_component(component, component);
                text.append(icalcomponent_as_ical_string(tzcomponent));
                icaltimezone_free(tz, 1);
            }
        }
    }

    icalcomponent_free(component);

    return text;
}

QString ICalFormat::toString(RecurrenceRule *recurrence)
{
    icalproperty *property = icalproperty_new_rrule(d->mImpl->writeRecurrenceRule(recurrence));
    QString text = QString::fromUtf8(icalproperty_as_ical_string(property));
    icalproperty_free(property);
    return text;
}

bool ICalFormat::fromString(RecurrenceRule *recurrence, const QString &rrule)
{
    if (!recurrence) {
        return false;
    }
    bool success = true;
    icalerror_clear_errno();
    struct icalrecurrencetype recur = icalrecurrencetype_from_string(rrule.toLatin1().constData());
    if (icalerrno != ICAL_NO_ERROR) {
        qDebug() << "Recurrence parsing error:" << icalerror_strerror(icalerrno);
        success = false;
    }

    if (success) {
        d->mImpl->readRecurrence(recur, recurrence);
    }

    return success;
}

QString ICalFormat::createScheduleMessage(const IncidenceBase::Ptr &incidence, iTIPMethod method)
{
    icalcomponent *message = nullptr;

    if (incidence->type() == Incidence::TypeEvent || incidence->type() == Incidence::TypeTodo) {
        Incidence::Ptr i = incidence.staticCast<Incidence>();

        // Recurring events need timezone information to allow proper calculations
        // across timezones with different DST.
        const bool useUtcTimes = !i->recurs();

        const bool hasSchedulingId = (i->schedulingID() != i->uid());

        const bool incidenceNeedChanges = (useUtcTimes || hasSchedulingId);

        if (incidenceNeedChanges) {
            // The incidence need changes, so clone it before we continue
            i = Incidence::Ptr(i->clone());

            // Handle conversion to UTC times
            if (useUtcTimes) {
                i->shiftTimes(QTimeZone::utc(), QTimeZone::utc());
            }

            // Handle scheduling ID being present
            if (hasSchedulingId) {
                // We have a separation of scheduling ID and UID
                i->setSchedulingID(QString(), i->schedulingID());
            }

            // Build the message with the cloned incidence
            message = d->mImpl->createScheduleComponent(i, method);
        }
    }

    if (message == nullptr) {
        message = d->mImpl->createScheduleComponent(incidence, method);
    }

    QString messageText = QString::fromUtf8(icalcomponent_as_ical_string(message));

    icalcomponent_free(message);
    return messageText;
}

FreeBusy::Ptr ICalFormat::parseFreeBusy(const QString &str)
{
    clearException();

    icalcomponent *message = icalparser_parse_string(str.toUtf8().constData());

    if (!message) {
        return FreeBusy::Ptr();
    }

    FreeBusy::Ptr freeBusy;

    icalcomponent *c = nullptr;
    for (c = icalcomponent_get_first_component(message, ICAL_VFREEBUSY_COMPONENT); c != nullptr;
         c = icalcomponent_get_next_component(message, ICAL_VFREEBUSY_COMPONENT)) {
        FreeBusy::Ptr fb = d->mImpl->readFreeBusy(c);

        if (freeBusy) {
            freeBusy->merge(fb);
        } else {
            freeBusy = fb;
        }
    }

    if (!freeBusy) {
        qDebug() << "object is not a freebusy.";
    }

    icalcomponent_free(message);

    return freeBusy;
}

ScheduleMessage::Ptr ICalFormat::parseScheduleMessage(const Calendar::Ptr &cal, const QString &messageText)
{
    setTimeZone(cal->timeZone());
    clearException();

    if (messageText.isEmpty()) {
        setException(new Exception(Exception::ParseErrorEmptyMessage));
        return ScheduleMessage::Ptr();
    }

    icalcomponent *message = icalparser_parse_string(messageText.toUtf8().constData());

    if (!message) {
        setException(new Exception(Exception::ParseErrorUnableToParse));

        return ScheduleMessage::Ptr();
    }

    icalproperty *m = icalcomponent_get_first_property(message, ICAL_METHOD_PROPERTY);
    if (!m) {
        setException(new Exception(Exception::ParseErrorMethodProperty));

        return ScheduleMessage::Ptr();
    }

    // Populate the message's time zone collection with all VTIMEZONE components
    ICalTimeZoneCache tzlist;
    ICalTimeZoneParser parser(&tzlist);
    parser.parse(message);

    IncidenceBase::Ptr incidence;
    icalcomponent *c = icalcomponent_get_first_component(message, ICAL_VEVENT_COMPONENT);
    if (c) {
        incidence = d->mImpl->readEvent(c, &tzlist).staticCast<IncidenceBase>();
    }

    if (!incidence) {
        c = icalcomponent_get_first_component(message, ICAL_VTODO_COMPONENT);
        if (c) {
            incidence = d->mImpl->readTodo(c, &tzlist).staticCast<IncidenceBase>();
        }
    }

    if (!incidence) {
        c = icalcomponent_get_first_component(message, ICAL_VJOURNAL_COMPONENT);
        if (c) {
            incidence = d->mImpl->readJournal(c, &tzlist).staticCast<IncidenceBase>();
        }
    }

    if (!incidence) {
        c = icalcomponent_get_first_component(message, ICAL_VFREEBUSY_COMPONENT);
        if (c) {
            incidence = d->mImpl->readFreeBusy(c).staticCast<IncidenceBase>();
        }
    }

    if (!incidence) {
        qDebug() << "object is not a freebusy, event, todo or journal";
        setException(new Exception(Exception::ParseErrorNotIncidence));

        return ScheduleMessage::Ptr();
    }

    icalproperty_method icalmethod = icalproperty_get_method(m);
    iTIPMethod method;

    switch (icalmethod) {
    case ICAL_METHOD_PUBLISH:
        method = iTIPPublish;
        break;
    case ICAL_METHOD_REQUEST:
        method = iTIPRequest;
        break;
    case ICAL_METHOD_REFRESH:
        method = iTIPRefresh;
        break;
    case ICAL_METHOD_CANCEL:
        method = iTIPCancel;
        break;
    case ICAL_METHOD_ADD:
        method = iTIPAdd;
        break;
    case ICAL_METHOD_REPLY:
        method = iTIPReply;
        break;
    case ICAL_METHOD_COUNTER:
        method = iTIPCounter;
        break;
    case ICAL_METHOD_DECLINECOUNTER:
        method = iTIPDeclineCounter;
        break;
    default:
        method = iTIPNoMethod;
        qDebug() << "Unknown method";
        break;
    }

    if (!icalrestriction_check(message)) {
        qWarning() << "\nkcalcore library reported a problem while parsing:";
        qWarning() << ScheduleMessage::methodName(method) << ":" << d->mImpl->extractErrorProperty(c);
    }

    Incidence::Ptr existingIncidence = cal->incidence(incidence->uid());

    icalcomponent *calendarComponent = nullptr;
    if (existingIncidence) {
        calendarComponent = d->mImpl->createCalendarComponent(cal);

        // TODO: check, if cast is required, or if it can be done by virtual funcs.
        // TODO: Use a visitor for this!
        if (existingIncidence->type() == Incidence::TypeTodo) {
            Todo::Ptr todo = existingIncidence.staticCast<Todo>();
            icalcomponent_add_component(calendarComponent, d->mImpl->writeTodo(todo));
        }
        if (existingIncidence->type() == Incidence::TypeEvent) {
            Event::Ptr event = existingIncidence.staticCast<Event>();
            icalcomponent_add_component(calendarComponent, d->mImpl->writeEvent(event));
        }
    } else {
        icalcomponent_free(message);
        return ScheduleMessage::Ptr(new ScheduleMessage(incidence, method, ScheduleMessage::Unknown));
    }

    icalproperty_xlicclass result = icalclassify(message, calendarComponent, static_cast<const char *>(""));

    ScheduleMessage::Status status;

    switch (result) {
    case ICAL_XLICCLASS_PUBLISHNEW:
        status = ScheduleMessage::PublishNew;
        break;
    case ICAL_XLICCLASS_PUBLISHUPDATE:
        status = ScheduleMessage::PublishUpdate;
        break;
    case ICAL_XLICCLASS_OBSOLETE:
        status = ScheduleMessage::Obsolete;
        break;
    case ICAL_XLICCLASS_REQUESTNEW:
        status = ScheduleMessage::RequestNew;
        break;
    case ICAL_XLICCLASS_REQUESTUPDATE:
        status = ScheduleMessage::RequestUpdate;
        break;
    case ICAL_XLICCLASS_UNKNOWN:
    default:
        status = ScheduleMessage::Unknown;
        break;
    }

    icalcomponent_free(message);
    icalcomponent_free(calendarComponent);

    return ScheduleMessage::Ptr(new ScheduleMessage(incidence, method, status));
}

void ICalFormat::setTimeZone(const QTimeZone &timeZone)
{
    d->mTimeZone = timeZone;
}

QTimeZone ICalFormat::timeZone() const
{
    return d->mTimeZone;
}

QByteArray ICalFormat::timeZoneId() const
{
    return d->mTimeZone.id();
}

void ICalFormat::virtual_hook(int id, void *data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
    Q_ASSERT(false);
}
