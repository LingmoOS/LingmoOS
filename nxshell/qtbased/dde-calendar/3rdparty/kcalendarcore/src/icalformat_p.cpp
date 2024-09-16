/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2006 David Jarvie <djarvie@kde.org>
  SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the internal ICalFormat classes.

  @brief
  This class provides the libical dependent functions for ICalFormat.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
  @author David Jarvie \<djarvie@kde.org\>
*/

#include "icalformat_p.h"
#include "compat_p.h"
#include "icalformat.h"
#include "icaltimezones_p.h"
#include "incidencebase.h"
#include "memorycalendar.h"
#include "visitor.h"

#include <QFile>
#include <QDebug>
#include <QString>

using namespace KCalendarCore;

static const char APP_NAME_FOR_XPROPERTIES[] = "KCALCORE";
static const char ENABLED_ALARM_XPROPERTY[] = "ENABLED";
static const char IMPLEMENTATION_VERSION_XPROPERTY[] = "X-KDE-ICAL-IMPLEMENTATION-VERSION";
//EVENT农历日程标识
static const char VEVENT_LUNNAR_XPROPERTY[] = "X-DDE-ICAL-LUNNAR";

/* Static helpers */
/*
static void _dumpIcaltime( const icaltimetype& t)
{
  qDebug() << "--- Y:" << t.year << "M:" << t.month << "D:" << t.day;
  qDebug() << "--- H:" << t.hour << "M:" << t.minute << "S:" << t.second;
  qDebug() << "--- isUtc:" << icaltime_is_utc( t );
  qDebug() << "--- zoneId:" << icaltimezone_get_tzid( const_cast<icaltimezone*>( t.zone ) );
}
*/

//@cond PRIVATE
template<typename K>
void removeAllICal(QVector<QSharedPointer<K>> &c, const QSharedPointer<K> &x)
{
    if (c.count() < 1) {
        return;
    }

    int cnt = c.count(x);
    if (cnt != 1) {
        qCritical() << "There number of relatedTos for this incidence is " << cnt << " (there must be 1 relatedTo only)";
        Q_ASSERT_X(false, "removeAllICal", "Count is not 1.");
        return;
    }

    c.remove(c.indexOf(x));
}

const int gSecondsPerMinute = 60;
const int gSecondsPerHour = gSecondsPerMinute * 60;
const int gSecondsPerDay = gSecondsPerHour * 24;
const int gSecondsPerWeek = gSecondsPerDay * 7;

class ToComponentVisitor : public Visitor
{
public:
    ToComponentVisitor(ICalFormatImpl *impl, iTIPMethod m, TimeZoneList *tzUsedList = nullptr)
        : mImpl(impl)
        , mComponent(nullptr)
        , mMethod(m)
        , mTzUsedList(tzUsedList)
    {
    }

    ~ToComponentVisitor();

    bool visit(const Event::Ptr &e) override
    {
        mComponent = mImpl->writeEvent(e, mTzUsedList);
        return true;
    }
    bool visit(const Todo::Ptr &t) override
    {
        mComponent = mImpl->writeTodo(t, mTzUsedList);
        return true;
    }
    bool visit(const Journal::Ptr &j) override
    {
        mComponent = mImpl->writeJournal(j, mTzUsedList);
        return true;
    }
    bool visit(const FreeBusy::Ptr &fb) override
    {
        mComponent = mImpl->writeFreeBusy(fb, mMethod);
        return true;
    }

    icalcomponent *component()
    {
        return mComponent;
    }

private:
    ICalFormatImpl *mImpl = nullptr;
    icalcomponent *mComponent = nullptr;
    iTIPMethod mMethod;
    TimeZoneList *mTzUsedList = nullptr;
};

ToComponentVisitor::~ToComponentVisitor()
{
}

class Q_DECL_HIDDEN ICalFormatImpl::Private
{
public:
    Private(ICalFormatImpl *impl, ICalFormat *parent)
        : mImpl(impl)
        , mParent(parent)
        , mCompat(new Compat)
    {
    }
    ~Private()
    {
        delete mCompat;
    }
    void writeIncidenceBase(icalcomponent *parent, const IncidenceBase::Ptr &);
    void readIncidenceBase(icalcomponent *parent, const IncidenceBase::Ptr &);
    void writeCustomProperties(icalcomponent *parent, CustomProperties *);
    void readCustomProperties(icalcomponent *parent, CustomProperties *);

    ICalFormatImpl *mImpl = nullptr;
    ICalFormat *mParent = nullptr;
    QString mLoadedProductId; // PRODID string loaded from calendar file
    Event::List mEventsRelate; // events with relations
    Todo::List mTodosRelate; // todos with relations
    Compat *mCompat = nullptr;
};
//@endcond

inline icaltimetype ICalFormatImpl::writeICalUtcDateTime(const QDateTime &dt, bool dayOnly)
{
    return writeICalDateTime(dt.toUTC(), dayOnly);
}

ICalFormatImpl::ICalFormatImpl(ICalFormat *parent)
    : d(new Private(this, parent))
{
}

ICalFormatImpl::~ICalFormatImpl()
{
    delete d;
}

QString ICalFormatImpl::loadedProductId() const
{
    return d->mLoadedProductId;
}

icalcomponent *ICalFormatImpl::writeIncidence(const IncidenceBase::Ptr &incidence, iTIPMethod method, TimeZoneList *tzUsedList)
{
    ToComponentVisitor v(this, method, tzUsedList);
    if (incidence->accept(v, incidence)) {
        return v.component();
    } else {
        return nullptr;
    }
}

icalcomponent *ICalFormatImpl::writeTodo(const Todo::Ptr &todo, TimeZoneList *tzUsedList)
{
    icalcomponent *vtodo = icalcomponent_new(ICAL_VTODO_COMPONENT);

    writeIncidence(vtodo, todo.staticCast<Incidence>(), tzUsedList);

    // due date
    icalproperty *prop;
    if (todo->hasDueDate()) {
        icaltimetype due;
        if (todo->allDay()) {
            due = writeICalDate(todo->dtDue(true).date());
            prop = icalproperty_new_due(due);
        } else {
            prop = writeICalDateTimeProperty(ICAL_DUE_PROPERTY, todo->dtDue(true), tzUsedList);
        }
        icalcomponent_add_property(vtodo, prop);
    }

    // start time
    if (todo->hasStartDate()) {
        icaltimetype start;
        if (todo->allDay()) {
            start = writeICalDate(todo->dtStart(true).date());
            prop = icalproperty_new_dtstart(start);
        } else {
            prop = writeICalDateTimeProperty(ICAL_DTSTART_PROPERTY, todo->dtStart(true), tzUsedList);
        }
        icalcomponent_add_property(vtodo, prop);
    }

    // completion date (UTC)
    if (todo->isCompleted()) {
        if (!todo->hasCompletedDate()) {
            // If the todo was created by KOrganizer<2.2 it does not have
            // a correct completion date. Set one now.
            todo->setCompleted(QDateTime::currentDateTimeUtc());
        }
        icaltimetype completed = writeICalUtcDateTime(todo->completed());
        icalcomponent_add_property(vtodo, icalproperty_new_completed(completed));
    }

    icalcomponent_add_property(vtodo, icalproperty_new_percentcomplete(todo->percentComplete()));

    if (todo->isCompleted()) {
        if (icalcomponent_count_properties(vtodo, ICAL_STATUS_PROPERTY)) {
            icalproperty *p = icalcomponent_get_first_property(vtodo, ICAL_STATUS_PROPERTY);
            icalcomponent_remove_property(vtodo, p);
            icalproperty_free(p);
        }
        icalcomponent_add_property(vtodo, icalproperty_new_status(ICAL_STATUS_COMPLETED));
    }

    if (todo->recurs() && todo->dtStart(false).isValid()) {
        prop = writeICalDateTimeProperty(ICAL_X_PROPERTY, todo->dtStart(false), tzUsedList);
        icalproperty_set_x_name(prop, "X-KDE-LIBKCAL-DTRECURRENCE");
        icalcomponent_add_property(vtodo, prop);
    }

    return vtodo;
}

icalcomponent *ICalFormatImpl::writeEvent(const Event::Ptr &event, TimeZoneList *tzUsedList)
{
    icalcomponent *vevent = icalcomponent_new(ICAL_VEVENT_COMPONENT);

    writeIncidence(vevent, event.staticCast<Incidence>(), tzUsedList);

    // start time
    icalproperty *prop = nullptr;
    icaltimetype start;

    QDateTime dt = event->dtStart();
    if (dt.isValid()) {
        if (event->allDay()) {
            start = writeICalDate(event->dtStart().date());
            prop = icalproperty_new_dtstart(start);
        } else {
            prop = writeICalDateTimeProperty(ICAL_DTSTART_PROPERTY, event->dtStart(), tzUsedList);
        }
        icalcomponent_add_property(vevent, prop);
    }

    if (event->hasEndDate()) {
        // End time.
        // RFC2445 says that if DTEND is present, it has to be greater than DTSTART.
        icaltimetype end;
        QDateTime dt = event->dtEnd();
        if (event->allDay()) {
            // +1 day because end date is non-inclusive.
            end = writeICalDate(dt.date().addDays(1));
            icalcomponent_add_property(vevent, icalproperty_new_dtend(end));
        } else {
            if (dt != event->dtStart()) {
                icalcomponent_add_property(vevent, writeICalDateTimeProperty(ICAL_DTEND_PROPERTY, dt, tzUsedList));
            }
        }
    }

// TODO: resources
#if 0
    // resources
    QStringList tmpStrList = anEvent->resources();
    QString tmpStr = tmpStrList.join(";");
    if (!tmpStr.isEmpty()) {
        addPropValue(vevent, VCResourcesProp, tmpStr.toUtf8());
    }

#endif

    // Transparency
    switch (event->transparency()) {
    case Event::Transparent:
        icalcomponent_add_property(vevent, icalproperty_new_transp(ICAL_TRANSP_TRANSPARENT));
        break;
    case Event::Opaque:
        icalcomponent_add_property(vevent, icalproperty_new_transp(ICAL_TRANSP_OPAQUE));
        break;
    }

    //农历
    if (event->lunnar()) {
        event->setNonKDECustomProperty(VEVENT_LUNNAR_XPROPERTY, QStringLiteral("TRUE"));
    }

    // Custom properties
    const QMap<QByteArray, QString> custom = event->customProperties();
    for (QMap<QByteArray, QString>::ConstIterator c = custom.begin(); c != custom.end(); ++c) {
        icalproperty *p = icalproperty_new_x(c.value().toUtf8().constData());
        icalproperty_set_x_name(p, c.key().constData());
        icalcomponent_add_property(vevent, p);
    }

    return vevent;
}

icalcomponent *ICalFormatImpl::writeFreeBusy(const FreeBusy::Ptr &freebusy, iTIPMethod method)
{
    icalcomponent *vfreebusy = icalcomponent_new(ICAL_VFREEBUSY_COMPONENT);

    d->writeIncidenceBase(vfreebusy, freebusy.staticCast<IncidenceBase>());

    icalcomponent_add_property(vfreebusy, icalproperty_new_dtstart(writeICalUtcDateTime(freebusy->dtStart())));

    icalcomponent_add_property(vfreebusy, icalproperty_new_dtend(writeICalUtcDateTime(freebusy->dtEnd())));

    Q_UNUSED(method);
    icalcomponent_add_property(vfreebusy, icalproperty_new_uid(freebusy->uid().toUtf8().constData()));

    // Loops through all the periods in the freebusy object
    FreeBusyPeriod::List list = freebusy->fullBusyPeriods();
    icalperiodtype period = icalperiodtype_null_period();
    for (int i = 0, count = list.count(); i < count; ++i) {
        const FreeBusyPeriod fbPeriod = list[i];
        period.start = writeICalUtcDateTime(fbPeriod.start());
        if (fbPeriod.hasDuration()) {
            period.duration = writeICalDuration(fbPeriod.duration());
        } else {
            period.end = writeICalUtcDateTime(fbPeriod.end());
        }

        icalproperty *property = icalproperty_new_freebusy(period);

        icalparameter_fbtype fbType;
        switch (fbPeriod.type()) {
        case FreeBusyPeriod::Free:
            fbType = ICAL_FBTYPE_FREE;
            break;
        case FreeBusyPeriod::Busy:
            fbType = ICAL_FBTYPE_BUSY;
            break;
        case FreeBusyPeriod::BusyTentative:
            fbType = ICAL_FBTYPE_BUSYTENTATIVE;
            break;
        case FreeBusyPeriod::BusyUnavailable:
            fbType = ICAL_FBTYPE_BUSYUNAVAILABLE;
            break;
        case FreeBusyPeriod::Unknown:
            fbType = ICAL_FBTYPE_X;
            break;
        default:
            fbType = ICAL_FBTYPE_NONE;
            break;
        }
        icalproperty_set_parameter(property, icalparameter_new_fbtype(fbType));

        if (!fbPeriod.summary().isEmpty()) {
            icalparameter *param = icalparameter_new_x("X-SUMMARY");
            icalparameter_set_xvalue(param, fbPeriod.summary().toUtf8().toBase64().constData());
            icalproperty_set_parameter(property, param);
        }
        if (!fbPeriod.location().isEmpty()) {
            icalparameter *param = icalparameter_new_x("X-LOCATION");
            icalparameter_set_xvalue(param, fbPeriod.location().toUtf8().toBase64().constData());
            icalproperty_set_parameter(property, param);
        }

        icalcomponent_add_property(vfreebusy, property);
    }

    return vfreebusy;
}

