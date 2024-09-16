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
#include "calformat.h"
#include "utils_p.h"

#include <QStringList>
#include <QTextDocument> // for .toHtmlEscaped() and Qt::mightBeRichText()
#include <QTime>
#include <QTimeZone>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::Incidence::Private
{
public:
    Private()
        : mGeoLatitude(INVALID_LATLON)
        , mGeoLongitude(INVALID_LATLON)
        , mRecurrence(nullptr)
        , mRevision(0)
        , mPriority(0)
        , mStatus(StatusNone)
        , mSecrecy(SecrecyPublic)
        , mDescriptionIsRich(false)
        , mSummaryIsRich(false)
        , mLocationIsRich(false)
        , mHasGeo(false)
        , mThisAndFuture(false)
        , mLocalOnly(false)
    {
    }

    Private(const Private &p)
        : mCreated(p.mCreated)
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
        , mHasGeo(p.mHasGeo)
        , mThisAndFuture(p.mThisAndFuture)
        , mLocalOnly(false)
    {
    }

    void clear()
    {
        mAlarms.clear();
        mAttachments.clear();
        delete mRecurrence;
        mRecurrence = nullptr;
    }

    void init(Incidence *dest, const Incidence &src)
    {
        mRevision = src.d->mRevision;
        mCreated = src.d->mCreated;
        mDescription = src.d->mDescription;
        mDescriptionIsRich = src.d->mDescriptionIsRich;
        mSummary = src.d->mSummary;
        mSummaryIsRich = src.d->mSummaryIsRich;
        mCategories = src.d->mCategories;
        mRelatedToUid = src.d->mRelatedToUid;
        mResources = src.d->mResources;
        mStatusString = src.d->mStatusString;
        mStatus = src.d->mStatus;
        mSecrecy = src.d->mSecrecy;
        mPriority = src.d->mPriority;
        mLocation = src.d->mLocation;
        mLocationIsRich = src.d->mLocationIsRich;
        mGeoLatitude = src.d->mGeoLatitude;
        mGeoLongitude = src.d->mGeoLongitude;
        mHasGeo = src.d->mHasGeo;
        mRecurrenceId = src.d->mRecurrenceId;
        mConferences = src.d->mConferences;
        mThisAndFuture = src.d->mThisAndFuture;
        mLocalOnly = src.d->mLocalOnly;
        mColor = src.d->mColor;

        // Alarms and Attachments are stored in ListBase<...>, which is a QValueList<...*>.
        // We need to really duplicate the objects stored therein, otherwise deleting
        // i will also delete all attachments from this object (setAutoDelete...)
        mAlarms.reserve(src.d->mAlarms.count());
        for (const Alarm::Ptr &alarm : qAsConst(src.d->mAlarms)) {
            Alarm::Ptr b(new Alarm(*alarm.data()));
            b->setParent(dest);
            mAlarms.append(b);
        }

        mAttachments = src.d->mAttachments;
        if (src.d->mRecurrence) {
            mRecurrence = new Recurrence(*(src.d->mRecurrence));
            mRecurrence->addObserver(dest);
        } else {
            mRecurrence = nullptr;
        }
    }

    QDateTime mCreated; // creation datetime
    QString mDescription; // description string
    QString mSummary; // summary string
    QString mLocation; // location string
    QStringList mCategories; // category list
    Attachment::List mAttachments; // attachments list
    Alarm::List mAlarms; // alarms list
    QStringList mResources; // resources list (not calendar resources)
    QString mStatusString; // status string, for custom status
    QString mSchedulingID; // ID for scheduling mails
    QMap<RelType, QString> mRelatedToUid; // incidence uid this is related to, for each relType
    QDateTime mRecurrenceId; // recurrenceId
    Conference::List mConferences; // conference list

    float mGeoLatitude; // Specifies latitude in decimal degrees
    float mGeoLongitude; // Specifies longitude in decimal degrees
    mutable Recurrence *mRecurrence; // recurrence
    int mRevision; // revision number
    int mPriority; // priority: 1 = highest, 2 = less, etc.
    Status mStatus; // status
    Secrecy mSecrecy; // secrecy
    QString mColor; // background color
    bool mDescriptionIsRich = false; // description string is richtext.
    bool mSummaryIsRich = false; // summary string is richtext.
    bool mLocationIsRich = false; // location string is richtext.
    bool mHasGeo = false; // if incidence has geo data
    bool mThisAndFuture = false;
    bool mLocalOnly = false; // allow changes that won't go to the server
};
//@endcond

