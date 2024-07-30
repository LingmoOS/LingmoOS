/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Incidence class.

  @brief
  Provides the class common to non-FreeBusy (Events, To-dos, Journals)
  calendar components known as incidences.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "incidence.h"
#include "incidence_p.h"
#include "calformat.h"
#include "kcalendarcore_debug.h"
#include "utils_p.h"

#include <math.h>

#include <QStringList>
#include <QTextDocument> // for .toHtmlEscaped() and Qt::mightBeRichText()
#include <QTime>
#include <QTimeZone>

using namespace KCalendarCore;

IncidencePrivate::IncidencePrivate() = default;

IncidencePrivate::IncidencePrivate(const IncidencePrivate &p)
    : IncidenceBasePrivate(p)
    , mCreated(p.mCreated)
    , mDescription(p.mDescription)
    , mSummary(p.mSummary)
    , mLocation(p.mLocation)
    , mCategories(p.mCategories)
    , mResources(p.mResources)
    , mStatusString(p.mStatusString)
    , mSchedulingID(p.mSchedulingID)
    , mRelatedToUid(p.mRelatedToUid)
    , mRecurrenceId(p.mRecurrenceId)
    , mConferences(p.mConferences)
    , mGeoLatitude(p.mGeoLatitude)
    , mGeoLongitude(p.mGeoLongitude)
    , mRecurrence(nullptr)
    , mRevision(p.mRevision)
    , mPriority(p.mPriority)
    , mStatus(p.mStatus)
    , mSecrecy(p.mSecrecy)
    , mColor(p.mColor)
    , mDescriptionIsRich(p.mDescriptionIsRich)
    , mSummaryIsRich(p.mSummaryIsRich)
    , mLocationIsRich(p.mLocationIsRich)
    , mThisAndFuture(p.mThisAndFuture)
    , mLocalOnly(false)
{
}

IncidencePrivate::IncidencePrivate(const Incidence &other)
    : IncidencePrivate(*other.d_func())
{
}

void IncidencePrivate::clear()
{
    mAlarms.clear();
    mAttachments.clear();
    delete mRecurrence;
    mRecurrence = nullptr;
}

void IncidencePrivate::init(Incidence *q, const IncidencePrivate &other)
{
    mRevision = other.mRevision;
    mCreated = other.mCreated;
    mDescription = other.mDescription;
    mDescriptionIsRich = other.mDescriptionIsRich;
    mSummary = other.mSummary;
    mSummaryIsRich = other.mSummaryIsRich;
    mCategories = other.mCategories;
    mRelatedToUid = other.mRelatedToUid;
    mResources = other.mResources;
    mStatusString = other.mStatusString;
    mStatus = other.mStatus;
    mSecrecy = other.mSecrecy;
    mPriority = other.mPriority;
    mLocation = other.mLocation;
    mLocationIsRich = other.mLocationIsRich;
    mGeoLatitude = other.mGeoLatitude;
    mGeoLongitude = other.mGeoLongitude;
    mRecurrenceId = other.mRecurrenceId;
    mConferences = other.mConferences;
    mThisAndFuture = other.mThisAndFuture;
    mLocalOnly = other.mLocalOnly;
    mColor = other.mColor;

    // Alarms and Attachments are stored in ListBase<...>, which is a QValueList<...*>.
    // We need to really duplicate the objects stored therein, otherwise deleting
    // i will also delete all attachments from this object (setAutoDelete...)
    mAlarms.reserve(other.mAlarms.count());
    for (const Alarm::Ptr &alarm : std::as_const(other.mAlarms)) {
        Alarm::Ptr b(new Alarm(*alarm.data()));
        b->setParent(q);
        mAlarms.append(b);
    }

    mAttachments = other.mAttachments;
    if (other.mRecurrence) {
        mRecurrence = new Recurrence(*(other.mRecurrence));
        mRecurrence->addObserver(q);
    } else {
        mRecurrence = nullptr;
    }
}

bool IncidencePrivate::validStatus(Incidence::Status status)
{
    return status == Incidence::StatusNone;
}
//@endcond

Incidence::Incidence(IncidencePrivate *p)
    : IncidenceBase(p)
{
    recreate();
    resetDirtyFields();
}

Incidence::Incidence(const Incidence &other, IncidencePrivate *p)
    : IncidenceBase(other, p)
    , Recurrence::RecurrenceObserver()
{
    Q_D(Incidence);
    d->init(this, *(other.d_func()));
    resetDirtyFields();
}