icalcomponent *ICalFormatImpl::writeJournal(const Journal::Ptr &journal, TimeZoneList *tzUsedList)
{
    icalcomponent *vjournal = icalcomponent_new(ICAL_VJOURNAL_COMPONENT);

    writeIncidence(vjournal, journal.staticCast<Incidence>(), tzUsedList);

    // start time
    icalproperty *prop = nullptr;
    QDateTime dt = journal->dtStart();
    if (dt.isValid()) {
        icaltimetype start;
        if (journal->allDay()) {
            start = writeICalDate(dt.date());
            prop = icalproperty_new_dtstart(start);
        } else {
            prop = writeICalDateTimeProperty(ICAL_DTSTART_PROPERTY, dt, tzUsedList);
        }
        icalcomponent_add_property(vjournal, prop);
    }

    return vjournal;
}

void ICalFormatImpl::writeIncidence(icalcomponent *parent, const Incidence::Ptr &incidence, TimeZoneList *tzUsedList)
{
    if (incidence->schedulingID() != incidence->uid()) {
        // We need to store the UID in here. The rawSchedulingID will
        // go into the iCal UID component
        incidence->setCustomProperty("LIBKCAL", "ID", incidence->uid());
    } else {
        incidence->removeCustomProperty("LIBKCAL", "ID");
    }

    d->writeIncidenceBase(parent, incidence.staticCast<IncidenceBase>());

    // creation date in storage
    icalcomponent_add_property(parent, writeICalDateTimeProperty(ICAL_CREATED_PROPERTY, incidence->created()));

    // unique id
    // If the scheduling ID is different from the real UID, the real
    // one is stored on X-REALID above
    if (!incidence->schedulingID().isEmpty()) {
        icalcomponent_add_property(parent, icalproperty_new_uid(incidence->schedulingID().toUtf8().constData()));
    }

    // revision
    if (incidence->revision() > 0) { // 0 is default, so don't write that out
        icalcomponent_add_property(parent, icalproperty_new_sequence(incidence->revision()));
    }

    // last modification date
    if (incidence->lastModified().isValid()) {
        icalcomponent_add_property(parent, writeICalDateTimeProperty(ICAL_LASTMODIFIED_PROPERTY, incidence->lastModified()));
    }

    // description
    if (!incidence->description().isEmpty()) {
        icalcomponent_add_property(parent, writeDescription(incidence->description(), incidence->descriptionIsRich()));
    }

    // summary
    if (!incidence->summary().isEmpty()) {
        icalcomponent_add_property(parent, writeSummary(incidence->summary(), incidence->summaryIsRich()));
    }

    // location
    if (!incidence->location().isEmpty()) {
        icalcomponent_add_property(parent, writeLocation(incidence->location(), incidence->locationIsRich()));
    }

    // status
    icalproperty_status status = ICAL_STATUS_NONE;
    switch (incidence->status()) {
    case Incidence::StatusTentative:
        status = ICAL_STATUS_TENTATIVE;
        break;
    case Incidence::StatusConfirmed:
        status = ICAL_STATUS_CONFIRMED;
        break;
    case Incidence::StatusCompleted:
        status = ICAL_STATUS_COMPLETED;
        break;
    case Incidence::StatusNeedsAction:
        status = ICAL_STATUS_NEEDSACTION;
        break;
    case Incidence::StatusCanceled:
        status = ICAL_STATUS_CANCELLED;
        break;
    case Incidence::StatusInProcess:
        status = ICAL_STATUS_INPROCESS;
        break;
    case Incidence::StatusDraft:
        status = ICAL_STATUS_DRAFT;
        break;
    case Incidence::StatusFinal:
        status = ICAL_STATUS_FINAL;
        break;
    case Incidence::StatusX: {
        icalproperty *p = icalproperty_new_status(ICAL_STATUS_X);
        icalvalue_set_x(icalproperty_get_value(p), incidence->customStatus().toUtf8().constData());
        icalcomponent_add_property(parent, p);
        break;
    }
    case Incidence::StatusNone:
    default:
        break;
    }
    if (status != ICAL_STATUS_NONE) {
        icalcomponent_add_property(parent, icalproperty_new_status(status));
    }

    // secrecy
    icalproperty_class secClass;
    switch (incidence->secrecy()) {
    case Incidence::SecrecyPublic:
        secClass = ICAL_CLASS_PUBLIC;
        break;
    case Incidence::SecrecyConfidential:
        secClass = ICAL_CLASS_CONFIDENTIAL;
        break;
    case Incidence::SecrecyPrivate:
    default:
        secClass = ICAL_CLASS_PRIVATE;
        break;
    }
    if (secClass != ICAL_CLASS_PUBLIC) {
        icalcomponent_add_property(parent, icalproperty_new_class(secClass));
    }

    // color
    if (!incidence->color().isEmpty()) {
        icalcomponent_add_property(parent, icalproperty_new_color(incidence->color().toUtf8().constData()));
    }

    // geo
    if (incidence->hasGeo()) {
        icalgeotype geo;
        geo.lat = incidence->geoLatitude();
        geo.lon = incidence->geoLongitude();
        icalcomponent_add_property(parent, icalproperty_new_geo(geo));
    }

    // priority
    if (incidence->priority() > 0) { // 0 is undefined priority
        icalcomponent_add_property(parent, icalproperty_new_priority(incidence->priority()));
    }

    // categories
    QString categories = incidence->categories().join(QLatin1Char(','));
    if (!categories.isEmpty()) {
        icalcomponent_add_property(parent, icalproperty_new_categories(categories.toUtf8().constData()));
    }

    // related event
    if (!incidence->relatedTo().isEmpty()) {
        icalcomponent_add_property(parent, icalproperty_new_relatedto(incidence->relatedTo().toUtf8().constData()));
    }

    // recurrenceid
    if (incidence->hasRecurrenceId()) {
        icalproperty *p = writeICalDateTimeProperty(ICAL_RECURRENCEID_PROPERTY, incidence->recurrenceId(), tzUsedList);
        if (incidence->thisAndFuture()) {
            icalproperty_add_parameter(p, icalparameter_new_range(ICAL_RANGE_THISANDFUTURE));
        }
        icalcomponent_add_property(parent, p);
    }

    RecurrenceRule::List rrules(incidence->recurrence()->rRules());
    RecurrenceRule::List::ConstIterator rit;
    for (rit = rrules.constBegin(); rit != rrules.constEnd(); ++rit) {
        icalcomponent_add_property(parent, icalproperty_new_rrule(writeRecurrenceRule((*rit))));
    }

    RecurrenceRule::List exrules(incidence->recurrence()->exRules());
    RecurrenceRule::List::ConstIterator exit;
    for (exit = exrules.constBegin(); exit != exrules.constEnd(); ++exit) {
        icalcomponent_add_property(parent, icalproperty_new_exrule(writeRecurrenceRule((*exit))));
    }

    DateList dateList = incidence->recurrence()->exDates();
    DateList::ConstIterator exIt;
    for (exIt = dateList.constBegin(); exIt != dateList.constEnd(); ++exIt) {
        icalcomponent_add_property(parent, icalproperty_new_exdate(writeICalDate(*exIt)));
    }

    auto dateTimeList = incidence->recurrence()->exDateTimes();
    for (auto extIt = dateTimeList.constBegin(); extIt != dateTimeList.constEnd(); ++extIt) {
        icalcomponent_add_property(parent, writeICalDateTimeProperty(ICAL_EXDATE_PROPERTY, *extIt, tzUsedList));
    }

    dateList = incidence->recurrence()->rDates();
    DateList::ConstIterator rdIt;
    for (rdIt = dateList.constBegin(); rdIt != dateList.constEnd(); ++rdIt) {
        icalcomponent_add_property(parent, icalproperty_new_rdate(writeICalDatePeriod(*rdIt)));
    }
    dateTimeList = incidence->recurrence()->rDateTimes();
    for (auto rdtIt = dateTimeList.constBegin(); rdtIt != dateTimeList.constEnd(); ++rdtIt) {
        icalcomponent_add_property(parent, writeICalDateTimeProperty(ICAL_RDATE_PROPERTY, *rdtIt, tzUsedList));
    }

    // attachments
    Attachment::List attachments = incidence->attachments();
    Attachment::List::ConstIterator atIt;
    for (atIt = attachments.constBegin(); atIt != attachments.constEnd(); ++atIt) {
        icalcomponent_add_property(parent, writeAttachment(*atIt));
    }

    // alarms
    auto alarms = incidence->alarms();
    for (auto it = alarms.cbegin(), end = alarms.cend(); it != end; ++it) {
        icalcomponent_add_component(parent, writeAlarm(*it));
    }

    // conferences
    const auto conferences = incidence->conferences();
    for (const auto &conf : conferences) {
        icalcomponent_add_property(parent, writeConference(conf));
    }

    // duration
    if (incidence->hasDuration()) {
        icaldurationtype duration;
        duration = writeICalDuration(incidence->duration());
        icalcomponent_add_property(parent, icalproperty_new_duration(duration));
    }
}

//@cond PRIVATE
void ICalFormatImpl::Private::writeIncidenceBase(icalcomponent *parent, const IncidenceBase::Ptr &incidenceBase)
{
    // organizer stuff
    if (!incidenceBase->organizer().isEmpty()) {
        icalproperty *p = mImpl->writeOrganizer(incidenceBase->organizer());
        if (p) {
            icalcomponent_add_property(parent, p);
        }
    }

    icalcomponent_add_property(parent, icalproperty_new_dtstamp(writeICalUtcDateTime(incidenceBase->lastModified())));

    // attendees
    if (incidenceBase->attendeeCount() > 0) {
        auto attendees = incidenceBase->attendees();
        for (auto it = attendees.constBegin(); it != attendees.constEnd(); ++it) {
            icalproperty *p = mImpl->writeAttendee(*it);
            if (p) {
                icalcomponent_add_property(parent, p);
            }
        }
    }

    // contacts
    QStringList contacts = incidenceBase->contacts();
    for (QStringList::const_iterator it = contacts.constBegin(); it != contacts.constEnd(); ++it) {
        icalcomponent_add_property(parent, icalproperty_new_contact((*it).toUtf8().constData()));
    }

    // comments
    QStringList comments = incidenceBase->comments();
    for (QStringList::const_iterator it = comments.constBegin(); it != comments.constEnd(); ++it) {
        icalcomponent_add_property(parent, icalproperty_new_comment((*it).toUtf8().constData()));
    }

    // url
    const QUrl url = incidenceBase->url();
    if (url.isValid()) {
        icalcomponent_add_property(parent, icalproperty_new_url(url.toString().toUtf8().constData()));
    }

    // custom properties
    writeCustomProperties(parent, incidenceBase.data());
}

void ICalFormatImpl::Private::writeCustomProperties(icalcomponent *parent, CustomProperties *properties)
{
    const QMap<QByteArray, QString> custom = properties->customProperties();
    for (QMap<QByteArray, QString>::ConstIterator c = custom.begin(); c != custom.end(); ++c) {
        if (c.key().startsWith("X-KDE-VOLATILE")) { // krazy:exclude=strings
            // We don't write these properties to disk to disk
            continue;
        }
        icalproperty *p = icalproperty_new_x(c.value().toUtf8().constData());
        QString parameters = properties->nonKDECustomPropertyParameters(c.key());

        // Minimalist parameter handler: extract icalparameter's out of
        // the given input text (not really parsing as such)
        if (!parameters.isEmpty()) {
            const QStringList sl = parameters.split(QLatin1Char(';'));
            for (const QString &parameter : sl) {
                icalparameter *param = icalparameter_new_from_string(parameter.toUtf8().constData());
                if (param) {
                    icalproperty_add_parameter(p, param);
                }
            }
        }

        icalproperty_set_x_name(p, c.key().constData());
        icalcomponent_add_property(parent, p);
    }
}
//@endcond

icalproperty *ICalFormatImpl::writeOrganizer(const Person &organizer)
{
    if (organizer.email().isEmpty()) {
        return nullptr;
    }

    icalproperty *p = icalproperty_new_organizer(QByteArray(QByteArray("MAILTO:") + organizer.email().toUtf8()).constData());

    if (!organizer.name().isEmpty()) {
        icalproperty_add_parameter(p,
                                   icalparameter_new_cn(organizer.name().toUtf8().constData()));
    }
    // TODO: Write dir, sent-by and language

    return p;
}

icalproperty *ICalFormatImpl::writeDescription(const QString &description, bool isRich)
{
    icalproperty *p = icalproperty_new_description(description.toUtf8().constData());
    if (isRich) {
        icalproperty_add_parameter(p, icalparameter_new_from_string("X-KDE-TEXTFORMAT=HTML"));
    }
    return p;
}

icalproperty *ICalFormatImpl::writeSummary(const QString &summary, bool isRich)
{
    icalproperty *p = icalproperty_new_summary(summary.toUtf8().constData());
    if (isRich) {
        icalproperty_add_parameter(p, icalparameter_new_from_string("X-KDE-TEXTFORMAT=HTML"));
    }
    return p;
}

icalproperty *ICalFormatImpl::writeLocation(const QString &location, bool isRich)
{
    icalproperty *p = icalproperty_new_location(location.toUtf8().constData());
    if (isRich) {
        icalproperty_add_parameter(p, icalparameter_new_from_string("X-KDE-TEXTFORMAT=HTML"));
    }
    return p;
}