Incidence::Incidence()
    : IncidenceBase()
    , d(new KCalendarCore::Incidence::Private)
{
    recreate();
    resetDirtyFields();
}

Incidence::Incidence(const Incidence &i)
    : IncidenceBase(i)
    , Recurrence::RecurrenceObserver()
    , d(new KCalendarCore::Incidence::Private(*i.d))
{
    d->init(this, i);
    resetDirtyFields();
}

Incidence::~Incidence()
{
    // Alarm has a raw incidence pointer, so we must set it to 0
    // so Alarm doesn't use it after Incidence is destroyed
    for (const Alarm::Ptr &alarm : qAsConst(d->mAlarms)) {
        alarm->setParent(nullptr);
    }
    delete d->mRecurrence;
    delete d;
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
    if (&other != this) {
        d->clear();
        // TODO: should relations be cleared out, as in destructor???
        IncidenceBase::assign(other);
        const Incidence *i = static_cast<const Incidence *>(&other);
        d->init(this, *i);
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

    const auto alarmList = alarms();
    const auto otherAlarmsList = i2->alarms();
    if (alarmList.count() != otherAlarmsList.count()) {
        return false;
    }

    Alarm::List::ConstIterator a1 = alarmList.constBegin();
    Alarm::List::ConstIterator a1end = alarmList.constEnd();
    Alarm::List::ConstIterator a2 = otherAlarmsList.constBegin();
    Alarm::List::ConstIterator a2end = otherAlarmsList.constEnd();
    for (; a1 != a1end && a2 != a2end; ++a1, ++a2) {
        if (**a1 == **a2) {
            continue;
        } else {
            return false;
        }
    }

    const auto attachmentList = attachments();
    const auto otherAttachmentList = i2->attachments();
    if (attachmentList.count() != otherAttachmentList.count()) {
        return false;
    }

    Attachment::List::ConstIterator att1 = attachmentList.constBegin();
    const Attachment::List::ConstIterator att1end = attachmentList.constEnd();
    Attachment::List::ConstIterator att2 = otherAttachmentList.constBegin();
    const Attachment::List::ConstIterator att2end = otherAttachmentList.constEnd();
    for (; att1 != att1end && att2 != att2end; ++att1, ++att2) {
        if (*att1 == *att2) {
            continue;
        } else {
            return false;
        }
    }

    bool recurrenceEqual = (d->mRecurrence == nullptr && i2->d->mRecurrence == nullptr);
    if (!recurrenceEqual) {
        recurrence(); // create if doesn't exist
        i2->recurrence(); // create if doesn't exist
        recurrenceEqual = d->mRecurrence != nullptr && i2->d->mRecurrence != nullptr && *d->mRecurrence == *i2->d->mRecurrence;
    }

    if (d->mHasGeo == i2->d->mHasGeo) {
        if (d->mHasGeo && (!qFuzzyCompare(d->mGeoLatitude, i2->d->mGeoLatitude) || !qFuzzyCompare(d->mGeoLongitude, i2->d->mGeoLongitude))) {
            return false;
        }
    } else {
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
        && d->mStatus == i2->d->mStatus
        && (d->mStatus == StatusNone || stringCompare(d->mStatusString, i2->d->mStatusString))
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
    if (!d->mLocalOnly) {
        IncidenceBase::setLastModified(lm);
    }
}

void Incidence::setReadOnly(bool readOnly)
{
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
    d->mLocalOnly = localOnly;
}

bool Incidence::localOnly() const
{
    return d->mLocalOnly;
}

void Incidence::setAllDay(bool allDay)
{
    if (mReadOnly) {
        return;
    }
    if (d->mRecurrence) {
        d->mRecurrence->setAllDay(allDay);
    }
    IncidenceBase::setAllDay(allDay);
}

void Incidence::setCreated(const QDateTime &created)
{
    if (mReadOnly || d->mLocalOnly) {
        return;
    }

    d->mCreated = created.toUTC();
    const auto ct = d->mCreated.time();
    // Remove milliseconds
    d->mCreated.setTime(QTime(ct.hour(), ct.minute(), ct.second()));
    setFieldDirty(FieldCreated);

    // FIXME: Shouldn't we call updated for the creation date, too?
    //  updated();
}

QDateTime Incidence::created() const
{
    return d->mCreated;
}

void Incidence::setRevision(int rev)
{
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
    return d->mRevision;
}

void Incidence::setDtStart(const QDateTime &dt)
{
    IncidenceBase::setDtStart(dt);
    if (d->mRecurrence && dirtyFields().contains(FieldDtStart)) {
        d->mRecurrence->setStartDateTime(dt, allDay());
    }
}

void Incidence::shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone)
{
    IncidenceBase::shiftTimes(oldZone, newZone);
    if (d->mRecurrence) {
        d->mRecurrence->shiftTimes(oldZone, newZone);
    }
    for (int i = 0, end = d->mAlarms.count(); i < end; ++i) {
        d->mAlarms[i]->shiftTimes(oldZone, newZone);
    }
}

void Incidence::setDescription(const QString &description, bool isRich)
{
    if (mReadOnly) {
        return;
    }
    update();
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
    return d->mDescription;
}

QString Incidence::richDescription() const
{
    if (descriptionIsRich()) {
        return d->mDescription;
    } else {
        return d->mDescription.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
    }
}

bool Incidence::descriptionIsRich() const
{
    return d->mDescriptionIsRich;
}

void Incidence::setSummary(const QString &summary, bool isRich)
{
    if (mReadOnly) {
        return;
    }
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
    return d->mSummary;
}

QString Incidence::richSummary() const
{
    if (summaryIsRich()) {
        return d->mSummary;
    } else {
        return d->mSummary.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
    }
}

bool Incidence::summaryIsRich() const
{
    return d->mSummaryIsRich;
}

void Incidence::setCategories(const QStringList &categories)
{
    if (mReadOnly) {
        return;
    }

    update();
    d->mCategories = categories;
    updated();
}

void Incidence::setCategories(const QString &catStr)
{
    if (mReadOnly) {
        return;
    }
    update();
    setFieldDirty(FieldCategories);

    d->mCategories.clear();

    if (catStr.isEmpty()) {
        updated();
        return;
    }

    d->mCategories = catStr.split(QLatin1Char(','));

    QStringList::Iterator it;
    for (it = d->mCategories.begin(); it != d->mCategories.end(); ++it) {
        *it = (*it).trimmed();
    }

    updated();
}

QStringList Incidence::categories() const
{
    return d->mCategories;
}

QString Incidence::categoriesStr() const
{
    return d->mCategories.join(QLatin1Char(','));
}

void Incidence::setRelatedTo(const QString &relatedToUid, RelType relType)
{
    // TODO: RFC says that an incidence can have more than one related-to field
    // even for the same relType.

    if (d->mRelatedToUid[relType] != relatedToUid) {
        update();
        d->mRelatedToUid[relType] = relatedToUid;
        setFieldDirty(FieldRelatedTo);
        updated();
    }
}

QString Incidence::relatedTo(RelType relType) const
{
    return d->mRelatedToUid.value(relType);
}

void Incidence::setColor(const QString &colorName)
{
    if (mReadOnly) {
        return;
    }
    if (!stringCompare(d->mColor, colorName)) {
        update();
        d->mColor = colorName;
        setFieldDirty(FieldColor);
        updated();
    }
}

QString Incidence::color() const
{
    return d->mColor;
}

// %%%%%%%%%%%%  Recurrence-related methods %%%%%%%%%%%%%%%%%%%%

Recurrence *Incidence::recurrence() const
{
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
    delete d->mRecurrence;
    d->mRecurrence = nullptr;
}

ushort Incidence::recurrenceType() const
{
    if (d->mRecurrence) {
        return d->mRecurrence->recurrenceType();
    } else {
        return Recurrence::rNone;
    }
}

bool Incidence::recurs() const
{
    if (d->mRecurrence) {
        return d->mRecurrence->recurs();
    } else {
        return false;
    }
}

bool Incidence::recursOn(const QDate &date, const QTimeZone &timeZone) const
{
    return d->mRecurrence && d->mRecurrence->recursOn(date, timeZone);
}

bool Incidence::recursAt(const QDateTime &qdt) const
{
    return d->mRecurrence && d->mRecurrence->recursAt(qdt);
}

QList<QDateTime> Incidence::startDateTimesForDate(const QDate &date, const QTimeZone &timeZone) const
{
    QDateTime start = dtStart();
    QDateTime end = dateTime(RoleEndRecurrenceBase);

    QList<QDateTime> result;

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
    QDateTime start = dtStart();
    QDateTime end = dateTime(RoleEndRecurrenceBase);

    QList<QDateTime> result;

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

    update();
    d->mAttachments.append(attachment);
    setFieldDirty(FieldAttachment);
    updated();
}

void Incidence::deleteAttachments(const QString &mime)
{
    Attachment::List result;
    Attachment::List::Iterator it = d->mAttachments.begin();
    while (it != d->mAttachments.end()) {
        if ((*it).mimeType() != mime) {
            result += *it;
        }
        ++it;
    }
    d->mAttachments = result;
    setFieldDirty(FieldAttachment);
}

Attachment::List Incidence::attachments() const
{
    return d->mAttachments;
}

Attachment::List Incidence::attachments(const QString &mime) const
{
    Attachment::List attachments;
    for (const Attachment &attachment : qAsConst(d->mAttachments)) {
        if (attachment.mimeType() == mime) {
            attachments.append(attachment);
        }
    }
    return attachments;
}

void Incidence::clearAttachments()
{
    setFieldDirty(FieldAttachment);
    d->mAttachments.clear();
}

void Incidence::setResources(const QStringList &resources)
{
    if (mReadOnly) {
        return;
    }

    update();
    d->mResources = resources;
    setFieldDirty(FieldResources);
    updated();
}

QStringList Incidence::resources() const
{
    return d->mResources;
}

void Incidence::setPriority(int priority)
{
    if (mReadOnly) {
        return;
    }

    update();
    d->mPriority = priority;
    setFieldDirty(FieldPriority);
    updated();
}

int Incidence::priority() const
{
    return d->mPriority;
}

void Incidence::setStatus(Incidence::Status status)
{
    if (mReadOnly || status == StatusX) {
        return;
    }

    update();
    d->mStatus = status;
    d->mStatusString.clear();
    setFieldDirty(FieldStatus);
    updated();
}

void Incidence::setCustomStatus(const QString &status)
{
    if (mReadOnly) {
        return;
    }

    update();
    d->mStatus = status.isEmpty() ? StatusNone : StatusX;
    d->mStatusString = status;
    setFieldDirty(FieldStatus);
    updated();
}

Incidence::Status Incidence::status() const
{
    return d->mStatus;
}

QString Incidence::customStatus() const
{
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
    d->mSecrecy = secrecy;
    setFieldDirty(FieldSecrecy);
    updated();
}

Incidence::Secrecy Incidence::secrecy() const
{
    return d->mSecrecy;
}

Alarm::List Incidence::alarms() const
{
    return d->mAlarms;
}

Alarm::Ptr Incidence::newAlarm()
{
    Alarm::Ptr alarm(new Alarm(this));
    d->mAlarms.append(alarm);
    return alarm;
}

void Incidence::addAlarm(const Alarm::Ptr &alarm)
{
    update();
    d->mAlarms.append(alarm);
    setFieldDirty(FieldAlarms);
    updated();
}

void Incidence::removeAlarm(const Alarm::Ptr &alarm)
{
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
    d->mAlarms.clear();
    setFieldDirty(FieldAlarms);
    updated();
}

bool Incidence::hasEnabledAlarms() const
{
    for (const Alarm::Ptr &alarm : qAsConst(d->mAlarms)) {
        if (alarm->enabled()) {
            return true;
        }
    }
    return false;
}

Conference::List Incidence::conferences() const
{
    return d->mConferences;
}

void Incidence::addConference(const Conference &conference)
{
    update();
    d->mConferences.push_back(conference);
    setFieldDirty(FieldConferences);
    updated();
}

void Incidence::setConferences(const Conference::List &conferences)
{
    update();
    d->mConferences = conferences;
    setFieldDirty(FieldConferences);
    updated();
}

void Incidence::clearConferences()
{
    update();
    d->mConferences.clear();
    setFieldDirty(FieldConferences);
    updated();
}

void Incidence::setLocation(const QString &location, bool isRich)
{
    if (mReadOnly) {
        return;
    }

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
    return d->mLocation;
}

QString Incidence::richLocation() const
{
    if (locationIsRich()) {
        return d->mLocation;
    } else {
        return d->mLocation.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
    }
}

bool Incidence::locationIsRich() const
{
    return d->mLocationIsRich;
}

void Incidence::setSchedulingID(const QString &sid, const QString &uid)
{
    if (!uid.isEmpty()) {
        setUid(uid);
    }
    if (sid != d->mSchedulingID) {
        d->mSchedulingID = sid;
        setFieldDirty(FieldSchedulingId);
    }
}

QString Incidence::schedulingID() const
{
    if (d->mSchedulingID.isNull()) {
        // Nothing set, so use the normal uid
        return uid();
    }
    return d->mSchedulingID;
}

bool Incidence::hasGeo() const
{
    return d->mHasGeo;
}

void Incidence::setHasGeo(bool hasGeo)
{
    if (mReadOnly) {
        return;
    }

    if (hasGeo == d->mHasGeo) {
        return;
    }

    update();
    d->mHasGeo = hasGeo;
    setFieldDirty(FieldGeoLatitude);
    setFieldDirty(FieldGeoLongitude);
    updated();
}

float Incidence::geoLatitude() const
{
    return d->mGeoLatitude;
}

void Incidence::setGeoLatitude(float geolatitude)
{
    if (mReadOnly) {
        return;
    }

    update();
    d->mGeoLatitude = geolatitude;
    setFieldDirty(FieldGeoLatitude);
    updated();
}

float Incidence::geoLongitude() const
{
    return d->mGeoLongitude;
}

void Incidence::setGeoLongitude(float geolongitude)
{
    if (!mReadOnly) {
        update();
        d->mGeoLongitude = geolongitude;
        setFieldDirty(FieldGeoLongitude);
        updated();
    }
}

bool Incidence::hasRecurrenceId() const
{
    return (allDay() && d->mRecurrenceId.date().isValid()) || d->mRecurrenceId.isValid();
}

QDateTime Incidence::recurrenceId() const
{
    return d->mRecurrenceId;
}

void Incidence::setThisAndFuture(bool thisAndFuture)
{
    d->mThisAndFuture = thisAndFuture;
}

bool Incidence::thisAndFuture() const
{
    return d->mThisAndFuture;
}

void Incidence::setRecurrenceId(const QDateTime &recurrenceId)
{
    if (!mReadOnly) {
        update();
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
    serializeQDateTimeAsKDateTime(out, d->mCreated);
    out << d->mRevision << d->mDescription << d->mDescriptionIsRich << d->mSummary << d->mSummaryIsRich << d->mLocation << d->mLocationIsRich << d->mCategories
        << d->mResources << d->mStatusString << d->mPriority << d->mSchedulingID << d->mGeoLatitude << d->mGeoLongitude << d->mHasGeo;
    serializeQDateTimeAsKDateTime(out, d->mRecurrenceId);
    out << d->mThisAndFuture << d->mLocalOnly << d->mStatus << d->mSecrecy << (d->mRecurrence ? true : false) << d->mAttachments.count() << d->mAlarms.count()
        << d->mConferences.count() << d->mRelatedToUid;

    if (d->mRecurrence) {
        out << d->mRecurrence;
    }

    for (const Attachment &attachment : qAsConst(d->mAttachments)) {
        out << attachment;
    }

    for (const Alarm::Ptr &alarm : qAsConst(d->mAlarms)) {
        out << alarm;
    }

    for (const Conference &conf : qAsConst(d->mConferences)) {
        out << conf;
    }
}

void Incidence::deserialize(QDataStream &in)
{
    quint32 status, secrecy;
    bool hasRecurrence;
    int attachmentCount, alarmCount, conferencesCount;
    QMap<int, QString> relatedToUid;
    deserializeKDateTimeAsQDateTime(in, d->mCreated);
    in >> d->mRevision >> d->mDescription >> d->mDescriptionIsRich >> d->mSummary >> d->mSummaryIsRich >> d->mLocation >> d->mLocationIsRich >> d->mCategories
        >> d->mResources >> d->mStatusString >> d->mPriority >> d->mSchedulingID >> d->mGeoLatitude >> d->mGeoLongitude >> d->mHasGeo;
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

    auto it = relatedToUid.cbegin(), end = relatedToUid.cend();
    for (; it != end; ++it) {
        d->mRelatedToUid.insert(static_cast<Incidence::RelType>(it.key()), it.value());
    }
}

namespace {
template<typename T>
QVariantList toVariantList(int size, typename QVector<T>::ConstIterator begin, typename QVector<T>::ConstIterator end)
{
    QVariantList l;
    l.reserve(size);
    std::transform(begin, end, std::back_inserter(l), [](const T &val) {
        return QVariant::fromValue(val);
    });
    return l;
}

} // namespace

QVariantList Incidence::attachmentsVariant() const
{
    return toVariantList<Attachment>(d->mAttachments.size(), d->mAttachments.cbegin(), d->mAttachments.cend());
}

QVariantList Incidence::conferencesVariant() const
{
    return toVariantList<Conference>(d->mConferences.size(), d->mConferences.cbegin(), d->mConferences.cend());
}