Incidence::~Incidence()
{
    // Alarm has a raw incidence pointer, so we must set it to 0
    // so Alarm doesn't use it after Incidence is destroyed
    Q_D(const Incidence);
    for (const Alarm::Ptr &alarm : std::as_const(d->mAlarms)) {
        alarm->setParent(nullptr);
    }
    delete d->mRecurrence;
}

//@cond PRIVATE
// A string comparison that considers that null and empty are the same
static bool stringCompare(const QString &s1, const QString &s2)
{
    return (s1.isEmpty() && s2.isEmpty()) || (s1 == s2);
}

//@endcond
IncidenceBase &Incidence::assign(const IncidenceBase &other)
{
    Q_D(Incidence);
    if (&other != this) {
        d->clear();
        // TODO: should relations be cleared out, as in destructor???
        IncidenceBase::assign(other);
        const Incidence *i = static_cast<const Incidence *>(&other);
        d->init(this, *(i->d_func()));
    }

    return *this;
}

bool Incidence::equals(const IncidenceBase &incidence) const
{
    if (!IncidenceBase::equals(incidence)) {
        return false;
    }

    // If they weren't the same type IncidenceBase::equals would had returned false already
    const Incidence *i2 = static_cast<const Incidence *>(&incidence);

    const Alarm::List alarmList = alarms();
    const Alarm::List otherAlarmsList = i2->alarms();
    if (alarmList.count() != otherAlarmsList.count()) {
        return false;
    }

    auto matchFunc = [](const Alarm::Ptr &a, const Alarm::Ptr &b) {
        return *a == *b;
    };

    const auto [it1, it2] = std::mismatch(alarmList.cbegin(), alarmList.cend(), otherAlarmsList.cbegin(), otherAlarmsList.cend(), matchFunc);
    // Checking the iterator from one list only, since both lists are the same size
    if (it1 != alarmList.cend()) {
        return false;
    }

    const Attachment::List attachmentList = attachments();
    const Attachment::List otherAttachmentList = i2->attachments();
    if (attachmentList.count() != otherAttachmentList.count()) {
        return false;
    }

    const auto [at1, at2] = std::mismatch(attachmentList.cbegin(), attachmentList.cend(), otherAttachmentList.cbegin(), otherAttachmentList.cend());

    // Checking the iterator from one list only, since both lists are the same size
    if (at1 != attachmentList.cend()) {
        return false;
    }

    Q_D(const Incidence);
    bool recurrenceEqual = (d->mRecurrence == nullptr && i2->d_func()->mRecurrence == nullptr);
    if (!recurrenceEqual) {
        recurrence(); // create if doesn't exist
        i2->recurrence(); // create if doesn't exist
        recurrenceEqual = d->mRecurrence != nullptr && i2->d_func()->mRecurrence != nullptr
            && *d->mRecurrence == *i2->d_func()->mRecurrence;
    }

    if (!qFuzzyCompare(d->mGeoLatitude, i2->d_func()->mGeoLatitude) || !qFuzzyCompare(d->mGeoLongitude, i2->d_func()->mGeoLongitude)) {
        return false;
    }
    // clang-format off
    return
        recurrenceEqual
        && created() == i2->created()
        && stringCompare(description(), i2->description())
        && descriptionIsRich() == i2->descriptionIsRich()
        && stringCompare(summary(), i2->summary())
        && summaryIsRich() == i2->summaryIsRich()
        && categories() == i2->categories()
        && stringCompare(relatedTo(), i2->relatedTo())
        && resources() == i2->resources()
        && d->mStatus == i2->d_func()->mStatus
        && (d->mStatus == StatusNone || stringCompare(d->mStatusString, i2->d_func()->mStatusString))
        && secrecy() == i2->secrecy()
        && priority() == i2->priority()
        && stringCompare(location(), i2->location())
        && locationIsRich() == i2->locationIsRich()
        && stringCompare(color(), i2->color())
        && stringCompare(schedulingID(), i2->schedulingID())
        && recurrenceId() == i2->recurrenceId()
        && conferences() == i2->conferences()
        && thisAndFuture() == i2->thisAndFuture();
    // clang-format on
}

QString Incidence::instanceIdentifier() const
{
    if (hasRecurrenceId()) {
        return uid() + recurrenceId().toString(Qt::ISODate);
    }
    return uid();
}