icalproperty *ICalFormatImpl::writeAttendee(const Attendee &attendee)
{
    if (attendee.email().isEmpty()) {
        return nullptr;
    }

    icalproperty *p = icalproperty_new_attendee(QByteArray(QByteArray("mailto:") + attendee.email().toUtf8()).constData());

    if (!attendee.name().isEmpty()) {
        icalproperty_add_parameter(p,
                                   icalparameter_new_cn(attendee.name().toUtf8().constData()));
    }

    icalproperty_add_parameter(p, icalparameter_new_rsvp(attendee.RSVP() ? ICAL_RSVP_TRUE : ICAL_RSVP_FALSE));

    icalparameter_partstat status = ICAL_PARTSTAT_NEEDSACTION;
    switch (attendee.status()) {
    default:
    case Attendee::NeedsAction:
        status = ICAL_PARTSTAT_NEEDSACTION;
        break;
    case Attendee::Accepted:
        status = ICAL_PARTSTAT_ACCEPTED;
        break;
    case Attendee::Declined:
        status = ICAL_PARTSTAT_DECLINED;
        break;
    case Attendee::Tentative:
        status = ICAL_PARTSTAT_TENTATIVE;
        break;
    case Attendee::Delegated:
        status = ICAL_PARTSTAT_DELEGATED;
        break;
    case Attendee::Completed:
        status = ICAL_PARTSTAT_COMPLETED;
        break;
    case Attendee::InProcess:
        status = ICAL_PARTSTAT_INPROCESS;
        break;
    }
    icalproperty_add_parameter(p, icalparameter_new_partstat(status));

    icalparameter_role role = ICAL_ROLE_REQPARTICIPANT;
    switch (attendee.role()) {
    case Attendee::Chair:
        role = ICAL_ROLE_CHAIR;
        break;
    default:
    case Attendee::ReqParticipant:
        role = ICAL_ROLE_REQPARTICIPANT;
        break;
    case Attendee::OptParticipant:
        role = ICAL_ROLE_OPTPARTICIPANT;
        break;
    case Attendee::NonParticipant:
        role = ICAL_ROLE_NONPARTICIPANT;
        break;
    }
    icalproperty_add_parameter(p, icalparameter_new_role(role));

    icalparameter_cutype cutype = ICAL_CUTYPE_INDIVIDUAL;
    switch (attendee.cuType()) {
    case Attendee::Unknown:
        cutype = ICAL_CUTYPE_UNKNOWN;
        break;
    default:
    case Attendee::Individual:
        cutype = ICAL_CUTYPE_INDIVIDUAL;
        break;
    case Attendee::Group:
        cutype = ICAL_CUTYPE_GROUP;
        break;
    case Attendee::Resource:
        cutype = ICAL_CUTYPE_RESOURCE;
        break;
    case Attendee::Room:
        cutype = ICAL_CUTYPE_ROOM;
        break;
    }
    icalproperty_add_parameter(p, icalparameter_new_cutype(cutype));

    if (!attendee.uid().isEmpty()) {
        icalparameter *icalparameter_uid = icalparameter_new_x(attendee.uid().toUtf8().constData());

        icalparameter_set_xname(icalparameter_uid, "X-UID");
        icalproperty_add_parameter(p, icalparameter_uid);
    }

    if (!attendee.delegate().isEmpty()) {
        icalparameter *icalparameter_delegate = icalparameter_new_delegatedto(attendee.delegate().toUtf8().constData());
        icalproperty_add_parameter(p, icalparameter_delegate);
    }

    if (!attendee.delegator().isEmpty()) {
        icalparameter *icalparameter_delegator = icalparameter_new_delegatedfrom(attendee.delegator().toUtf8().constData());
        icalproperty_add_parameter(p, icalparameter_delegator);
    }

    return p;
}

icalproperty *ICalFormatImpl::writeAttachment(const Attachment &att)
{
    icalattach *attach;
    if (att.isUri()) {
        attach = icalattach_new_from_url(att.uri().toUtf8().data());
    } else {
        attach = icalattach_new_from_data((const char *)att.data().constData(), nullptr, nullptr);
    }
    icalproperty *p = icalproperty_new_attach(attach);

    icalattach_unref(attach);

    if (!att.mimeType().isEmpty()) {
        icalproperty_add_parameter(p, icalparameter_new_fmttype(att.mimeType().toUtf8().data()));
    }

    if (att.isBinary()) {
        icalproperty_add_parameter(p, icalparameter_new_value(ICAL_VALUE_BINARY));
        icalproperty_add_parameter(p, icalparameter_new_encoding(ICAL_ENCODING_BASE64));
    }

    if (att.showInline()) {
        icalparameter *icalparameter_inline = icalparameter_new_x("inline");
        icalparameter_set_xname(icalparameter_inline, "X-CONTENT-DISPOSITION");
        icalproperty_add_parameter(p, icalparameter_inline);
    }

    if (!att.label().isEmpty()) {
        icalparameter *icalparameter_label = icalparameter_new_x(att.label().toUtf8().constData());
        icalparameter_set_xname(icalparameter_label, "X-LABEL");
        icalproperty_add_parameter(p, icalparameter_label);
    }

    if (att.isLocal()) {
        icalparameter *icalparameter_local = icalparameter_new_x("local");
        icalparameter_set_xname(icalparameter_local, "X-KONTACT-TYPE");
        icalproperty_add_parameter(p, icalparameter_local);
    }

    return p;
}

icalrecurrencetype ICalFormatImpl::writeRecurrenceRule(RecurrenceRule *recur)
{
    icalrecurrencetype r;
    icalrecurrencetype_clear(&r);

    switch (recur->recurrenceType()) {
    case RecurrenceRule::rSecondly:
        r.freq = ICAL_SECONDLY_RECURRENCE;
        break;
    case RecurrenceRule::rMinutely:
        r.freq = ICAL_MINUTELY_RECURRENCE;
        break;
    case RecurrenceRule::rHourly:
        r.freq = ICAL_HOURLY_RECURRENCE;
        break;
    case RecurrenceRule::rDaily:
        r.freq = ICAL_DAILY_RECURRENCE;
        break;
    case RecurrenceRule::rWeekly:
        r.freq = ICAL_WEEKLY_RECURRENCE;
        break;
    case RecurrenceRule::rMonthly:
        r.freq = ICAL_MONTHLY_RECURRENCE;
        break;
    case RecurrenceRule::rYearly:
        r.freq = ICAL_YEARLY_RECURRENCE;
        break;
    default:
        r.freq = ICAL_NO_RECURRENCE;
        qDebug() << "no recurrence";
        break;
    }

    int index = 0;
    QList<int> bys;
    QList<int>::ConstIterator it;

    // Now write out the BY* parts:
    bys = recur->bySeconds();
    index = 0;
    for (it = bys.constBegin(); it != bys.constEnd(); ++it) {
        r.by_second[index++] = *it;
        r.by_second[index++] = static_cast<short>(*it);
    }

    bys = recur->byMinutes();
    index = 0;
    for (it = bys.constBegin(); it != bys.constEnd(); ++it) {
        r.by_minute[index++] = *it;
        r.by_minute[index++] = static_cast<short>(*it);
    }

    bys = recur->byHours();
    index = 0;
    for (it = bys.constBegin(); it != bys.constEnd(); ++it) {
        r.by_hour[index++] = *it;
        r.by_hour[index++] = static_cast<short>(*it);
    }

    bys = recur->byMonthDays();
    index = 0;
    for (it = bys.constBegin(); it != bys.constEnd(); ++it) {
        short dShort = static_cast<short>((*it) * 8);
        r.by_month_day[index++] = static_cast<short>(icalrecurrencetype_day_position(dShort));
    }

    bys = recur->byYearDays();
    index = 0;
    for (it = bys.constBegin(); it != bys.constEnd(); ++it) {
        r.by_year_day[index++] = static_cast<short>(*it);
    }

    bys = recur->byWeekNumbers();
    index = 0;
    for (it = bys.constBegin(); it != bys.constEnd(); ++it) {
        r.by_week_no[index++] = static_cast<short>(*it);
    }

    bys = recur->byMonths();
    index = 0;
    for (it = bys.constBegin(); it != bys.constEnd(); ++it) {
        r.by_month[index++] = static_cast<short>(*it);
    }

    bys = recur->bySetPos();
    index = 0;
    for (it = bys.constBegin(); it != bys.constEnd(); ++it) {
        r.by_set_pos[index++] = static_cast<short>(*it);
    }

    const QList<RecurrenceRule::WDayPos> &byd = recur->byDays();
    int day;
    index = 0;
    for (QList<RecurrenceRule::WDayPos>::ConstIterator dit = byd.constBegin(); dit != byd.constEnd(); ++dit) {
        day = (*dit).day() % 7 + 1; // convert from Monday=1 to Sunday=1
        if ((*dit).pos() < 0) {
            day += (-(*dit).pos()) * 8;
            day = -day;
        } else {
            day += (*dit).pos() * 8;
        }
        r.by_day[index++] = static_cast<short>(day);
    }

    r.week_start = static_cast<icalrecurrencetype_weekday>(recur->weekStart() % 7 + 1);

    if (recur->frequency() > 1) {
        // Don't write out INTERVAL=1, because that's the default anyway
        r.interval = static_cast<short>(recur->frequency());
    }

    if (recur->duration() > 0) {
        r.count = recur->duration();
    } else if (recur->duration() == -1) {
        r.count = 0;
    } else {
        if (recur->allDay()) {
            r.until = writeICalDate(recur->endDt().date());
        } else {
            r.until = writeICalDateTime(recur->endDt());
        }
    }

    return r;
}

icalcomponent *ICalFormatImpl::writeAlarm(const Alarm::Ptr &alarm)
{
    if (alarm->enabled()) {
        alarm->setCustomProperty(APP_NAME_FOR_XPROPERTIES, ENABLED_ALARM_XPROPERTY, QStringLiteral("TRUE"));
    } else {
        alarm->setCustomProperty(APP_NAME_FOR_XPROPERTIES, ENABLED_ALARM_XPROPERTY, QStringLiteral("FALSE"));
    }

    icalcomponent *a = icalcomponent_new(ICAL_VALARM_COMPONENT);

    icalproperty_action action;
    icalattach *attach = nullptr;

    switch (alarm->type()) {
    case Alarm::Procedure:
        action = ICAL_ACTION_PROCEDURE;
        attach = icalattach_new_from_url(QFile::encodeName(alarm->programFile()).data());
        icalcomponent_add_property(a, icalproperty_new_attach(attach));
        if (!alarm->programArguments().isEmpty()) {
            icalcomponent_add_property(a, icalproperty_new_description(alarm->programArguments().toUtf8().constData()));
        }
        break;
    case Alarm::Audio:
        action = ICAL_ACTION_AUDIO;
        if (!alarm->audioFile().isEmpty()) {
            attach = icalattach_new_from_url(QFile::encodeName(alarm->audioFile()).data());
            icalcomponent_add_property(a, icalproperty_new_attach(attach));
        }
        break;
    case Alarm::Email: {
        action = ICAL_ACTION_EMAIL;
        const Person::List addresses = alarm->mailAddresses();
        for (Person::List::ConstIterator ad = addresses.constBegin(); ad != addresses.constEnd(); ++ad) {
            if (!(*ad).email().isEmpty()) {
                icalproperty *p = icalproperty_new_attendee(QByteArray(QByteArray("MAILTO:") + (*ad).email().toUtf8()).constData());
                if (!(*ad).name().isEmpty()) {
                    icalproperty_add_parameter(p,
                                               icalparameter_new_cn((*ad).name().toUtf8().constData()));
                }
                icalcomponent_add_property(a, p);
            }
        }
        icalcomponent_add_property(a, icalproperty_new_summary(alarm->mailSubject().toUtf8().constData()));
        icalcomponent_add_property(a, icalproperty_new_description(alarm->mailText().toUtf8().constData()));
        const QStringList attachments = alarm->mailAttachments();
        if (!attachments.isEmpty()) {
            for (QStringList::const_iterator at = attachments.constBegin(), end = attachments.constEnd(); at != end; ++at) {
                attach = icalattach_new_from_url(QFile::encodeName(*at).data());
                icalcomponent_add_property(a, icalproperty_new_attach(attach));
            }
        }
        break;
    }
    case Alarm::Display:
        action = ICAL_ACTION_DISPLAY;
        icalcomponent_add_property(a, icalproperty_new_description(alarm->text().toUtf8().constData()));
        break;
    case Alarm::Invalid:
    default:
        qDebug() << "Unknown type of alarm";
        action = ICAL_ACTION_NONE;
        break;
    }
    icalcomponent_add_property(a, icalproperty_new_action(action));

    // Trigger time
    icaltriggertype trigger;
    if (alarm->hasTime()) {
        trigger.time = writeICalUtcDateTime(alarm->time(), false);
        trigger.duration = icaldurationtype_null_duration();
    } else {
        trigger.time = icaltime_null_time();
        Duration offset;
        if (alarm->hasStartOffset()) {
            offset = alarm->startOffset();
        } else {
            offset = alarm->endOffset();
        }
        trigger.duration = writeICalDuration(offset);
    }
    icalproperty *p = icalproperty_new_trigger(trigger);
    if (alarm->hasEndOffset()) {
        icalproperty_add_parameter(p, icalparameter_new_related(ICAL_RELATED_END));
    }
    icalcomponent_add_property(a, p);

    // Repeat count and duration
    if (alarm->repeatCount()) {
        icalcomponent_add_property(a, icalproperty_new_repeat(alarm->repeatCount()));
        icalcomponent_add_property(a, icalproperty_new_duration(writeICalDuration(alarm->snoozeTime())));
    }

    // Custom properties
    const QMap<QByteArray, QString> custom = alarm->customProperties();
    for (QMap<QByteArray, QString>::ConstIterator c = custom.begin(); c != custom.end(); ++c) {
        icalproperty *p = icalproperty_new_x(c.value().toUtf8().constData());
        icalproperty_set_x_name(p, c.key().constData());
        icalcomponent_add_property(a, p);
    }

    icalattach_unref(attach);

    return a;
}

icalproperty *ICalFormatImpl::writeConference(const Conference &conference)
{
    icalproperty *p = icalproperty_new_conference(conference.uri().toString().toUtf8().constData());
    icalproperty_set_parameter_from_string(p, "VALUE", "URI");
    icalproperty_set_parameter_from_string(p, "FEATURE", conference.features().join(QLatin1Char(',')).toUtf8().constData());
    icalproperty_set_parameter_from_string(p, "LABEL", conference.label().toUtf8().constData());

    return p;
}

