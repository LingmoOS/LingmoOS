/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001,2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  SPDX-FileContributor: Alvaro Manera <alvaro.manera@nokia.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the IncidenceBase class.

  @brief
  An abstract base class that provides a common base for all calendar incidence
  classes.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "incidencebase.h"
#include "incidencebase_p.h"
#include "calformat.h"
#include "utils_p.h"
#include "visitor.h"

#include "kcalendarcore_debug.h"
#include <QTime>

#include <QStringList>

#define KCALCORE_MAGIC_NUMBER 0xCA1C012E
#define KCALCORE_SERIALIZATION_VERSION 1

using namespace KCalendarCore;

//@cond PRIVATE
void IncidenceBasePrivate::init(const IncidenceBasePrivate &other)
{
    mLastModified = other.mLastModified;
    mDtStart = other.mDtStart;
    mOrganizer = other.mOrganizer;
    mUid = other.mUid;
    mDuration = other.mDuration;
    mAllDay = other.mAllDay;
    mHasDuration = other.mHasDuration;

    mComments = other.mComments;
    mContacts = other.mContacts;

    mAttendees = other.mAttendees;
    mAttendees.reserve(other.mAttendees.count());
    mUrl = other.mUrl;
}

//@endcond

IncidenceBase::IncidenceBase(IncidenceBasePrivate  *p)
    : d_ptr(p)
{
    mReadOnly = false;
    setUid(CalFormat::createUniqueId());
}

IncidenceBase::IncidenceBase(const IncidenceBase &i, IncidenceBasePrivate  *p)
    : CustomProperties(i)
    , d_ptr(p)
{
    mReadOnly = i.mReadOnly;
}

IncidenceBase::~IncidenceBase()
{
    delete d_ptr;
}

IncidenceBase &IncidenceBase::operator=(const IncidenceBase &other)
{
    Q_ASSERT(type() == other.type());

    startUpdates();

    // assign is virtual, will call the derived class's
    IncidenceBase &ret = assign(other);
    endUpdates();
    return ret;
}

IncidenceBase &IncidenceBase::assign(const IncidenceBase &other)
{
    CustomProperties::operator=(other);
    d_ptr->init(*other.d_ptr);
    mReadOnly = other.mReadOnly;
    d_ptr->mDirtyFields.clear();
    d_ptr->mDirtyFields.insert(FieldUnknown);
    return *this;
}

bool IncidenceBase::operator==(const IncidenceBase &i2) const
{
    if (i2.type() != type()) {
        return false;
    } else {
        // equals is virtual, so here we're calling the derived class method
        return equals(i2);
    }
}

bool IncidenceBase::operator!=(const IncidenceBase &i2) const
{
    return !operator==(i2);
}

bool IncidenceBase::equals(const IncidenceBase &other) const
{
    if (attendees().count() != other.attendees().count()) {
        // qCDebug(KCALCORE_LOG) << "Attendee count is different";
        return false;
    }

    // TODO Does the order of attendees in the list really matter?
    // Please delete this comment if you know it's ok, kthx
    const Attendee::List list = attendees();
    const Attendee::List otherList = other.attendees();

    if (list.size() != otherList.size()) {
        return false;
    }

    auto [it1, it2] = std::mismatch(list.cbegin(), list.cend(), otherList.cbegin(), otherList.cend());

    // Checking the iterator from one list only, since we've already checked
    // they are the same size
    if (it1 != list.cend()) {
        // qCDebug(KCALCORE_LOG) << "Attendees are different";
        return false;
    }

    if (!CustomProperties::operator==(other)) {
        // qCDebug(KCALCORE_LOG) << "Properties are different";
        return false;
    }

    // Don't compare lastModified, otherwise the operator is not
    // of much use. We are not comparing for identity, after all.
    // no need to compare mObserver

    bool a = identical(dtStart(), other.dtStart());
    bool b = organizer() == other.organizer();
    bool c = uid() == other.uid();
    bool d = allDay() == other.allDay();
    bool e = duration() == other.duration();
    bool f = hasDuration() == other.hasDuration();
    bool g = url() == other.url();

    // qCDebug(KCALCORE_LOG) << a << b << c << d << e << f << g;
    return a && b && c && d && e && f && g;
}

bool IncidenceBase::accept(Visitor &v, const IncidenceBase::Ptr &incidence)
{
    Q_UNUSED(v);
    Q_UNUSED(incidence);
    return false;
}

void IncidenceBase::setUid(const QString &uid)
{
    if (d_ptr->mUid != uid) {
        update();
        d_ptr->mUid = uid;
        d_ptr->mDirtyFields.insert(FieldUid);
        updated();
    }
}

QString IncidenceBase::uid() const
{
    return d_ptr->mUid;
}