void Incidence::recreate()
{
    const QDateTime nowUTC = QDateTime::currentDateTimeUtc();
    setCreated(nowUTC);

    setSchedulingID(QString(), CalFormat::createUniqueId());
    setRevision(0);
    setLastModified(nowUTC); // NOLINT false clang-analyzer-optin.cplusplus.VirtualCall
}

void Incidence::setLastModified(const QDateTime &lm)
{
    Q_D(const Incidence);
    if (!d->mLocalOnly) {
        IncidenceBase::setLastModified(lm);
    }
}

void Incidence::setReadOnly(bool readOnly)
{
    Q_D(const Incidence);
    IncidenceBase::setReadOnly(readOnly);
    if (d->mRecurrence) {
        d->mRecurrence->setRecurReadOnly(readOnly);
    }
}

void Incidence::setLocalOnly(bool localOnly)
{
    if (mReadOnly) {
        return;
    }
    Q_D(Incidence);
    d->mLocalOnly = localOnly;
}

bool Incidence::localOnly() const
{
    Q_D(const Incidence);
    return d->mLocalOnly;
}

void Incidence::setAllDay(bool allDay)
{
    if (mReadOnly) {
        return;
    }
    Q_D(const Incidence);
    if (d->mRecurrence) {
        d->mRecurrence->setAllDay(allDay);
    }
    IncidenceBase::setAllDay(allDay);
}

void Incidence::setCreated(const QDateTime &created)
{
    Q_D(Incidence);
    if (mReadOnly || d->mLocalOnly) {
        return;
    }

    update();
    d->mCreated = created.toUTC();
    const auto ct = d->mCreated.time();
    // Remove milliseconds
    d->mCreated.setTime(QTime(ct.hour(), ct.minute(), ct.second()));
    setFieldDirty(FieldCreated);
    updated();
}

QDateTime Incidence::created() const
{
    Q_D(const Incidence);
    return d->mCreated;
}

void Incidence::setRevision(int rev)
{
    Q_D(Incidence);
    if (mReadOnly || d->mLocalOnly) {
        return;
    }

    update();
    d->mRevision = rev;
    setFieldDirty(FieldRevision);
    updated();
}

int Incidence::revision() const
{
    Q_D(const Incidence);
    return d->mRevision;
}

void Incidence::setDtStart(const QDateTime &dt)
{
    Q_D(const Incidence);
    IncidenceBase::setDtStart(dt);
    if (d->mRecurrence && dirtyFields().contains(FieldDtStart)) {
        d->mRecurrence->setStartDateTime(dt, allDay());
    }
}

void Incidence::shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone)
{
    Q_D(const Incidence);
    IncidenceBase::shiftTimes(oldZone, newZone);
    if (d->mRecurrence) {
        d->mRecurrence->shiftTimes(oldZone, newZone);
    }
    if (d->mAlarms.count() > 0) {
        update();
        for (auto alarm : d->mAlarms) {
            alarm->shiftTimes(oldZone, newZone);
        }
        setFieldDirty(FieldAlarms);
        updated();
    }
}

void Incidence::setDescription(const QString &description, bool isRich)
{
    if (mReadOnly) {
        return;
    }
    update();
    Q_D(Incidence);
    d->mDescription = description;
    d->mDescriptionIsRich = isRich;
    setFieldDirty(FieldDescription);
    updated();
}

void Incidence::setDescription(const QString &description)
{
    setDescription(description, Qt::mightBeRichText(description));
}

QString Incidence::description() const
{
    Q_D(const Incidence);
    return d->mDescription;
}

QString Incidence::richDescription() const
{
    Q_D(const Incidence);
    if (descriptionIsRich()) {
        return d->mDescription;
    } else {
        return d->mDescription.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
    }
}

bool Incidence::descriptionIsRich() const
{
    Q_D(const Incidence);
    return d->mDescriptionIsRich;
}

void Incidence::setSummary(const QString &summary, bool isRich)
{
    if (mReadOnly) {
        return;
    }
    Q_D(Incidence);
    if (d->mSummary != summary || d->mSummaryIsRich != isRich) {
        update();
        d->mSummary = summary;
        d->mSummaryIsRich = isRich;
        setFieldDirty(FieldSummary);
        updated();
    }
}

void Incidence::setSummary(const QString &summary)
{
    setSummary(summary, Qt::mightBeRichText(summary));
}

QString Incidence::summary() const
{
    Q_D(const Incidence);
    return d->mSummary;
}

QString Incidence::richSummary() const
{
    Q_D(const Incidence);
    if (summaryIsRich()) {
        return d->mSummary;
    } else {
        return d->mSummary.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
    }
}