Todo::Ptr ICalFormatImpl::readTodo(icalcomponent *vtodo, const ICalTimeZoneCache *tzlist)
{
    Todo::Ptr todo(new Todo);

    readIncidence(vtodo, todo, tzlist);

    icalproperty *p = icalcomponent_get_first_property(vtodo, ICAL_ANY_PROPERTY);

    while (p) {
        icalproperty_kind kind = icalproperty_isa(p);
        switch (kind) {
        case ICAL_DUE_PROPERTY: {
            // due date/time
            bool allDay = false;
            QDateTime kdt = readICalDateTimeProperty(p, tzlist, false, &allDay);
            todo->setDtDue(kdt, true);
            todo->setAllDay(allDay);
            break;
        }
        case ICAL_COMPLETED_PROPERTY: // completion date/time
            todo->setCompleted(readICalDateTimeProperty(p, tzlist));
            break;

        case ICAL_PERCENTCOMPLETE_PROPERTY: // Percent completed
            todo->setPercentComplete(icalproperty_get_percentcomplete(p));
            break;

        case ICAL_RELATEDTO_PROPERTY: // related todo (parent)
            todo->setRelatedTo(QString::fromUtf8(icalproperty_get_relatedto(p)));
            d->mTodosRelate.append(todo);
            break;

        case ICAL_DTSTART_PROPERTY:
            // Flag that todo has start date. Value is read in by readIncidence().
            if (!todo->comments().filter(QStringLiteral("NoStartDate")).isEmpty()) {
                todo->setDtStart(QDateTime());
            }
            break;
        case ICAL_X_PROPERTY: {
            const char *name = icalproperty_get_x_name(p);
            if (QLatin1String(name) == QLatin1String("X-KDE-LIBKCAL-DTRECURRENCE")) {
                const QDateTime dateTime = readICalDateTimeProperty(p, tzlist);
                if (dateTime.isValid()) {
                    todo->setDtRecurrence(dateTime);
                } else {
                    qDebug() << "Invalid dateTime";
                }
            }
        } break;
        default:
            // TODO: do something about unknown properties?
            break;
        }

        p = icalcomponent_get_next_property(vtodo, ICAL_ANY_PROPERTY);
    }

    if (d->mCompat) {
        d->mCompat->fixEmptySummary(todo);
    }

    todo->resetDirtyFields();
    return todo;
}

Event::Ptr ICalFormatImpl::readEvent(icalcomponent *vevent, const ICalTimeZoneCache *tzlist)
{
    Event::Ptr event(new Event);

    readIncidence(vevent, event, tzlist);

    icalproperty *p = icalcomponent_get_first_property(vevent, ICAL_ANY_PROPERTY);

    bool dtEndProcessed = false;

    while (p) {
        icalproperty_kind kind = icalproperty_isa(p);
        switch (kind) {
        case ICAL_DTEND_PROPERTY: {
            // end date and time
            bool allDay = false;
            QDateTime kdt = readICalDateTimeProperty(p, tzlist, false, &allDay);
            if (allDay) {
                // End date is non-inclusive
                QDate endDate = kdt.date().addDays(-1);
                if (d->mCompat) {
                    d->mCompat->fixFloatingEnd(endDate);
                }
                if (endDate < event->dtStart().date()) {
                    endDate = event->dtStart().date();
                }
                event->setDtEnd(QDateTime(endDate, {}, event->dtStart().timeZone()));
                event->setAllDay(true);
            } else {
                event->setDtEnd(kdt);
                event->setAllDay(false);
            }
            dtEndProcessed = true;
            break;
        }
        case ICAL_RELATEDTO_PROPERTY: // related event (parent)
            event->setRelatedTo(QString::fromUtf8(icalproperty_get_relatedto(p)));
            d->mEventsRelate.append(event);
            break;

        case ICAL_TRANSP_PROPERTY: { // Transparency
            icalproperty_transp transparency = icalproperty_get_transp(p);
            if (transparency == ICAL_TRANSP_TRANSPARENT) {
                event->setTransparency(Event::Transparent);
            } else {
                event->setTransparency(Event::Opaque);
            }
            break;
        }
        default:
            // TODO: do something about unknown properties?
            break;
        }

        p = icalcomponent_get_next_property(vevent, ICAL_ANY_PROPERTY);
    }

    // according to rfc2445 the dtend shouldn't be written when it equals
    // start date. so assign one equal to start date.
    if (!dtEndProcessed && !event->hasDuration()) {
        event->setDtEnd(event->dtStart());
    }

    //全天
    QString msade = event->nonKDECustomProperty("X-MICROSOFT-CDO-ALLDAYEVENT");
    if (!msade.isEmpty()) {
        bool allDay = (msade == QLatin1String("TRUE"));
        event->setAllDay(allDay);
    }

    //农历
    QString lunnar = event->nonKDECustomProperty(VEVENT_LUNNAR_XPROPERTY);
    if (!lunnar.isEmpty()) {
        //TODO:多次转换时会出现多个TRUE,待跟踪
        bool isLunnar = (lunnar.contains(QLatin1String("TRUE")));
        event->setLunnar(isLunnar);
    }

    if (d->mCompat) {
        d->mCompat->fixEmptySummary(event);
    }

    event->resetDirtyFields();
    return event;
}

FreeBusy::Ptr ICalFormatImpl::readFreeBusy(icalcomponent *vfreebusy)
{
    FreeBusy::Ptr freebusy(new FreeBusy);

    d->readIncidenceBase(vfreebusy, freebusy);

    icalproperty *p = icalcomponent_get_first_property(vfreebusy, ICAL_ANY_PROPERTY);

    FreeBusyPeriod::List periods;

    while (p) {
        icalproperty_kind kind = icalproperty_isa(p);
        switch (kind) {
        case ICAL_DTSTART_PROPERTY: // start date and time (UTC)
            freebusy->setDtStart(readICalUtcDateTimeProperty(p, nullptr));
            break;

        case ICAL_DTEND_PROPERTY: // end Date and Time (UTC)
            freebusy->setDtEnd(readICalUtcDateTimeProperty(p, nullptr));
            break;

        case ICAL_FREEBUSY_PROPERTY: { // Any FreeBusy Times (UTC)
            icalperiodtype icalperiod = icalproperty_get_freebusy(p);
            QDateTime period_start = readICalUtcDateTime(p, icalperiod.start);
            FreeBusyPeriod period;
            if (!icaltime_is_null_time(icalperiod.end)) {
                QDateTime period_end = readICalUtcDateTime(p, icalperiod.end);
                period = FreeBusyPeriod(period_start, period_end);
            } else {
                Duration duration(readICalDuration(icalperiod.duration));
                period = FreeBusyPeriod(period_start, duration);
            }

            icalparameter *param = icalproperty_get_first_parameter(p, ICAL_FBTYPE_PARAMETER);
            if (param) {
                icalparameter_fbtype fbType = icalparameter_get_fbtype(param);
                switch (fbType) {
                case ICAL_FBTYPE_FREE:
                    period.setType(FreeBusyPeriod::Free);
                    break;
                case ICAL_FBTYPE_BUSY:
                    period.setType(FreeBusyPeriod::Busy);
                    break;
                case ICAL_FBTYPE_BUSYTENTATIVE:
                    period.setType(FreeBusyPeriod::BusyTentative);
                    break;
                case ICAL_FBTYPE_BUSYUNAVAILABLE:
                    period.setType(FreeBusyPeriod::BusyUnavailable);
                    break;
                case ICAL_FBTYPE_X:
                    period.setType(FreeBusyPeriod::Unknown);
                    break;
                case ICAL_FBTYPE_NONE:
                    period.setType(FreeBusyPeriod::Free);
                    break;
                }
            }

            param = icalproperty_get_first_parameter(p, ICAL_X_PARAMETER);
            while (param) {
                if (strncmp(icalparameter_get_xname(param), "X-SUMMARY", 9) == 0) {
                    period.setSummary(QString::fromUtf8(QByteArray::fromBase64(icalparameter_get_xvalue(param))));
                }
                if (strncmp(icalparameter_get_xname(param), "X-LOCATION", 10) == 0) {
                    period.setLocation(QString::fromUtf8(QByteArray::fromBase64(icalparameter_get_xvalue(param))));
                }
                param = icalproperty_get_next_parameter(p, ICAL_X_PARAMETER);
            }

            periods.append(period);
            break;
        }

        default:
            // TODO: do something about unknown properties?
            break;
        }
        p = icalcomponent_get_next_property(vfreebusy, ICAL_ANY_PROPERTY);
    }
    freebusy->addPeriods(periods);

    freebusy->resetDirtyFields();
    return freebusy;
}

Journal::Ptr ICalFormatImpl::readJournal(icalcomponent *vjournal, const ICalTimeZoneCache *tzList)
{
    Journal::Ptr journal(new Journal);
    readIncidence(vjournal, journal, tzList);

    journal->resetDirtyFields();
    return journal;
}

Attendee ICalFormatImpl::readAttendee(icalproperty *attendee)
{
    // the following is a hack to support broken calendars (like WebCalendar 1.0.x)
    // that include non-RFC-compliant attendees.  Otherwise libical 0.42 asserts.
    if (!icalproperty_get_value(attendee)) {
        return {};
    }

    icalparameter *p = nullptr;

    QString email = QString::fromUtf8(icalproperty_get_attendee(attendee));
    if (email.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
        email.remove(0, 7);
    }

    // libical may return everything after ATTENDEE tag if the rest is
    // not meaningful. Verify the address to filter out these cases.
    if (!Person::isValidEmail(email)) {
        return {};
    }

    QString name;
    QString uid;
    p = icalproperty_get_first_parameter(attendee, ICAL_CN_PARAMETER);
    if (p) {
        name = QString::fromUtf8(icalparameter_get_cn(p));
    } else {
    }

    bool rsvp = false;
    p = icalproperty_get_first_parameter(attendee, ICAL_RSVP_PARAMETER);
    if (p) {
        icalparameter_rsvp rsvpParameter = icalparameter_get_rsvp(p);
        if (rsvpParameter == ICAL_RSVP_TRUE) {
            rsvp = true;
        }
    }

    Attendee::PartStat status = Attendee::NeedsAction;
    p = icalproperty_get_first_parameter(attendee, ICAL_PARTSTAT_PARAMETER);
    if (p) {
        icalparameter_partstat partStatParameter = icalparameter_get_partstat(p);
        switch (partStatParameter) {
        default:
        case ICAL_PARTSTAT_NEEDSACTION:
            status = Attendee::NeedsAction;
            break;
        case ICAL_PARTSTAT_ACCEPTED:
            status = Attendee::Accepted;
            break;
        case ICAL_PARTSTAT_DECLINED:
            status = Attendee::Declined;
            break;
        case ICAL_PARTSTAT_TENTATIVE:
            status = Attendee::Tentative;
            break;
        case ICAL_PARTSTAT_DELEGATED:
            status = Attendee::Delegated;
            break;
        case ICAL_PARTSTAT_COMPLETED:
            status = Attendee::Completed;
            break;
        case ICAL_PARTSTAT_INPROCESS:
            status = Attendee::InProcess;
            break;
        }
    }

    Attendee::Role role = Attendee::ReqParticipant;
    p = icalproperty_get_first_parameter(attendee, ICAL_ROLE_PARAMETER);
    if (p) {
        icalparameter_role roleParameter = icalparameter_get_role(p);
        switch (roleParameter) {
        case ICAL_ROLE_CHAIR:
            role = Attendee::Chair;
            break;
        default:
        case ICAL_ROLE_REQPARTICIPANT:
            role = Attendee::ReqParticipant;
            break;
        case ICAL_ROLE_OPTPARTICIPANT:
            role = Attendee::OptParticipant;
            break;
        case ICAL_ROLE_NONPARTICIPANT:
            role = Attendee::NonParticipant;
            break;
        }
    }

    Attendee::CuType cuType = Attendee::Individual;
    p = icalproperty_get_first_parameter(attendee, ICAL_CUTYPE_PARAMETER);
    if (p) {
        icalparameter_cutype cutypeParameter = icalparameter_get_cutype(p);
        switch (cutypeParameter) {
        case ICAL_CUTYPE_X:
        case ICAL_CUTYPE_UNKNOWN:
            cuType = Attendee::Unknown;
            break;
        default:
        case ICAL_CUTYPE_NONE:
        case ICAL_CUTYPE_INDIVIDUAL:
            cuType = Attendee::Individual;
            break;
        case ICAL_CUTYPE_GROUP:
            cuType = Attendee::Group;
            break;
        case ICAL_CUTYPE_RESOURCE:
            cuType = Attendee::Resource;
            break;
        case ICAL_CUTYPE_ROOM:
            cuType = Attendee::Room;
            break;
        }
    }

    p = icalproperty_get_first_parameter(attendee, ICAL_X_PARAMETER);
    QMap<QByteArray, QString> custom;
    while (p) {
        QString xname = QString::fromLatin1(icalparameter_get_xname(p)).toUpper();
        QString xvalue = QString::fromUtf8(icalparameter_get_xvalue(p));
        if (xname == QLatin1String("X-UID")) {
            uid = xvalue;
        } else {
            custom[xname.toUtf8()] = xvalue;
        }
        p = icalproperty_get_next_parameter(attendee, ICAL_X_PARAMETER);
    }

    Attendee a(name, email, rsvp, status, role, uid);
    a.setCuType(cuType);
    a.customProperties().setCustomProperties(custom);

    p = icalproperty_get_first_parameter(attendee, ICAL_DELEGATEDTO_PARAMETER);
    if (p) {
        a.setDelegate(QLatin1String(icalparameter_get_delegatedto(p)));
    }

    p = icalproperty_get_first_parameter(attendee, ICAL_DELEGATEDFROM_PARAMETER);
    if (p) {
        a.setDelegator(QLatin1String(icalparameter_get_delegatedfrom(p)));
    }

    return a;
}