void IncidenceBase::setLastModified(const QDateTime &lm)
{
    // DON'T! updated() because we call this from
    // Calendar::updateEvent().

    d_ptr->mDirtyFields.insert(FieldLastModified);

    // Convert to UTC and remove milliseconds part.
    QDateTime current = lm.toUTC();
    QTime t = current.time();
    t.setHMS(t.hour(), t.minute(), t.second(), 0);
    current.setTime(t);

    d_ptr->mLastModified = current;
}

QDateTime IncidenceBase::lastModified() const
{
    return d_ptr->mLastModified;
}

void IncidenceBase::setOrganizer(const Person &organizer)
{
    update();
    // we don't check for readonly here, because it is
    // possible that by setting the organizer we are changing
    // the event's readonly status...
    d_ptr->mOrganizer = organizer;

    d_ptr->mDirtyFields.insert(FieldOrganizer);

    updated();
}

void IncidenceBase::setOrganizer(const QString &o)
{
    QString mail(o);
    if (mail.startsWith(QLatin1String("MAILTO:"), Qt::CaseInsensitive)) {
        mail.remove(0, 7);
    }

    // split the string into full name plus email.
    const Person organizer = Person::fromFullName(mail);
    setOrganizer(organizer);
}

Person IncidenceBase::organizer() const
{
    return d_ptr->mOrganizer;
}

void IncidenceBase::setReadOnly(bool readOnly)
{
    mReadOnly = readOnly;
}

bool IncidenceBase::isReadOnly() const
{
    return mReadOnly;
}

void IncidenceBase::setDtStart(const QDateTime &dtStart)
{
    //  if ( mReadOnly ) return;

    if (!dtStart.isValid() && type() != IncidenceBase::TypeTodo) {
        qCWarning(KCALCORE_LOG) << "Invalid dtStart";
    }

    if (!identical(d_ptr->mDtStart, dtStart)) {
        update();
        d_ptr->mDtStart = dtStart;
        d_ptr->mDirtyFields.insert(FieldDtStart);
        updated();
    }
}

QDateTime IncidenceBase::dtStart() const
{
    return d_ptr->mDtStart;
}

bool IncidenceBase::allDay() const
{
    return d_ptr->mAllDay;
}

void IncidenceBase::setAllDay(bool f)
{
    if (mReadOnly || f == d_ptr->mAllDay) {
        return;
    }
    update();
    d_ptr->mAllDay = f;
    if (d_ptr->mDtStart.isValid()) {
        d_ptr->mDirtyFields.insert(FieldDtStart);
    }
    updated();
}

void IncidenceBase::shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone)
{
    update();
    d_ptr->mDtStart = d_ptr->mDtStart.toTimeZone(oldZone);
    d_ptr->mDtStart.setTimeZone(newZone);
    d_ptr->mDirtyFields.insert(FieldDtStart);
    updated();
}

void IncidenceBase::addComment(const QString &comment)
{
    update();
    d_ptr->mComments += comment;
    d_ptr->mDirtyFields.insert(FieldComment);
    updated();
}

bool IncidenceBase::removeComment(const QString &comment)
{
    auto it = std::find(d_ptr->mComments.begin(), d_ptr->mComments.end(), comment);
    bool found = it != d_ptr->mComments.end();
    if (found) {
        update();
        d_ptr->mComments.erase(it);
        d_ptr->mDirtyFields.insert(FieldComment);
        updated();
    }
    return found;
}

void IncidenceBase::clearComments()
{
    update();
    d_ptr->mDirtyFields.insert(FieldComment);
    d_ptr->mComments.clear();
    updated();
}

QStringList IncidenceBase::comments() const
{
    return d_ptr->mComments;
}

void IncidenceBase::addContact(const QString &contact)
{
    if (!contact.isEmpty()) {
        update();
        d_ptr->mContacts += contact;
        d_ptr->mDirtyFields.insert(FieldContact);
        updated();
    }
}

bool IncidenceBase::removeContact(const QString &contact)
{
    auto it = std::find(d_ptr->mContacts.begin(), d_ptr->mContacts.end(), contact);
    bool found = it != d_ptr->mContacts.end();
    if (found) {
        update();
        d_ptr->mContacts.erase(it);
        d_ptr->mDirtyFields.insert(FieldContact);
        updated();
    }
    return found;
}

void IncidenceBase::clearContacts()
{
    update();
    d_ptr->mDirtyFields.insert(FieldContact);
    d_ptr->mContacts.clear();
    updated();
}

QStringList IncidenceBase::contacts() const
{
    return d_ptr->mContacts;
}