bool Incidence::summaryIsRich() const
{
    Q_D(const Incidence);
    return d->mSummaryIsRich;
}

void Incidence::setCategories(const QStringList &categories)
{
    if (mReadOnly) {
        return;
    }

    Q_D(Incidence);
    update();
    d->mCategories = categories;
    setFieldDirty(FieldCategories);
    updated();
}

void Incidence::setCategories(const QString &catStr)
{
    if (mReadOnly) {
        return;
    }
    update();
    setFieldDirty(FieldCategories);

    Q_D(Incidence);
    d->mCategories.clear();

    if (catStr.isEmpty()) {
        updated();
        return;
    }

    d->mCategories = catStr.split(QLatin1Char(','));

    for (auto &category : d->mCategories) {
        category = category.trimmed();
    }

    updated();
}

QStringList Incidence::categories() const
{
    Q_D(const Incidence);
    return d->mCategories;
}

QString Incidence::categoriesStr() const
{
    Q_D(const Incidence);
    return d->mCategories.join(QLatin1Char(','));
}

void Incidence::setRelatedTo(const QString &relatedToUid, RelType relType)
{
    // TODO: RFC says that an incidence can have more than one related-to field
    // even for the same relType.

    Q_D(Incidence);
    if (d->mRelatedToUid[relType] != relatedToUid) {
        update();
        d->mRelatedToUid[relType] = relatedToUid;
        setFieldDirty(FieldRelatedTo);
        updated();
    }
}

QString Incidence::relatedTo(RelType relType) const
{
    Q_D(const Incidence);
    return d->mRelatedToUid.value(relType);
}

void Incidence::setColor(const QString &colorName)
{
    if (mReadOnly) {
        return;
    }
    Q_D(Incidence);
    if (!stringCompare(d->mColor, colorName)) {
        update();
        d->mColor = colorName;
        setFieldDirty(FieldColor);
        updated();
    }
}

QString Incidence::color() const
{
    Q_D(const Incidence);
    return d->mColor;
}

// %%%%%%%%%%%%  Recurrence-related methods %%%%%%%%%%%%%%%%%%%%

Recurrence *Incidence::recurrence() const
{
    Q_D(const Incidence);
    if (!d->mRecurrence) {
        d->mRecurrence = new Recurrence();
        d->mRecurrence->setStartDateTime(dateTime(RoleRecurrenceStart), allDay());
        d->mRecurrence->setAllDay(allDay());
        d->mRecurrence->setRecurReadOnly(mReadOnly);
        d->mRecurrence->addObserver(const_cast<KCalendarCore::Incidence *>(this));
    }

    return d->mRecurrence;
}

void Incidence::clearRecurrence()
{
    Q_D(const Incidence);
    delete d->mRecurrence;
    d->mRecurrence = nullptr;
}

ushort Incidence::recurrenceType() const
{
    Q_D(const Incidence);
    if (d->mRecurrence) {
        return d->mRecurrence->recurrenceType();
    } else {
        return Recurrence::rNone;
    }
}

bool Incidence::recurs() const
{
    Q_D(const Incidence);
    if (d->mRecurrence) {
        return d->mRecurrence->recurs();
    } else {
        return false;
    }
}

bool Incidence::recursOn(const QDate &date, const QTimeZone &timeZone) const
{
    Q_D(const Incidence);
    return d->mRecurrence && d->mRecurrence->recursOn(date, timeZone);
}

bool Incidence::recursAt(const QDateTime &qdt) const
{
    Q_D(const Incidence);
    return d->mRecurrence && d->mRecurrence->recursAt(qdt);
}

QList<QDateTime> Incidence::startDateTimesForDate(const QDate &date, const QTimeZone &timeZone) const
{
    QList<QDateTime> result;
    if (!date.isValid()) {
        qCWarning(KCALCORE_LOG) << "Invalid date encountered";
        return result;
    }

    QDateTime start = dtStart();
    QDateTime end = dateTime(RoleEndRecurrenceBase);

    // TODO_Recurrence: Also work if only due date is given...
    if (!start.isValid() && !end.isValid()) {
        return result;
    }

    // if the incidence doesn't recur,
    QDateTime kdate(date, {}, timeZone);
    if (!recurs()) {
        if (start.date() <= date && end.date() >= date) {
            result << start;
        }
        return result;
    }

    qint64 days = start.daysTo(end);
    // Account for possible recurrences going over midnight, while the original event doesn't
    QDate tmpday(date.addDays(-days - 1));
    QDateTime tmp;
    while (tmpday <= date) {
        if (recurrence()->recursOn(tmpday, timeZone)) {
            const QList<QTime> times = recurrence()->recurTimesOn(tmpday, timeZone);
            for (const QTime &time : times) {
                tmp = QDateTime(tmpday, time, start.timeZone());
                if (endDateForStart(tmp) >= kdate) {
                    result << tmp;
                }
            }
        }
        tmpday = tmpday.addDays(1);
    }
    return result;
}