Person ICalFormatImpl::readOrganizer(icalproperty *organizer)
{
    QString email = QString::fromUtf8(icalproperty_get_organizer(organizer));
    if (email.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
        email.remove(0, 7);
    }
    QString cn;

    icalparameter *p = icalproperty_get_first_parameter(organizer, ICAL_CN_PARAMETER);

    if (p) {
        cn = QString::fromUtf8(icalparameter_get_cn(p));
    }
    Person org(cn, email);
    // TODO: Treat sent-by, dir and language here, too
    return org;
}

Attachment ICalFormatImpl::readAttachment(icalproperty *attach)
{
    Attachment attachment;

    QByteArray p;
    icalvalue *value = icalproperty_get_value(attach);

    switch (icalvalue_isa(value)) {
    case ICAL_ATTACH_VALUE: {
        icalattach *a = icalproperty_get_attach(attach);
        if (!icalattach_get_is_url(a)) {
            p = QByteArray(reinterpret_cast<const char *>(icalattach_get_data(a)));
            if (!p.isEmpty()) {
                attachment = Attachment(p);
            }
        } else {
            p = icalattach_get_url(a);
            if (!p.isEmpty()) {
                attachment = Attachment(QString::fromUtf8(p));
            }
        }
        break;
    }
    case ICAL_BINARY_VALUE: {
        icalattach *a = icalproperty_get_attach(attach);
        p = QByteArray(reinterpret_cast<const char *>(icalattach_get_data(a)));
        if (!p.isEmpty()) {
            attachment = Attachment(p);
        }
        break;
    }
    case ICAL_URI_VALUE:
        p = icalvalue_get_uri(value);
        attachment = Attachment(QString::fromUtf8(p));
        break;
    default:
        break;
    }

    if (!attachment.isEmpty()) {
        icalparameter *p = icalproperty_get_first_parameter(attach, ICAL_FMTTYPE_PARAMETER);
        if (p) {
            attachment.setMimeType(QLatin1String(icalparameter_get_fmttype(p)));
        }

        p = icalproperty_get_first_parameter(attach, ICAL_X_PARAMETER);
        while (p) {
            QString xname = QString::fromLatin1(icalparameter_get_xname(p)).toUpper();
            QString xvalue = QString::fromUtf8(icalparameter_get_xvalue(p));
            if (xname == QLatin1String("X-CONTENT-DISPOSITION")) {
                attachment.setShowInline(xvalue.toLower() == QLatin1String("inline"));
            } else if (xname == QLatin1String("X-LABEL")) {
                attachment.setLabel(xvalue);
            } else if (xname == QLatin1String("X-KONTACT-TYPE")) {
                attachment.setLocal(xvalue.toLower() == QLatin1String("local"));
            }
            p = icalproperty_get_next_parameter(attach, ICAL_X_PARAMETER);
        }

        p = icalproperty_get_first_parameter(attach, ICAL_X_PARAMETER);
        while (p) {
            if (strncmp(icalparameter_get_xname(p), "X-LABEL", 7) == 0) {
                attachment.setLabel(QString::fromUtf8(icalparameter_get_xvalue(p)));
            }
            p = icalproperty_get_next_parameter(attach, ICAL_X_PARAMETER);
        }
    }

    return attachment;
}

void ICalFormatImpl::readIncidence(icalcomponent *parent, const Incidence::Ptr &incidence, const ICalTimeZoneCache *tzlist)
{
    d->readIncidenceBase(parent, incidence);

    icalproperty *p = icalcomponent_get_first_property(parent, ICAL_ANY_PROPERTY);

    const char *text;
    int intvalue, inttext;
    icaldurationtype icalduration;
    QDateTime kdt;
    QDateTime dtstamp;

    QStringList categories;

    while (p) {
        icalproperty_kind kind = icalproperty_isa(p);
        switch (kind) {
        case ICAL_CREATED_PROPERTY:
            incidence->setCreated(readICalDateTimeProperty(p, tzlist));
            break;

        case ICAL_DTSTAMP_PROPERTY:
            dtstamp = readICalDateTimeProperty(p, tzlist);
            break;

        case ICAL_SEQUENCE_PROPERTY: // sequence
            intvalue = icalproperty_get_sequence(p);
            incidence->setRevision(intvalue);
            break;

        case ICAL_LASTMODIFIED_PROPERTY: // last modification UTC date/time
            incidence->setLastModified(readICalDateTimeProperty(p, tzlist));
            break;

        case ICAL_DTSTART_PROPERTY: { // start date and time
            bool allDay = false;
            kdt = readICalDateTimeProperty(p, tzlist, false, &allDay);
            incidence->setDtStart(kdt);
            incidence->setAllDay(allDay);
            break;
        }

        case ICAL_DURATION_PROPERTY: // start date and time
            icalduration = icalproperty_get_duration(p);
            incidence->setDuration(readICalDuration(icalduration));
            break;

        case ICAL_DESCRIPTION_PROPERTY: { // description
            QString textStr = QString::fromUtf8(icalproperty_get_description(p));
            if (!textStr.isEmpty()) {
                QString valStr = QString::fromUtf8(icalproperty_get_parameter_as_string(p, "X-KDE-TEXTFORMAT"));
                if (!valStr.compare(QLatin1String("HTML"), Qt::CaseInsensitive)) {
                    incidence->setDescription(textStr, true);
                } else {
                    incidence->setDescription(textStr, false);
                }
            }
        } break;

        case ICAL_SUMMARY_PROPERTY: { // summary
            QString textStr = QString::fromUtf8(icalproperty_get_summary(p));
            if (!textStr.isEmpty()) {
                QString valStr = QString::fromUtf8(icalproperty_get_parameter_as_string(p, "X-KDE-TEXTFORMAT"));
                if (!valStr.compare(QLatin1String("HTML"), Qt::CaseInsensitive)) {
                    incidence->setSummary(textStr, true);
                } else {
                    incidence->setSummary(textStr, false);
                }
            }
        } break;

        case ICAL_LOCATION_PROPERTY: { // location
            if (!icalproperty_get_value(p)) {
                // Fix for #191472. This is a pre-crash guard in case libical was
                // compiled in superstrict mode (--enable-icalerrors-are-fatal)
                // TODO: pre-crash guard other property getters too.
                break;
            }
            QString textStr = QString::fromUtf8(icalproperty_get_location(p));
            if (!textStr.isEmpty()) {
                QString valStr = QString::fromUtf8(icalproperty_get_parameter_as_string(p, "X-KDE-TEXTFORMAT"));
                if (!valStr.compare(QLatin1String("HTML"), Qt::CaseInsensitive)) {
                    incidence->setLocation(textStr, true);
                } else {
                    incidence->setLocation(textStr, false);
                }
            }
        } break;

        case ICAL_STATUS_PROPERTY: { // status
            Incidence::Status stat;
            switch (icalproperty_get_status(p)) {
            case ICAL_STATUS_TENTATIVE:
                stat = Incidence::StatusTentative;
                break;
            case ICAL_STATUS_CONFIRMED:
                stat = Incidence::StatusConfirmed;
                break;
            case ICAL_STATUS_COMPLETED:
                stat = Incidence::StatusCompleted;
                break;
            case ICAL_STATUS_NEEDSACTION:
                stat = Incidence::StatusNeedsAction;
                break;
            case ICAL_STATUS_CANCELLED:
                stat = Incidence::StatusCanceled;
                break;
            case ICAL_STATUS_INPROCESS:
                stat = Incidence::StatusInProcess;
                break;
            case ICAL_STATUS_DRAFT:
                stat = Incidence::StatusDraft;
                break;
            case ICAL_STATUS_FINAL:
                stat = Incidence::StatusFinal;
                break;
            case ICAL_STATUS_X:
                incidence->setCustomStatus(QString::fromUtf8(icalvalue_get_x(icalproperty_get_value(p))));
                stat = Incidence::StatusX;
                break;
            case ICAL_STATUS_NONE:
            default:
                stat = Incidence::StatusNone;
                break;
            }
            if (stat != Incidence::StatusX) {
                incidence->setStatus(stat);
            }
            break;
        }

        case ICAL_GEO_PROPERTY: { // geo
            icalgeotype geo = icalproperty_get_geo(p);
            incidence->setGeoLatitude(geo.lat);
            incidence->setGeoLongitude(geo.lon);
            incidence->setHasGeo(true);
            break;
        }

        case ICAL_PRIORITY_PROPERTY: // priority
            intvalue = icalproperty_get_priority(p);
            if (d->mCompat) {
                intvalue = d->mCompat->fixPriority(intvalue);
            }
            incidence->setPriority(intvalue);
            break;

        case ICAL_CATEGORIES_PROPERTY: { // categories
            // We have always supported multiple CATEGORIES properties per component
            // even though the RFC seems to indicate only 1 is permitted.
            // We can't change that -- in order to retain backwards compatibility.
            text = icalproperty_get_categories(p);
            const QString val = QString::fromUtf8(text);
            const QStringList lstVal = val.split(QLatin1Char(','), QString::SkipEmptyParts);
            for (const QString &cat : lstVal) {
                // ensure no duplicates
                if (!categories.contains(cat)) {
                    categories.append(cat);
                }
            }
            break;
        }

        case ICAL_RECURRENCEID_PROPERTY: // recurrenceId
            kdt = readICalDateTimeProperty(p, tzlist);
            if (kdt.isValid()) {
                incidence->setRecurrenceId(kdt);
                const icalparameter *param = icalproperty_get_first_parameter(p, ICAL_RANGE_PARAMETER);
                if (param && icalparameter_get_range(param) == ICAL_RANGE_THISANDFUTURE) {
                    incidence->setThisAndFuture(true);
                } else {
                    // A workaround for a bug in libical (https://github.com/libical/libical/issues/185)
                    // If a recurrenceId has both tzid and range, both parameters end up in the tzid.
                    // This results in invalid tzid's like: "Europe/Berlin;RANGE=THISANDFUTURE"
                    const icalparameter *param = icalproperty_get_first_parameter(p, ICAL_TZID_PARAMETER);
                    QString tzid = QString::fromLatin1(icalparameter_get_tzid(param));
                    const QStringList parts = tzid.toLower().split(QLatin1Char(';'));
                    for (const QString &part : parts) {
                        if (part == QLatin1String("range=thisandfuture")) {
                            incidence->setThisAndFuture(true);
                            break;
                        }
                    }
                }
            }
            break;

        case ICAL_RRULE_PROPERTY:
            readRecurrenceRule(p, incidence);
            break;

        case ICAL_RDATE_PROPERTY: {
            bool allDay = false;
            kdt = readICalDateTimeProperty(p, tzlist, false, &allDay);
            if (kdt.isValid()) {
                if (allDay) {
                    incidence->recurrence()->addRDate(kdt.date());
                } else {
                    incidence->recurrence()->addRDateTime(kdt);
                }
            } else {
                // TODO: RDates as period are not yet implemented!
            }
            break;
        }

        case ICAL_EXRULE_PROPERTY:
            readExceptionRule(p, incidence);
            break;

        case ICAL_EXDATE_PROPERTY: {
            bool allDay = false;
            kdt = readICalDateTimeProperty(p, tzlist, false, &allDay);
            if (allDay) {
                incidence->recurrence()->addExDate(kdt.date());
            } else {
                incidence->recurrence()->addExDateTime(kdt);
            }
            break;
        }

        case ICAL_CLASS_PROPERTY:
            inttext = icalproperty_get_class(p);
            if (inttext == ICAL_CLASS_PUBLIC) {
                incidence->setSecrecy(Incidence::SecrecyPublic);
            } else if (inttext == ICAL_CLASS_CONFIDENTIAL) {
                incidence->setSecrecy(Incidence::SecrecyConfidential);
            } else {
                incidence->setSecrecy(Incidence::SecrecyPrivate);
            }
            break;

        case ICAL_ATTACH_PROPERTY: // attachments
            incidence->addAttachment(readAttachment(p));
            break;

        case ICAL_COLOR_PROPERTY:
            incidence->setColor(QString::fromUtf8(icalproperty_get_color(p)));
            break;

        default:
            // TODO: do something about unknown properties?
            break;
        }

        p = icalcomponent_get_next_property(parent, ICAL_ANY_PROPERTY);
    }

    // Set the scheduling ID
    const QString uid = incidence->customProperty("LIBKCAL", "ID");
    if (!uid.isNull()) {
        // The UID stored in incidencebase is actually the scheduling ID
        // It has to be stored in the iCal UID component for compatibility
        // with other iCal applications
        incidence->setSchedulingID(incidence->uid(), uid);
    }

    // Now that recurrence and exception stuff is completely set up,
    // do any backwards compatibility adjustments.
    if (incidence->recurs() && d->mCompat) {
        d->mCompat->fixRecurrence(incidence);
    }

    // add categories
    incidence->setCategories(categories);

    // iterate through all alarms
    for (icalcomponent *alarm = icalcomponent_get_first_component(parent, ICAL_VALARM_COMPONENT); alarm;
         alarm = icalcomponent_get_next_component(parent, ICAL_VALARM_COMPONENT)) {
        readAlarm(alarm, incidence);
    }

    // iterate through all conferences
    Conference::List conferences;
    for (auto *conf = icalcomponent_get_first_property(parent, ICAL_CONFERENCE_PROPERTY); conf;
         conf = icalcomponent_get_next_property(parent, ICAL_CONFERENCE_PROPERTY)) {
        conferences.push_back(readConference(conf));
    }
    incidence->setConferences(conferences);

    if (d->mCompat) {
        // Fix incorrect alarm settings by other applications (like outloook 9)
        d->mCompat->fixAlarms(incidence);
        d->mCompat->setCreatedToDtStamp(incidence, dtstamp);
    }
}