void IncidenceBase::addAttendee(const Attendee &a, bool doupdate)
{
    if (a.isNull() || mReadOnly) {
        return;
    }
    Q_ASSERT(!a.uid().isEmpty());

    if (doupdate) {
        update();
    }

    d_ptr->mAttendees.append(a);
    if (doupdate) {
        d_ptr->mDirtyFields.insert(FieldAttendees);
        updated();
    }
}

Attendee::List IncidenceBase::attendees() const
{
    return d_ptr->mAttendees;
}

int IncidenceBase::attendeeCount() const
{
    return d_ptr->mAttendees.count();
}

void IncidenceBase::setAttendees(const Attendee::List &attendees, bool doUpdate)
{
    if (mReadOnly) {
        return;
    }

    // don't simply assign, we need the logic in addAttendee here too
    clearAttendees();

    if (doUpdate) {
        update();
    }

    d_ptr->mAttendees.reserve(attendees.size());
    for (const auto &a : attendees) {
        addAttendee(a, false);
    }

    if (doUpdate) {
        d_ptr->mDirtyFields.insert(FieldAttendees);
        updated();
    }
}

void IncidenceBase::clearAttendees()
{
    if (mReadOnly) {
        return;
    }
    update();
    d_ptr->mDirtyFields.insert(FieldAttendees);
    d_ptr->mAttendees.clear();
    updated();
}

Attendee IncidenceBase::attendeeByMail(const QString &email) const
{
    auto it = std::find_if(d_ptr->mAttendees.cbegin(), d_ptr->mAttendees.cend(), [&email](const Attendee &att) {
        return att.email() == email;
    });

    return it != d_ptr->mAttendees.cend() ? *it : Attendee{};
}

Attendee IncidenceBase::attendeeByMails(const QStringList &emails, const QString &email) const
{
    QStringList mails = emails;
    if (!email.isEmpty()) {
        mails.append(email);
    }

    auto it = std::find_if(d_ptr->mAttendees.cbegin(), d_ptr->mAttendees.cend(), [&mails](const Attendee &a) {
        return mails.contains(a.email());
    });

    return it != d_ptr->mAttendees.cend() ? *it : Attendee{};
}

Attendee IncidenceBase::attendeeByUid(const QString &uid) const
{
    auto it = std::find_if(d_ptr->mAttendees.cbegin(), d_ptr->mAttendees.cend(), [&uid](const Attendee &a) {
        return a.uid() == uid;
    });
    return it != d_ptr->mAttendees.cend() ? *it : Attendee{};
}

void IncidenceBase::setDuration(const Duration &duration)
{
    update();
    d_ptr->mDuration = duration;
    setHasDuration(true);
    d_ptr->mDirtyFields.insert(FieldDuration);
    updated();
}

Duration IncidenceBase::duration() const
{
    return d_ptr->mDuration;
}

void IncidenceBase::setHasDuration(bool hasDuration)
{
    d_ptr->mHasDuration = hasDuration;
}

bool IncidenceBase::hasDuration() const
{
    return d_ptr->mHasDuration;
}

void IncidenceBase::setUrl(const QUrl &url)
{
    update();
    d_ptr->mDirtyFields.insert(FieldUrl);
    d_ptr->mUrl = url;
    updated();
}

QUrl IncidenceBase::url() const
{
    return d_ptr->mUrl;
}

void IncidenceBase::registerObserver(IncidenceBase::IncidenceObserver *observer)
{
    if (observer && !d_ptr->mObservers.contains(observer)) {
        d_ptr->mObservers.append(observer);
    }
}

void IncidenceBase::unRegisterObserver(IncidenceBase::IncidenceObserver *observer)
{
    d_ptr->mObservers.removeAll(observer);
}

void IncidenceBase::update()
{
    if (!d_ptr->mUpdateGroupLevel) {
        d_ptr->mUpdatedPending = true;
        const auto rid = recurrenceId();
        for (IncidenceObserver *o : std::as_const(d_ptr->mObservers)) {
            o->incidenceUpdate(uid(), rid);
        }
    }
}

void IncidenceBase::updated()
{
    if (d_ptr->mUpdateGroupLevel) {
        d_ptr->mUpdatedPending = true;
    } else {
        const auto rid = recurrenceId();
        for (IncidenceObserver *o : std::as_const(d_ptr->mObservers)) {
            o->incidenceUpdated(uid(), rid);
        }
    }
}

void IncidenceBase::startUpdates()
{
    update();
    ++d_ptr->mUpdateGroupLevel;
}

void IncidenceBase::endUpdates()
{
    if (d_ptr->mUpdateGroupLevel > 0) {
        if (--d_ptr->mUpdateGroupLevel == 0 && d_ptr->mUpdatedPending) {
            d_ptr->mUpdatedPending = false;
            updated();
        }
    }
}

void IncidenceBase::customPropertyUpdate()
{
    update();
}