QList<QDateTime> Incidence::startDateTimesForDateTime(const QDateTime &datetime) const
{
    QList<QDateTime> result;
    if (!datetime.isValid()) {
        qCWarning(KCALCORE_LOG) << "Invalid datetime encountered";
        return result;
    }

    QDateTime start = dtStart();
    QDateTime end = dateTime(RoleEndRecurrenceBase);

    // TODO_Recurrence: Also work if only due date is given...
    if (!start.isValid() && !end.isValid()) {
        return result;
    }

    // if the incidence doesn't recur,
    if (!recurs()) {
        if (!(start > datetime || end < datetime)) {
            result << start;
        }
        return result;
    }

    qint64 days = start.daysTo(end);
    // Account for possible recurrences going over midnight, while the original event doesn't
    QDate tmpday(datetime.date().addDays(-days - 1));
    QDateTime tmp;
    while (tmpday <= datetime.date()) {
        if (recurrence()->recursOn(tmpday, datetime.timeZone())) {
            // Get the times during the day (in start date's time zone) when recurrences happen
            const QList<QTime> times = recurrence()->recurTimesOn(tmpday, start.timeZone());
            for (const QTime &time : times) {
                tmp = QDateTime(tmpday, time, start.timeZone());
                if (!(tmp > datetime || endDateForStart(tmp) < datetime)) {
                    result << tmp;
                }
            }
        }
        tmpday = tmpday.addDays(1);
    }
    return result;
}

QDateTime Incidence::endDateForStart(const QDateTime &startDt) const
{
    QDateTime start = dtStart();
    QDateTime end = dateTime(RoleEndRecurrenceBase);
    if (!end.isValid()) {
        return start;
    }
    if (!start.isValid()) {
        return end;
    }

    return startDt.addSecs(start.secsTo(end));
}

void Incidence::addAttachment(const Attachment &attachment)
{
    if (mReadOnly || attachment.isEmpty()) {
        return;
    }

    Q_D(Incidence);
    update();
    d->mAttachments.append(attachment);
    setFieldDirty(FieldAttachment);
    updated();
}

void Incidence::deleteAttachments(const QString &mime)
{
    Q_D(Incidence);
    auto it = std::remove_if(d->mAttachments.begin(), d->mAttachments.end(), [&mime](const Attachment &a) {
        return a.mimeType() == mime;
    });
    if (it != d->mAttachments.end()) {
        update();
        d->mAttachments.erase(it, d->mAttachments.end());
        setFieldDirty(FieldAttachment);
        updated();
    }
}

Attachment::List Incidence::attachments() const
{
    Q_D(const Incidence);
    return d->mAttachments;
}

Attachment::List Incidence::attachments(const QString &mime) const
{
    Q_D(const Incidence);
    Attachment::List attachments;
    for (const Attachment &attachment : std::as_const(d->mAttachments)) {
        if (attachment.mimeType() == mime) {
            attachments.append(attachment);
        }
    }
    return attachments;
}

void Incidence::clearAttachments()
{
    Q_D(Incidence);
    update();
    setFieldDirty(FieldAttachment);
    d->mAttachments.clear();
    updated();
}

void Incidence::setResources(const QStringList &resources)
{
    if (mReadOnly) {
        return;
    }

    update();
    Q_D(Incidence);
    d->mResources = resources;
    setFieldDirty(FieldResources);
    updated();
}

QStringList Incidence::resources() const
{
    Q_D(const Incidence);
    return d->mResources;
}

void Incidence::setPriority(int priority)
{
    if (mReadOnly) {
        return;
    }

    if (priority < 0 || priority > 9) {
        qCWarning(KCALCORE_LOG)  << "Ignoring invalid priority" << priority;
        return;
    }

    update();
    Q_D(Incidence);
    d->mPriority = priority;
    setFieldDirty(FieldPriority);
    updated();
}