//@cond PRIVATE
void ICalFormatImpl::Private::readIncidenceBase(icalcomponent *parent, const IncidenceBase::Ptr &incidenceBase)
{
    icalproperty *p = icalcomponent_get_first_property(parent, ICAL_ANY_PROPERTY);
    bool uidProcessed = false;
    while (p) {
        icalproperty_kind kind = icalproperty_isa(p);
        switch (kind) {
        case ICAL_UID_PROPERTY: // unique id
            uidProcessed = true;
            incidenceBase->setUid(QString::fromUtf8(icalproperty_get_uid(p)));
            break;

        case ICAL_ORGANIZER_PROPERTY: // organizer
            incidenceBase->setOrganizer(mImpl->readOrganizer(p));
            break;

        case ICAL_ATTENDEE_PROPERTY: // attendee
            incidenceBase->addAttendee(mImpl->readAttendee(p));
            break;

        case ICAL_COMMENT_PROPERTY:
            incidenceBase->addComment(QString::fromUtf8(icalproperty_get_comment(p)));
            break;

        case ICAL_CONTACT_PROPERTY:
            incidenceBase->addContact(QString::fromUtf8(icalproperty_get_contact(p)));
            break;

        case ICAL_URL_PROPERTY:
            incidenceBase->setUrl(QUrl(QString::fromUtf8(icalproperty_get_url(p))));
            break;

        default:
            break;
        }

        p = icalcomponent_get_next_property(parent, ICAL_ANY_PROPERTY);
    }

    if (!uidProcessed) {
        qWarning() << "The incidence didn't have any UID! Report a bug "
                   << "to the application that generated this file.";

        // Our in-memory incidence has a random uid generated in Event's ctor.
        // Make it empty so it matches what's in the file:
        incidenceBase->setUid(QString());

        // Otherwise, next time we read the file, this function will return
        // an event with another random uid and we will have two events in the calendar.
    }

    // custom properties
    readCustomProperties(parent, incidenceBase.data());
}

void ICalFormatImpl::Private::readCustomProperties(icalcomponent *parent, CustomProperties *properties)
{
    QByteArray property;
    QString value, parameters;
    icalproperty *p = icalcomponent_get_first_property(parent, ICAL_X_PROPERTY);
    icalparameter *param = nullptr;

    while (p) {
        QString nvalue = QString::fromUtf8(icalproperty_get_x(p));
        if (nvalue.isEmpty()) {
            icalvalue *value = icalproperty_get_value(p);
            if (icalvalue_isa(value) == ICAL_TEXT_VALUE) {
                // Calling icalvalue_get_text( value ) on a datetime value crashes.
                nvalue = QString::fromUtf8(icalvalue_get_text(value));
            } else {
                p = icalcomponent_get_next_property(parent, ICAL_X_PROPERTY);
                continue;
            }
        }
        const char *name = icalproperty_get_x_name(p);
        QByteArray nproperty(name);
        if (property != nproperty) {
            // New property
            if (!property.isEmpty()) {
                properties->setNonKDECustomProperty(property, value, parameters);
            }
            property = name;
            value = nvalue;
            QStringList parametervalues;
            for (param = icalproperty_get_first_parameter(p, ICAL_ANY_PARAMETER); param; param = icalproperty_get_next_parameter(p, ICAL_ANY_PARAMETER)) {
                // 'c' is owned by ical library => all we need to do is just use it
                const char *c = icalparameter_as_ical_string(param);
                parametervalues.push_back(QLatin1String(c));
            }
            parameters = parametervalues.join(QLatin1Char(';'));
        } else {
            value = value.append(QLatin1Char(',')).append(nvalue);
        }
        p = icalcomponent_get_next_property(parent, ICAL_X_PROPERTY);
    }
    if (!property.isEmpty()) {
        properties->setNonKDECustomProperty(property, value, parameters);
    }
}
//@endcond

void ICalFormatImpl::readRecurrenceRule(icalproperty *rrule, const Incidence::Ptr &incidence)
{
    Recurrence *recur = incidence->recurrence();

    struct icalrecurrencetype r = icalproperty_get_rrule(rrule);
    // dumpIcalRecurrence(r);

    RecurrenceRule *recurrule = new RecurrenceRule(/*incidence*/);
    recurrule->setStartDt(incidence->dtStart());
    readRecurrence(r, recurrule);
    recur->addRRule(recurrule);
}

void ICalFormatImpl::readExceptionRule(icalproperty *rrule, const Incidence::Ptr &incidence)
{
    struct icalrecurrencetype r = icalproperty_get_exrule(rrule);
    // dumpIcalRecurrence(r);

    RecurrenceRule *recurrule = new RecurrenceRule(/*incidence*/);
    recurrule->setStartDt(incidence->dtStart());
    readRecurrence(r, recurrule);

    Recurrence *recur = incidence->recurrence();
    recur->addExRule(recurrule);
}