void IncidenceBase::customPropertyUpdated()
{
    updated();
}

QDateTime IncidenceBase::recurrenceId() const
{
    return QDateTime();
}

void IncidenceBase::resetDirtyFields()
{
    d_ptr->mDirtyFields.clear();
}

QSet<IncidenceBase::Field> IncidenceBase::dirtyFields() const
{
    return d_ptr->mDirtyFields;
}

void IncidenceBase::setFieldDirty(IncidenceBase::Field field)
{
    d_ptr->mDirtyFields.insert(field);
}

QUrl IncidenceBase::uri() const
{
    return QUrl(QStringLiteral("urn:x-ical:") + uid());
}

void IncidenceBase::setDirtyFields(const QSet<IncidenceBase::Field> &dirtyFields)
{
    d_ptr->mDirtyFields = dirtyFields;
}

void IncidenceBase::serialize(QDataStream &out) const
{
    Q_UNUSED(out);
}

void IncidenceBase::deserialize(QDataStream &in)
{
    Q_UNUSED(in);
}

/** static */
quint32 IncidenceBase::magicSerializationIdentifier()
{
    return KCALCORE_MAGIC_NUMBER;
}

static bool isUtc(const QDateTime &dt)
{
    return dt.timeSpec() == Qt::UTC || (dt.timeSpec() == Qt::TimeZone && dt.timeZone() == QTimeZone::utc())
        || (dt.timeSpec() == Qt::OffsetFromUTC && dt.offsetFromUtc() == 0);
}

bool KCalendarCore::identical(const QDateTime &dt1, const QDateTime &dt2)
{
    if (dt1 != dt2) {
        return false;
    }

    return (dt1.timeSpec() == dt2.timeSpec() && dt1.timeZone() == dt2.timeZone()) || (isUtc(dt1) && isUtc(dt2));
}

QDataStream &KCalendarCore::operator<<(QDataStream &out, const KCalendarCore::IncidenceBase::Ptr &i)
{
    if (!i) {
        return out;
    }

    out << static_cast<quint32>(KCALCORE_MAGIC_NUMBER); // Magic number to identify KCalendarCore data
    out << static_cast<quint32>(KCALCORE_SERIALIZATION_VERSION);
    out << static_cast<qint32>(i->type());

    out << *(static_cast<CustomProperties *>(i.data()));
    serializeQDateTimeAsKDateTime(out, i->d_ptr->mLastModified);
    serializeQDateTimeAsKDateTime(out, i->d_ptr->mDtStart);
    out << i->organizer() << i->d_ptr->mUid << i->d_ptr->mDuration << i->d_ptr->mAllDay << i->d_ptr->mHasDuration << i->d_ptr->mComments << i->d_ptr->mContacts
        << (qint32)i->d_ptr->mAttendees.count() << i->d_ptr->mUrl;

    for (const Attendee &attendee : std::as_const(i->d_ptr->mAttendees)) {
        out << attendee;
    }

    // Serialize the sub-class data.
    i->serialize(out);

    return out;
}

QDataStream &KCalendarCore::operator>>(QDataStream &in, KCalendarCore::IncidenceBase::Ptr &i)
{
    if (!i) {
        return in;
    }

    qint32 attendeeCount;
    qint32 type;
    quint32 magic;
    quint32 version;

    in >> magic;

    if (magic != KCALCORE_MAGIC_NUMBER) {
        qCWarning(KCALCORE_LOG) << "Invalid magic on serialized data";
        return in;
    }

    in >> version;

    if (version > KCALCORE_MAGIC_NUMBER) {
        qCWarning(KCALCORE_LOG) << "Invalid version on serialized data";
        return in;
    }

    in >> type;

    in >> *(static_cast<CustomProperties *>(i.data()));
    deserializeKDateTimeAsQDateTime(in, i->d_ptr->mLastModified);
    deserializeKDateTimeAsQDateTime(in, i->d_ptr->mDtStart);
    in >> i->d_ptr->mOrganizer >> i->d_ptr->mUid >> i->d_ptr->mDuration >> i->d_ptr->mAllDay >> i->d_ptr->mHasDuration >> i->d_ptr->mComments >> i->d_ptr->mContacts >> attendeeCount
        >> i->d_ptr->mUrl;

    i->d_ptr->mAttendees.clear();
    i->d_ptr->mAttendees.reserve(attendeeCount);
    for (int it = 0; it < attendeeCount; it++) {
        Attendee attendee;
        in >> attendee;
        i->d_ptr->mAttendees.append(attendee);
    }

    // Deserialize the sub-class data.
    i->deserialize(in);

    return in;
}

IncidenceBase::IncidenceObserver::~IncidenceObserver()
{
}

#include "moc_incidencebase.cpp"