int Incidence::priority() const
{
    Q_D(const Incidence);
    return d->mPriority;
}

void Incidence::setStatus(Incidence::Status status)
{
    if (mReadOnly) {
        qCWarning(KCALCORE_LOG)  << "Attempt to set status of read-only incidence";
        return;
    }

    Q_D(Incidence);
    if (d->validStatus(status)) {
        update();
        d->mStatus = status;
        d->mStatusString.clear();
        setFieldDirty(FieldStatus);
        updated();
    } else {
        qCWarning(KCALCORE_LOG)  << "Ignoring invalid status" << status << "for" << typeStr();
    }
}

void Incidence::setCustomStatus(const QString &status)
{
    if (mReadOnly) {
        return;
    }

    update();
    Q_D(Incidence);
    d->mStatus = status.isEmpty() ? StatusNone : StatusX;
    d->mStatusString = status;
    setFieldDirty(FieldStatus);
    updated();
}

Incidence::Status Incidence::status() const
{
    Q_D(const Incidence);
    return d->mStatus;
}

QString Incidence::customStatus() const
{
    Q_D(const Incidence);
    if (d->mStatus == StatusX) {
        return d->mStatusString;
    } else {
        return QString();
    }
}

void Incidence::setSecrecy(Incidence::Secrecy secrecy)
{
    if (mReadOnly) {
        return;
    }

    update();
    Q_D(Incidence);
    d->mSecrecy = secrecy;
    setFieldDirty(FieldSecrecy);
    updated();
}

Incidence::Secrecy Incidence::secrecy() const
{
    Q_D(const Incidence);
    return d->mSecrecy;
}

Alarm::List Incidence::alarms() const
{
    Q_D(const Incidence);
    return d->mAlarms;
}

Alarm::Ptr Incidence::newAlarm()
{
    Q_D(Incidence);
    Alarm::Ptr alarm(new Alarm(this));
    addAlarm(alarm);
    return alarm;
}

void Incidence::addAlarm(const Alarm::Ptr &alarm)
{
    Q_D(Incidence);
    update();
    d->mAlarms.append(alarm);
    setFieldDirty(FieldAlarms);
    updated();
}

void Incidence::removeAlarm(const Alarm::Ptr &alarm)
{
    Q_D(Incidence);
    const int index = d->mAlarms.indexOf(alarm);
    if (index > -1) {
        update();
        d->mAlarms.remove(index);
        setFieldDirty(FieldAlarms);
        updated();
    }
}

void Incidence::clearAlarms()
{
    update();
    Q_D(Incidence);
    d->mAlarms.clear();
    setFieldDirty(FieldAlarms);
    updated();
}

bool Incidence::hasEnabledAlarms() const
{
    Q_D(const Incidence);
    return std::any_of(d->mAlarms.cbegin(), d->mAlarms.cend(), [](const Alarm::Ptr &alarm) {
        return alarm->enabled();
    });
}

Conference::List Incidence::conferences() const
{
    Q_D(const Incidence);
    return d->mConferences;
}

void Incidence::addConference(const Conference &conference)
{
    update();
    Q_D(Incidence);
    d->mConferences.push_back(conference);
    setFieldDirty(FieldConferences);
    updated();
}

void Incidence::setConferences(const Conference::List &conferences)
{
    update();
    Q_D(Incidence);
    d->mConferences = conferences;
    setFieldDirty(FieldConferences);
    updated();
}

void Incidence::clearConferences()
{
    update();
    Q_D(Incidence);
    d->mConferences.clear();
    setFieldDirty(FieldConferences);
    updated();
}

void Incidence::setLocation(const QString &location, bool isRich)
{
    if (mReadOnly) {
        return;
    }

    Q_D(Incidence);
    if (d->mLocation != location || d->mLocationIsRich != isRich) {
        update();
        d->mLocation = location;
        d->mLocationIsRich = isRich;
        setFieldDirty(FieldLocation);
        updated();
    }
}

void Incidence::setLocation(const QString &location)
{
    setLocation(location, Qt::mightBeRichText(location));
}

QString Incidence::location() const
{
    Q_D(const Incidence);
    return d->mLocation;
}

QString Incidence::richLocation() const
{
    Q_D(const Incidence);
    if (locationIsRich()) {
        return d->mLocation;
    } else {
        return d->mLocation.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
    }
}

bool Incidence::locationIsRich() const
{
    Q_D(const Incidence);
    return d->mLocationIsRich;
}