void ICalFormatImpl::readRecurrence(const struct icalrecurrencetype &r, RecurrenceRule *recur)
{
    // Generate the RRULE string
    recur->setRRule(QLatin1String(icalrecurrencetype_as_string(const_cast<struct icalrecurrencetype *>(&r))));
    // Period
    switch (r.freq) {
    case ICAL_SECONDLY_RECURRENCE:
        recur->setRecurrenceType(RecurrenceRule::rSecondly);
        break;
    case ICAL_MINUTELY_RECURRENCE:
        recur->setRecurrenceType(RecurrenceRule::rMinutely);
        break;
    case ICAL_HOURLY_RECURRENCE:
        recur->setRecurrenceType(RecurrenceRule::rHourly);
        break;
    case ICAL_DAILY_RECURRENCE:
        recur->setRecurrenceType(RecurrenceRule::rDaily);
        break;
    case ICAL_WEEKLY_RECURRENCE:
        recur->setRecurrenceType(RecurrenceRule::rWeekly);
        break;
    case ICAL_MONTHLY_RECURRENCE:
        recur->setRecurrenceType(RecurrenceRule::rMonthly);
        break;
    case ICAL_YEARLY_RECURRENCE:
        recur->setRecurrenceType(RecurrenceRule::rYearly);
        break;
    case ICAL_NO_RECURRENCE:
    default:
        recur->setRecurrenceType(RecurrenceRule::rNone);
    }
    // Frequency
    recur->setFrequency(r.interval);

    // Duration & End Date
    if (!icaltime_is_null_time(r.until)) {
        icaltimetype t = r.until;
        //结束时间不切换零时区
        recur->setEndDt(readICalDateTime(nullptr, t, nullptr, false));
    } else {
        if (r.count == 0) {
            recur->setDuration(-1);
        } else {
            recur->setDuration(r.count);
        }
    }

    // Week start setting
    short wkst = static_cast<short>((r.week_start + 5) % 7 + 1);
    recur->setWeekStart(wkst);

    // And now all BY*
    QList<int> lst;
    int i;
    int index = 0;

// clang-format off
//@cond PRIVATE
#define readSetByList( rrulecomp, setfunc )                             \
    index = 0;                                                            \
    lst.clear();                                                          \
    while ( ( i = r.rrulecomp[index++] ) != ICAL_RECURRENCE_ARRAY_MAX ) { \
        lst.append( i );                                                    \
    }                                                                     \
    if ( !lst.isEmpty() ) {                                               \
        recur->setfunc( lst );                                              \
    }
//@endcond
    // clang-format on

    // BYSECOND, MINUTE and HOUR, MONTHDAY, YEARDAY, WEEKNUMBER, MONTH
    // and SETPOS are standard int lists, so we can treat them with the
    // same macro
    readSetByList(by_second, setBySeconds);
    readSetByList(by_minute, setByMinutes);
    readSetByList(by_hour, setByHours);
    readSetByList(by_month_day, setByMonthDays);
    readSetByList(by_year_day, setByYearDays);
    readSetByList(by_week_no, setByWeekNumbers);
    readSetByList(by_month, setByMonths);
    readSetByList(by_set_pos, setBySetPos);
#undef readSetByList

    // BYDAY is a special case, since it's not an int list
    QList<RecurrenceRule::WDayPos> wdlst;
    short day;
    index = 0;
    while ((day = r.by_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
        RecurrenceRule::WDayPos pos;
        pos.setDay(static_cast<short>((icalrecurrencetype_day_day_of_week(day) + 5) % 7 + 1));
        pos.setPos(icalrecurrencetype_day_position(day));
        wdlst.append(pos);
    }
    if (!wdlst.isEmpty()) {
        recur->setByDays(wdlst);
    }

    // TODO: Store all X- fields of the RRULE inside the recurrence (so they are
    // preserved
}

void ICalFormatImpl::readAlarm(icalcomponent *alarm, const Incidence::Ptr &incidence)
{
    Alarm::Ptr ialarm = incidence->newAlarm();
    ialarm->setRepeatCount(0);
    ialarm->setEnabled(true);

    // Determine the alarm's action type
    icalproperty *p = icalcomponent_get_first_property(alarm, ICAL_ACTION_PROPERTY);
    Alarm::Type type = Alarm::Display;
    icalproperty_action action = ICAL_ACTION_DISPLAY;
    if (!p) {
        qDebug() << "Unknown type of alarm, using default";
        // TODO: do something about unknown alarm type?
    } else {
        action = icalproperty_get_action(p);
        switch (action) {
        case ICAL_ACTION_DISPLAY:
            type = Alarm::Display;
            break;
        case ICAL_ACTION_AUDIO:
            type = Alarm::Audio;
            break;
        case ICAL_ACTION_PROCEDURE:
            type = Alarm::Procedure;
            break;
        case ICAL_ACTION_EMAIL:
            type = Alarm::Email;
            break;
        default:
            break;
            // TODO: do something about invalid alarm type?
        }
    }
    ialarm->setType(type);

    p = icalcomponent_get_first_property(alarm, ICAL_ANY_PROPERTY);
    while (p) {
        icalproperty_kind kind = icalproperty_isa(p);

        switch (kind) {
        case ICAL_TRIGGER_PROPERTY: {
            icaltriggertype trigger = icalproperty_get_trigger(p);
            if (!icaltime_is_null_time(trigger.time)) {
                // set the trigger to a specific time (which is not in rfc2445, btw)
                ialarm->setTime(readICalUtcDateTime(p, trigger.time));
            } else {
                // set the trigger to an offset from the incidence start or end time.
                if (!icaldurationtype_is_bad_duration(trigger.duration)) {
                    Duration duration(readICalDuration(trigger.duration));
                    icalparameter *param = icalproperty_get_first_parameter(p, ICAL_RELATED_PARAMETER);
                    if (param && icalparameter_get_related(param) == ICAL_RELATED_END) {
                        ialarm->setEndOffset(duration);
                    } else {
                        ialarm->setStartOffset(duration);
                    }
                } else {
                    // a bad duration was encountered, just set a 0 duration from start
                    ialarm->setStartOffset(Duration(0));
                }
            }
            break;
        }
        case ICAL_DURATION_PROPERTY: {
            icaldurationtype duration = icalproperty_get_duration(p);
            ialarm->setSnoozeTime(readICalDuration(duration));
            break;
        }
        case ICAL_REPEAT_PROPERTY:
            ialarm->setRepeatCount(icalproperty_get_repeat(p));
            break;

        case ICAL_DESCRIPTION_PROPERTY: {
            // Only in DISPLAY and EMAIL and PROCEDURE alarms
            QString description = QString::fromUtf8(icalproperty_get_description(p));
            switch (action) {
            case ICAL_ACTION_DISPLAY:
                ialarm->setText(description);
                break;
            case ICAL_ACTION_PROCEDURE:
                ialarm->setProgramArguments(description);
                break;
            case ICAL_ACTION_EMAIL:
                ialarm->setMailText(description);
                break;
            default:
                break;
            }
            break;
        }
        case ICAL_SUMMARY_PROPERTY:
            // Only in EMAIL alarm
            ialarm->setMailSubject(QString::fromUtf8(icalproperty_get_summary(p)));
            break;

        case ICAL_ATTENDEE_PROPERTY: {
            // Only in EMAIL alarm
            QString email = QString::fromUtf8(icalproperty_get_attendee(p));
            if (email.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
                email.remove(0, 7);
            }
            QString name;
            icalparameter *param = icalproperty_get_first_parameter(p, ICAL_CN_PARAMETER);
            if (param) {
                name = QString::fromUtf8(icalparameter_get_cn(param));
            }
            ialarm->addMailAddress(Person(name, email));
            break;
        }

        case ICAL_ATTACH_PROPERTY: {
            // Only in AUDIO and EMAIL and PROCEDURE alarms
            Attachment attach = readAttachment(p);
            if (!attach.isEmpty() && attach.isUri()) {
                switch (action) {
                case ICAL_ACTION_AUDIO:
                    ialarm->setAudioFile(attach.uri());
                    break;
                case ICAL_ACTION_PROCEDURE:
                    ialarm->setProgramFile(attach.uri());
                    break;
                case ICAL_ACTION_EMAIL:
                    ialarm->addMailAttachment(attach.uri());
                    break;
                default:
                    break;
                }
            } else {
                qDebug() << "Alarm attachments currently only support URIs,"
                         << "but no binary data";
            }
            break;
        }
        default:
            break;
        }
        p = icalcomponent_get_next_property(alarm, ICAL_ANY_PROPERTY);
    }

    // custom properties
    d->readCustomProperties(alarm, ialarm.data());

    QString locationRadius = ialarm->nonKDECustomProperty("X-LOCATION-RADIUS");
    if (!locationRadius.isEmpty()) {
        ialarm->setLocationRadius(locationRadius.toInt());
        ialarm->setHasLocationRadius(true);
    }

    if (ialarm->customProperty(APP_NAME_FOR_XPROPERTIES, ENABLED_ALARM_XPROPERTY) == QLatin1String("FALSE")) {
        ialarm->setEnabled(false);
    }
    // TODO: check for consistency of alarm properties
}

icaldatetimeperiodtype ICalFormatImpl::writeICalDatePeriod(const QDate &date)
{
    icaldatetimeperiodtype t;
    t.time = writeICalDate(date);
    t.period = icalperiodtype_null_period();
    return t;
}

Conference ICalFormatImpl::readConference(icalproperty *prop)
{
    Conference conf;
    conf.setUri(QUrl(QString::fromUtf8(icalproperty_get_conference(prop))));
    conf.setLabel(QString::fromUtf8(icalproperty_get_parameter_as_string(prop, "LABEL")));
    conf.setFeatures(QString::fromUtf8(icalproperty_get_parameter_as_string(prop, "FEATURE")).split(QLatin1Char(',')));
    conf.setLanguage(QString::fromUtf8(icalproperty_get_parameter_as_string(prop, "LANGUAGE")));
    return conf;
}

icaltimetype ICalFormatImpl::writeICalDate(const QDate &date)
{
    icaltimetype t = icaltime_null_time();

    t.year = date.year();
    t.month = date.month();
    t.day = date.day();

    t.hour = 0;
    t.minute = 0;
    t.second = 0;

    t.is_date = 1;
    t.zone = nullptr;

    return t;
}

icaltimetype ICalFormatImpl::writeICalDateTime(const QDateTime &datetime, bool dateOnly)
{
    icaltimetype t = icaltime_null_time();

    t.year = datetime.date().year();
    t.month = datetime.date().month();
    t.day = datetime.date().day();

    t.is_date = dateOnly;

    if (!t.is_date) {
        t.hour = datetime.time().hour();
        t.minute = datetime.time().minute();
        t.second = datetime.time().second();
    }
    t.zone = nullptr; // zone is NOT set
    if (datetime.timeSpec() == Qt::UTC || (datetime.timeSpec() == Qt::TimeZone && datetime.timeZone() == QTimeZone::utc()) || (datetime.timeSpec() == Qt::OffsetFromUTC && datetime.offsetFromUtc() == 0)) {
        t = icaltime_convert_to_zone(t, icaltimezone_get_utc_timezone());
    }
    return t;
}

icalproperty *ICalFormatImpl::writeICalDateTimeProperty(const icalproperty_kind type, const QDateTime &dt, TimeZoneList *tzUsedList)
{
    icaltimetype t;

    switch (type) {
    case ICAL_DTSTAMP_PROPERTY:
    case ICAL_CREATED_PROPERTY:
    case ICAL_LASTMODIFIED_PROPERTY:
        t = writeICalDateTime(dt.toUTC());
        break;
    default:
        t = writeICalDateTime(dt);
        break;
    }

    icalproperty *p;
    switch (type) {
    case ICAL_DTSTAMP_PROPERTY:
        p = icalproperty_new_dtstamp(t);
        break;
    case ICAL_CREATED_PROPERTY:
        p = icalproperty_new_created(t);
        break;
    case ICAL_LASTMODIFIED_PROPERTY:
        p = icalproperty_new_lastmodified(t);
        break;
    case ICAL_DTSTART_PROPERTY: // start date and time
        p = icalproperty_new_dtstart(t);
        break;
    case ICAL_DTEND_PROPERTY: // end date and time
        p = icalproperty_new_dtend(t);
        break;
    case ICAL_DUE_PROPERTY:
        p = icalproperty_new_due(t);
        break;
    case ICAL_RECURRENCEID_PROPERTY:
        p = icalproperty_new_recurrenceid(t);
        break;
    case ICAL_EXDATE_PROPERTY:
        p = icalproperty_new_exdate(t);
        break;
    case ICAL_X_PROPERTY: {
        p = icalproperty_new_x("");
        icaltimetype timeType = writeICalDateTime(dt);
        icalvalue *text = icalvalue_new_datetime(timeType);
        icalproperty_set_value(p, text);
    } break;
    default: {
        icaldatetimeperiodtype tp;
        tp.time = t;
        tp.period = icalperiodtype_null_period();
        switch (type) {
        case ICAL_RDATE_PROPERTY:
            p = icalproperty_new_rdate(tp);
            break;
        default:
            return nullptr;
        }
    }
    }

    QTimeZone qtz;
    if (!icaltime_is_utc(t)) {
        qtz = dt.timeZone();
    }

    if (qtz.isValid()) {
        if (tzUsedList) {
            if (!tzUsedList->contains(qtz)) {
                tzUsedList->push_back(qtz);
            }
        }

        icalproperty_add_parameter(p, icalparameter_new_tzid(qtz.id().constData()));
    }
    return p;
}

QDateTime ICalFormatImpl::readICalDateTime(icalproperty *p, const icaltimetype &t, const ICalTimeZoneCache *tzCache, bool utc)
{
    //  qDebug();
    //  _dumpIcaltime( t );

    QTimeZone timeZone;
    if (icaltime_is_utc(t) || t.zone == icaltimezone_get_utc_timezone()) {
        timeZone = QTimeZone::utc(); // the time zone is UTC
        utc = false; // no need to convert to UTC
    } else {
        icalparameter *param = p ? icalproperty_get_first_parameter(p, ICAL_TZID_PARAMETER) : nullptr;
        QByteArray tzid = param ? QByteArray(icalparameter_get_tzid(param)) : QByteArray();

        // A workaround for a bug in libical (https://github.com/libical/libical/issues/185)
        // If a recurrenceId has both tzid and range, both parameters end up in the tzid.
        // This results in invalid tzid's like: "Europe/Berlin;RANGE=THISANDFUTURE"
        QStringList parts = QString::fromLatin1(tzid).split(QLatin1Char(';'));
        if (parts.count() > 1) {
            tzid = parts.first().toLatin1();
        }

        if (tzCache) {
            // First try to get the timezone from cache
            timeZone = tzCache->tzForTime(QDateTime({t.year, t.month, t.day}, {}), tzid);
        }
        if (!timeZone.isValid()) {
            // Fallback to trying to match against Qt timezone
            timeZone = QTimeZone(tzid);
        }
        if (!timeZone.isValid()) {
            // Finally, give up and assume local timezone
            timeZone = QTimeZone::systemTimeZone();
        }
    }
    QDateTime result;
    if (t.is_date) {
        result = QDateTime(QDate(t.year, t.month, t.day), {}, timeZone);
    } else {
        result = QDateTime(QDate(t.year, t.month, t.day), QTime(t.hour, t.minute, t.second), timeZone);
    }
    return utc ? result.toUTC() : result;
}

QDate ICalFormatImpl::readICalDate(const icaltimetype &t)
{
    return QDate(t.year, t.month, t.day);
}

QDateTime ICalFormatImpl::readICalDateTimeProperty(icalproperty *p, const ICalTimeZoneCache *tzList, bool utc, bool *allDay)
{
    icaldatetimeperiodtype tp;
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {
    case ICAL_CREATED_PROPERTY: // UTC date/time
        tp.time = icalproperty_get_created(p);
        utc = true;
        break;
    case ICAL_DTSTAMP_PROPERTY: // UTC date/time
        tp.time = icalproperty_get_dtstamp(p);
        utc = true;
        break;
    case ICAL_LASTMODIFIED_PROPERTY: // last modification UTC date/time
        tp.time = icalproperty_get_lastmodified(p);
        utc = true;
        break;
    case ICAL_DTSTART_PROPERTY: // start date and time (UTC for freebusy)
        tp.time = icalproperty_get_dtstart(p);
        break;
    case ICAL_DTEND_PROPERTY: // end date and time (UTC for freebusy)
        tp.time = icalproperty_get_dtend(p);
        break;
    case ICAL_DUE_PROPERTY: // due date/time
        tp.time = icalproperty_get_due(p);
        break;
    case ICAL_COMPLETED_PROPERTY: // UTC completion date/time
        tp.time = icalproperty_get_completed(p);
        utc = true;
        break;
    case ICAL_RECURRENCEID_PROPERTY:
        tp.time = icalproperty_get_recurrenceid(p);
        break;
    case ICAL_EXDATE_PROPERTY:
        tp.time = icalproperty_get_exdate(p);
        break;
    case ICAL_X_PROPERTY: {
        const char *name = icalproperty_get_x_name(p);
        if (QLatin1String(name) == QLatin1String("X-KDE-LIBKCAL-DTRECURRENCE")) {
            const char *value = icalvalue_as_ical_string(icalproperty_get_value(p));
            icalvalue *v = icalvalue_new_from_string(ICAL_DATETIME_VALUE, value);
            tp.time = icalvalue_get_datetime(v);
            icalvalue_free(v);
            break;
        }
    } // end of ICAL_X_PROPERTY
        Q_FALLTHROUGH();
    default:
        switch (kind) {
        case ICAL_RDATE_PROPERTY:
            tp = icalproperty_get_rdate(p);
            break;
        default:
            return QDateTime();
        }
        if (!icaltime_is_valid_time(tp.time)) {
            return QDateTime(); // a time period was found (not implemented yet)
        }
        break;
    }

    if (allDay) {
        *allDay = tp.time.is_date;
    }

    if (tp.time.is_date) {
        return QDateTime(readICalDate(tp.time), QTime());
    } else {
        return readICalDateTime(p, tp.time, tzList, utc);
    }
}

icaldurationtype ICalFormatImpl::writeICalDuration(const Duration &duration)
{
    // should be able to use icaldurationtype_from_int(), except we know
    // that some older tools do not properly support weeks. So we never
    // set a week duration, only days

    icaldurationtype d;

    int value = duration.value();
    d.is_neg = (value < 0) ? 1 : 0;
    if (value < 0) {
        value = -value;
    }
    // RFC2445 states that an ical duration value must be
    // EITHER weeks OR days/time, not both.
    if (duration.isDaily()) {
        if (!(value % 7)) {
            d.weeks = value / 7;
            d.days = 0;
        } else {
            d.weeks = 0;
            d.days = value;
        }
        d.hours = d.minutes = d.seconds = 0;
    } else {
        if (!(value % gSecondsPerWeek)) {
            d.weeks = value / gSecondsPerWeek;
            d.days = d.hours = d.minutes = d.seconds = 0;
        } else {
            d.weeks = 0;
            d.days = value / gSecondsPerDay;
            value %= gSecondsPerDay;
            d.hours = value / gSecondsPerHour;
            value %= gSecondsPerHour;
            d.minutes = value / gSecondsPerMinute;
            value %= gSecondsPerMinute;
            d.seconds = value;
        }
    }

    return d;
}

Duration ICalFormatImpl::readICalDuration(const icaldurationtype &d)
{
    int days = d.weeks * 7;
    days += d.days;
    int seconds = d.hours * gSecondsPerHour;
    seconds += d.minutes * gSecondsPerMinute;
    seconds += d.seconds;
    if (seconds || !days) { // Create second-type duration for 0 delay durations.
        seconds += days * gSecondsPerDay;
        if (d.is_neg) {
            seconds = -seconds;
        }
        return Duration(seconds, Duration::Seconds);
    } else {
        if (d.is_neg) {
            days = -days;
        }
        return Duration(days, Duration::Days);
    }
}

icalcomponent *ICalFormatImpl::createCalendarComponent(const Calendar::Ptr &cal)
{
    icalcomponent *calendar;

    // Root component
    calendar = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

    // Product Identifier
    icalproperty *p = icalproperty_new_prodid(CalFormat::productId().toUtf8().constData());
    icalcomponent_add_property(calendar, p);

    // iCalendar version (2.0)
    p = icalproperty_new_version(const_cast<char *>(_ICAL_VERSION));
    icalcomponent_add_property(calendar, p);

    // Implementation Version
    p = icalproperty_new_x(_ICAL_IMPLEMENTATION_VERSION);
    icalproperty_set_x_name(p, IMPLEMENTATION_VERSION_XPROPERTY);
    icalcomponent_add_property(calendar, p);

    // Add time zone
    // NOTE: Commented out since relevant timezones are added by the caller.
    // Previously we got some timezones listed twice in the ical file.
    /*
    if ( cal && cal->timeZones() ) {
      const ICalTimeZones::ZoneMap zmaps = cal->timeZones()->zones();
      for ( ICalTimeZones::ZoneMap::ConstIterator it=zmaps.constBegin();
            it != zmaps.constEnd(); ++it ) {
        icaltimezone *icaltz = (*it).icalTimezone();
        if ( !icaltz ) {
          qCritical() << "bad time zone";
        } else {
          icalcomponent *tz = icalcomponent_new_clone( icaltimezone_get_component( icaltz ) );
          icalcomponent_add_component( calendar, tz );
          icaltimezone_free( icaltz, 1 );
        }
      }
    }
    */
    // Custom properties
    if (cal != nullptr) {
        d->writeCustomProperties(calendar, cal.data());
    }

    return calendar;
}

Incidence::Ptr ICalFormatImpl::readOneIncidence(icalcomponent *calendar, const ICalTimeZoneCache *tzlist)
{
    if (!calendar) {
        qWarning() << "Populate called with empty calendar";
        return Incidence::Ptr();
    }
    icalcomponent *c = icalcomponent_get_first_component(calendar, ICAL_VEVENT_COMPONENT);
    if (c) {
        return readEvent(c, tzlist);
    }
    c = icalcomponent_get_first_component(calendar, ICAL_VTODO_COMPONENT);
    if (c) {
        return readTodo(c, tzlist);
    }
    c = icalcomponent_get_first_component(calendar, ICAL_VJOURNAL_COMPONENT);
    if (c) {
        return readJournal(c, tzlist);
    }
    qWarning() << "Found no incidence";
    return Incidence::Ptr();
}

// take a raw vcalendar (i.e. from a file on disk, clipboard, etc. etc.
// and break it down from its tree-like format into the dictionary format
// that is used internally in the ICalFormatImpl.
//获取原始vcalendar（即，从磁盘上的文件、剪贴板等），并将其从树状格式分解为ICalFormatImpl内部使用的字典格式。
bool ICalFormatImpl::populate(const Calendar::Ptr &cal, icalcomponent *calendar, bool deleted, const QString &notebook)
{
    Q_UNUSED(notebook);

    // qDebug()<<"Populate called";

    // this function will populate the caldict dictionary and other event
    // lists. It turns vevents into Events and then inserts them.
    //

    if (!calendar) {
        qWarning() << "Populate called with empty calendar";
        return false;
    }

    // TODO: check for METHOD

    icalproperty *p = icalcomponent_get_first_property(calendar, ICAL_X_PROPERTY);
    QString implementationVersion;

    while (p) {
        const char *name = icalproperty_get_x_name(p);
        QByteArray nproperty(name);
        if (nproperty == QByteArray(IMPLEMENTATION_VERSION_XPROPERTY)) {
            QString nvalue = QString::fromUtf8(icalproperty_get_x(p));
            if (nvalue.isEmpty()) {
                icalvalue *value = icalproperty_get_value(p);
                if (icalvalue_isa(value) == ICAL_TEXT_VALUE) {
                    nvalue = QString::fromUtf8(icalvalue_get_text(value));
                }
            }
            implementationVersion = nvalue;
            icalcomponent_remove_property(calendar, p);
            icalproperty_free(p);
        }
        p = icalcomponent_get_next_property(calendar, ICAL_X_PROPERTY);
    }

    p = icalcomponent_get_first_property(calendar, ICAL_PRODID_PROPERTY);
    if (!p) {
        qDebug() << "No PRODID property found";
        d->mLoadedProductId.clear();
    } else {
        d->mLoadedProductId = QString::fromUtf8(icalproperty_get_prodid(p));

        delete d->mCompat;
        d->mCompat = CompatFactory::createCompat(d->mLoadedProductId, implementationVersion);
    }

    p = icalcomponent_get_first_property(calendar, ICAL_VERSION_PROPERTY);
    if (!p) {
        qDebug() << "No VERSION property found";
        d->mParent->setException(new Exception(Exception::CalVersionUnknown));
        return false;
    } else {
        const char *version = icalproperty_get_version(p);
        if (!version) {
            qDebug() << "No VERSION property found";
            d->mParent->setException(new Exception(Exception::VersionPropertyMissing));

            return false;
        }
        if (strcmp(version, "1.0") == 0) {
            qDebug() << "Expected iCalendar, got vCalendar";
            d->mParent->setException(new Exception(Exception::CalVersion1));
            return false;
        } else if (strcmp(version, "2.0") != 0) {
            qDebug() << "Expected iCalendar, got unknown format";
            d->mParent->setException(new Exception(Exception::CalVersionUnknown));
            return false;
        }
    }

    // Populate the calendar's time zone collection with all VTIMEZONE components
    ICalTimeZoneCache timeZoneCache;
    ICalTimeZoneParser parser(&timeZoneCache);
    parser.parse(calendar);

    // custom properties
    d->readCustomProperties(calendar, cal.data());

    // Store all events with a relatedTo property in a list for post-processing
    d->mEventsRelate.clear();
    d->mTodosRelate.clear();
    // TODO: make sure that only actually added events go to this lists.

    icalcomponent *c = icalcomponent_get_first_component(calendar, ICAL_VTODO_COMPONENT);
    while (c) {
        Todo::Ptr todo = readTodo(c, &timeZoneCache);
        if (todo) {
            // qDebug() << "todo is not zero and deleted is " << deleted;
            Todo::Ptr old = cal->todo(todo->uid(), todo->recurrenceId());
            if (old) {
                if (old->uid().isEmpty()) {
                    qWarning() << "Skipping invalid VTODO";
                    c = icalcomponent_get_next_component(calendar, ICAL_VTODO_COMPONENT);
                    continue;
                }
                // qDebug() << "Found an old todo with uid " << old->uid();
                if (deleted) {
                    // qDebug() << "Todo " << todo->uid() << " already deleted";
                    cal->deleteTodo(old); // move old to deleted
                    removeAllICal(d->mTodosRelate, old);
                } else if (todo->revision() > old->revision()) {
                    // qDebug() << "Replacing old todo " << old.data() << " with this one " << todo.data();
                    cal->deleteTodo(old); // move old to deleted
                    removeAllICal(d->mTodosRelate, old);
                    cal->addTodo(todo); // and replace it with this one
                }
            } else if (deleted) {
                // qDebug() << "Todo " << todo->uid() << " already deleted";
                old = cal->deletedTodo(todo->uid(), todo->recurrenceId());
                if (!old) {
                    cal->addTodo(todo); // add this one
                    cal->deleteTodo(todo); // and move it to deleted
                }
            } else {
                // qDebug() << "Adding todo " << todo.data() << todo->uid();
                cal->addTodo(todo); // just add this one
            }
        }
        c = icalcomponent_get_next_component(calendar, ICAL_VTODO_COMPONENT);
    }

    // Iterate through all events
    c = icalcomponent_get_first_component(calendar, ICAL_VEVENT_COMPONENT);
    while (c) {
        Event::Ptr event = readEvent(c, &timeZoneCache);
        if (event) {
            // qDebug() << "event is not zero and deleted is " << deleted;
            Event::Ptr old = cal->event(event->uid(), event->recurrenceId());
            if (old) {
                if (old->uid().isEmpty()) {
                    qWarning() << "Skipping invalid VEVENT";
                    c = icalcomponent_get_next_component(calendar, ICAL_VEVENT_COMPONENT);
                    continue;
                }
                // qDebug() << "Found an old event with uid " << old->uid();
                if (deleted) {
                    // qDebug() << "Event " << event->uid() << " already deleted";
                    cal->deleteEvent(old); // move old to deleted
                    removeAllICal(d->mEventsRelate, old);
                } else if (event->revision() > old->revision()) {
                    // qDebug() << "Replacing old event " << old.data()
                    //                       << " with this one " << event.data();
                    cal->deleteEvent(old); // move old to deleted
                    removeAllICal(d->mEventsRelate, old);
                    cal->addEvent(event); // and replace it with this one
                }
            } else if (deleted) {
                // qDebug() << "Event " << event->uid() << " already deleted";
                old = cal->deletedEvent(event->uid(), event->recurrenceId());
                if (!old) {
                    cal->addEvent(event); // add this one
                    cal->deleteEvent(event); // and move it to deleted
                }
            } else {
                // qDebug() << "Adding event " << event.data() << event->uid();
                cal->addEvent(event); // just add this one
            }
        }
        c = icalcomponent_get_next_component(calendar, ICAL_VEVENT_COMPONENT);
    }

    // Iterate through all journals
    c = icalcomponent_get_first_component(calendar, ICAL_VJOURNAL_COMPONENT);
    while (c) {
        Journal::Ptr journal = readJournal(c, &timeZoneCache);
        if (journal) {
            Journal::Ptr old = cal->journal(journal->uid(), journal->recurrenceId());
            if (old) {
                if (deleted) {
                    cal->deleteJournal(old); // move old to deleted
                } else if (journal->revision() > old->revision()) {
                    cal->deleteJournal(old); // move old to deleted
                    cal->addJournal(journal); // and replace it with this one
                }
            } else if (deleted) {
                old = cal->deletedJournal(journal->uid(), journal->recurrenceId());
                if (!old) {
                    cal->addJournal(journal); // add this one
                    cal->deleteJournal(journal); // and move it to deleted
                }
            } else {
                cal->addJournal(journal); // just add this one
            }
        }
        c = icalcomponent_get_next_component(calendar, ICAL_VJOURNAL_COMPONENT);
    }

    // TODO: Remove any previous time zones no longer referenced in the calendar

    return true;
}

QString ICalFormatImpl::extractErrorProperty(icalcomponent *c)
{
    QString errorMessage;

    icalproperty *error = icalcomponent_get_first_property(c, ICAL_XLICERROR_PROPERTY);
    while (error) {
        errorMessage += QLatin1String(icalproperty_get_xlicerror(error));
        errorMessage += QLatin1Char('\n');
        error = icalcomponent_get_next_property(c, ICAL_XLICERROR_PROPERTY);
    }

    return errorMessage;
}

/*
void ICalFormatImpl::dumpIcalRecurrence( const icalrecurrencetype &r )
{
  int i;

  qDebug() << " Freq:" << int( r.freq );
  qDebug() << " Until:" << icaltime_as_ical_string( r.until );
  qDebug() << " Count:" << r.count;
  if ( r.by_day[0] != ICAL_RECURRENCE_ARRAY_MAX ) {
    int index = 0;
    QString out = " By Day: ";
    while ( ( i = r.by_day[index++] ) != ICAL_RECURRENCE_ARRAY_MAX ) {
      out.append( QString::number( i ) + ' ' );
    }
    qDebug() << out;
  }
  if ( r.by_month_day[0] != ICAL_RECURRENCE_ARRAY_MAX ) {
    int index = 0;
    QString out = " By Month Day: ";
    while ( ( i = r.by_month_day[index++] ) != ICAL_RECURRENCE_ARRAY_MAX ) {
      out.append( QString::number( i ) + ' ' );
    }
    qDebug() << out;
  }
  if ( r.by_year_day[0] != ICAL_RECURRENCE_ARRAY_MAX ) {
    int index = 0;
    QString out = " By Year Day: ";
    while ( ( i = r.by_year_day[index++] ) != ICAL_RECURRENCE_ARRAY_MAX ) {
      out.append( QString::number( i ) + ' ' );
    }
    qDebug() << out;
  }
  if ( r.by_month[0] != ICAL_RECURRENCE_ARRAY_MAX ) {
    int index = 0;
    QString out = " By Month: ";
    while ( ( i = r.by_month[index++] ) != ICAL_RECURRENCE_ARRAY_MAX ) {
      out.append( QString::number( i ) + ' ' );
    }
    qDebug() << out;
  }
  if ( r.by_set_pos[0] != ICAL_RECURRENCE_ARRAY_MAX ) {
    int index = 0;
    QString out = " By Set Pos: ";
    while ( ( i = r.by_set_pos[index++] ) != ICAL_RECURRENCE_ARRAY_MAX ) {
      qDebug() << "=========" << i;
      out.append( QString::number( i ) + ' ' );
    }
    qDebug() << out;
  }
}
*/

icalcomponent *ICalFormatImpl::createScheduleComponent(const IncidenceBase::Ptr &incidence, iTIPMethod method)
{
    icalcomponent *message = createCalendarComponent();

    // Create VTIMEZONE components for this incidence
    TimeZoneList zones;
    if (incidence) {
        const QDateTime kd1 = incidence->dateTime(IncidenceBase::RoleStartTimeZone);
        const QDateTime kd2 = incidence->dateTime(IncidenceBase::RoleEndTimeZone);

        if (kd1.isValid() && kd1.timeZone() != QTimeZone::utc()) {
            zones << kd1.timeZone();
        }

        if (kd2.isValid() && kd2.timeZone() != QTimeZone::utc() && kd1.timeZone() != kd2.timeZone()) {
            zones << kd2.timeZone();
        }

        TimeZoneEarliestDate earliestTz;
        ICalTimeZoneParser::updateTzEarliestDate(incidence, &earliestTz);

        for (const auto &qtz : qAsConst(zones)) {
            icaltimezone *icaltz = ICalTimeZoneParser::icaltimezoneFromQTimeZone(qtz, earliestTz[qtz]);
            if (!icaltz) {
                qCritical() << "bad time zone";
            } else {
                icalcomponent *tz = icalcomponent_new_clone(icaltimezone_get_component(icaltz));
                icalcomponent_add_component(message, tz);
                icaltimezone_free(icaltz, 1);
            }
        }
    } else {
        qDebug() << "No incidence";
        return message;
    }

    icalproperty_method icalmethod = ICAL_METHOD_NONE;

    switch (method) {
    case iTIPPublish:
        icalmethod = ICAL_METHOD_PUBLISH;
        break;
    case iTIPRequest:
        icalmethod = ICAL_METHOD_REQUEST;
        break;
    case iTIPRefresh:
        icalmethod = ICAL_METHOD_REFRESH;
        break;
    case iTIPCancel:
        icalmethod = ICAL_METHOD_CANCEL;
        break;
    case iTIPAdd:
        icalmethod = ICAL_METHOD_ADD;
        break;
    case iTIPReply:
        icalmethod = ICAL_METHOD_REPLY;
        break;
    case iTIPCounter:
        icalmethod = ICAL_METHOD_COUNTER;
        break;
    case iTIPDeclineCounter:
        icalmethod = ICAL_METHOD_DECLINECOUNTER;
        break;
    default:
        qDebug() << "Unknown method";
        return message;
    }

    icalcomponent_add_property(message, icalproperty_new_method(icalmethod));

    icalcomponent *inc = writeIncidence(incidence, method);

    if (method != KCalendarCore::iTIPNoMethod) {
        // Not very nice, but since dtstamp changes semantics if used in scheduling, we have to adapt
        icalcomponent_set_dtstamp(inc, writeICalUtcDateTime(QDateTime::currentDateTimeUtc()));
    }

    /*
     * RFC 2446 states in section 3.4.3 ( REPLY to a VTODO ), that
     * a REQUEST-STATUS property has to be present. For the other two, event and
     * free busy, it can be there, but is optional. Until we do more
     * fine grained handling, assume all is well. Note that this is the
     * status of the _request_, not the attendee. Just to avoid confusion.
     * - till
     */
    if (icalmethod == ICAL_METHOD_REPLY) {
        struct icalreqstattype rst;
        rst.code = ICAL_2_0_SUCCESS_STATUS;
        rst.desc = nullptr;
        rst.debug = nullptr;
        icalcomponent_add_property(inc, icalproperty_new_requeststatus(rst));
    }
    icalcomponent_add_component(message, inc);

    return message;
}