void Incidence::setSchedulingID(const QString &sid, const QString &uid)
{
    if (!uid.isEmpty()) {
        setUid(uid);
    }
    Q_D(Incidence);
    if (sid != d->mSchedulingID) {
        update();
        d->mSchedulingID = sid;
        setFieldDirty(FieldSchedulingId);
        updated();
    }
}

QString Incidence::schedulingID() const
{
    Q_D(const Incidence);
    if (d->mSchedulingID.isNull()) {
        // Nothing set, so use the normal uid
        return uid();
    }
    return d->mSchedulingID;
}

bool Incidence::hasGeo() const
{
    Q_D(const Incidence);
    // For internal consistency, return false if either coordinate is invalid.
    return d->mGeoLatitude != INVALID_LATLON && d->mGeoLongitude != INVALID_LATLON;
}

float Incidence::geoLatitude() const
{
    Q_D(const Incidence);
    // For internal consistency, both coordinates are considered invalid if either is.
    return (INVALID_LATLON == d->mGeoLongitude) ? INVALID_LATLON : d->mGeoLatitude;
}

void Incidence::setGeoLatitude(float geolatitude)
{
    if (mReadOnly) {
        return;
    }

    if (isnan(geolatitude)) {
        geolatitude = INVALID_LATLON;
    }
    if (geolatitude != INVALID_LATLON && (geolatitude < -90.0 || geolatitude > 90.0)) {
        qCWarning(KCALCORE_LOG) << "Ignoring invalid  latitude" << geolatitude;
        return;
    }

    Q_D(Incidence);
    update();
    d->mGeoLatitude = geolatitude;
    setFieldDirty(FieldGeoLatitude);
    updated();
}

float Incidence::geoLongitude() const
{
    Q_D(const Incidence);
    // For internal consistency, both coordinates are considered invalid if either is.
    return (INVALID_LATLON == d->mGeoLatitude) ? INVALID_LATLON : d->mGeoLongitude;
}

void Incidence::setGeoLongitude(float geolongitude)
{
    if (mReadOnly) {
        return;
    }

    if (isnan(geolongitude)) {
        geolongitude = INVALID_LATLON;
    }
    if (geolongitude != INVALID_LATLON && (geolongitude < -180.0 || geolongitude > 180.0)) {
        qCWarning(KCALCORE_LOG) << "Ignoring invalid  longitude" << geolongitude;
        return;
    }

    Q_D(Incidence);
    update();
    d->mGeoLongitude = geolongitude;
    setFieldDirty(FieldGeoLongitude);
    updated();
}

bool Incidence::hasRecurrenceId() const
{
    Q_D(const Incidence);
    return (allDay() && d->mRecurrenceId.date().isValid()) || d->mRecurrenceId.isValid();
}

QDateTime Incidence::recurrenceId() const
{
    Q_D(const Incidence);
    return d->mRecurrenceId;
}

void Incidence::setThisAndFuture(bool thisAndFuture)
{
    Q_D(Incidence);
    d->mThisAndFuture = thisAndFuture;
}

bool Incidence::thisAndFuture() const
{
    Q_D(const Incidence);
    return d->mThisAndFuture;
}

void Incidence::setRecurrenceId(const QDateTime &recurrenceId)
{
    if (!mReadOnly) {
        update();
        Q_D(Incidence);
        d->mRecurrenceId = recurrenceId;
        setFieldDirty(FieldRecurrenceId);
        updated();
    }
}

/** Observer interface for the recurrence class. If the recurrence is changed,
    this method will be called for the incidence the recurrence object
    belongs to. */
void Incidence::recurrenceUpdated(Recurrence *recurrence)
{
    Q_D(const Incidence);
    if (recurrence == d->mRecurrence) {
        update();
        setFieldDirty(FieldRecurrence);
        updated();
    }
}

//@cond PRIVATE
#define ALT_DESC_FIELD "X-ALT-DESC"
#define ALT_DESC_PARAMETERS QStringLiteral("FMTTYPE=text/html")
//@endcond

bool Incidence::hasAltDescription() const
{
    const QString value = nonKDECustomProperty(ALT_DESC_FIELD);
    const QString parameter = nonKDECustomPropertyParameters(ALT_DESC_FIELD);

    return parameter == ALT_DESC_PARAMETERS && !value.isEmpty();
}

void Incidence::setAltDescription(const QString &altdescription)
{
    if (altdescription.isEmpty()) {
        removeNonKDECustomProperty(ALT_DESC_FIELD);
    } else {
        setNonKDECustomProperty(ALT_DESC_FIELD, altdescription, ALT_DESC_PARAMETERS);
    }
}

QString Incidence::altDescription() const
{
    if (!hasAltDescription()) {
        return QString();
    } else {
        return nonKDECustomProperty(ALT_DESC_FIELD);
    }
}

/** static */
QStringList Incidence::mimeTypes()
{
    return QStringList() << QStringLiteral("text/calendar") << KCalendarCore::Event::eventMimeType() << KCalendarCore::Todo::todoMimeType()
                         << KCalendarCore::Journal::journalMimeType();
}

void Incidence::serialize(QDataStream &out) const
{
    Q_D(const Incidence);
    serializeQDateTimeAsKDateTime(out, d->mCreated);
    out << d->mRevision << d->mDescription << d->mDescriptionIsRich << d->mSummary << d->mSummaryIsRich << d->mLocation << d->mLocationIsRich << d->mCategories
        << d->mResources << d->mStatusString << d->mPriority << d->mSchedulingID << d->mGeoLatitude << d->mGeoLongitude
        << hasGeo();    // No longer used, but serialized/deserialized for compatibility.
    serializeQDateTimeAsKDateTime(out, d->mRecurrenceId);
    out << d->mThisAndFuture << d->mLocalOnly << d->mStatus << d->mSecrecy << (d->mRecurrence ? true : false) << (qint32)d->mAttachments.count()
        << (qint32)d->mAlarms.count() << (qint32)d->mConferences.count() << d->mRelatedToUid;

    if (d->mRecurrence) {
        out << d->mRecurrence;
    }

    for (const Attachment &attachment : std::as_const(d->mAttachments)) {
        out << attachment;
    }

    for (const Alarm::Ptr &alarm : std::as_const(d->mAlarms)) {
        out << alarm;
    }

    for (const Conference &conf : std::as_const(d->mConferences)) {
        out << conf;
    }
}

void Incidence::deserialize(QDataStream &in)
{
    bool hasGeo;    // No longer used, but serialized/deserialized for compatibility.
    quint32 status;
    quint32 secrecy;
    bool hasRecurrence;
    int attachmentCount;
    int alarmCount;
    int conferencesCount;
    QMap<int, QString> relatedToUid;
    Q_D(Incidence);
    deserializeKDateTimeAsQDateTime(in, d->mCreated);
    in >> d->mRevision >> d->mDescription >> d->mDescriptionIsRich >> d->mSummary >> d->mSummaryIsRich >> d->mLocation >> d->mLocationIsRich >> d->mCategories
        >> d->mResources >> d->mStatusString >> d->mPriority >> d->mSchedulingID >> d->mGeoLatitude >> d->mGeoLongitude >> hasGeo;
    deserializeKDateTimeAsQDateTime(in, d->mRecurrenceId);
    in >> d->mThisAndFuture >> d->mLocalOnly >> status >> secrecy >> hasRecurrence >> attachmentCount >> alarmCount >> conferencesCount >> relatedToUid;

    if (hasRecurrence) {
        d->mRecurrence = new Recurrence();
        d->mRecurrence->addObserver(const_cast<KCalendarCore::Incidence *>(this));
        in >> d->mRecurrence;
    }

    d->mAttachments.clear();
    d->mAlarms.clear();
    d->mConferences.clear();

    d->mAttachments.reserve(attachmentCount);
    for (int i = 0; i < attachmentCount; ++i) {
        Attachment attachment;
        in >> attachment;
        d->mAttachments.append(attachment);
    }

    d->mAlarms.reserve(alarmCount);
    for (int i = 0; i < alarmCount; ++i) {
        Alarm::Ptr alarm = Alarm::Ptr(new Alarm(this));
        in >> alarm;
        d->mAlarms.append(alarm);
    }

    d->mConferences.reserve(conferencesCount);
    for (int i = 0; i < conferencesCount; ++i) {
        Conference conf;
        in >> conf;
        d->mConferences.push_back(conf);
    }

    d->mStatus = static_cast<Incidence::Status>(status);
    d->mSecrecy = static_cast<Incidence::Secrecy>(secrecy);

    d->mRelatedToUid.clear();

    auto it = relatedToUid.cbegin();
    auto end = relatedToUid.cend();
    for (; it != end; ++it) {
        d->mRelatedToUid.insert(static_cast<Incidence::RelType>(it.key()), it.value());
    }
}

#include "moc_incidence.cpp"
