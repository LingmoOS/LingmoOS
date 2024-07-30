/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the VCalFormat base class.

  This class implements the vCalendar format. It provides methods for
  loading/saving/converting vCalendar format data into the internal
  representation as Calendar and Incidences.

  @brief
  vCalendar format implementation.

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
*/
#include "vcalformat.h"
#include "calendar.h"
#include "calformat_p.h"
#include "exceptions.h"

#include "kcalendarcore_debug.h"

extern "C" {
#include <libical/vcc.h>
#include <libical/vobject.h>
}

#include <QBitArray>
#include <QFile>
#include <QTextDocument> // for .toHtmlEscaped() and Qt::mightBeRichText()
#include <QTimeZone>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
template<typename K>
void removeAllVCal(QList<QSharedPointer<K>> &c, const QSharedPointer<K> &x)
{
    if (c.count() < 1) {
        return;
    }

    int cnt = c.count(x);
    if (cnt != 1) {
        qCritical() << "There number of relatedTos for this incidence is " << cnt << " (there must be 1 relatedTo only)";
        Q_ASSERT_X(false, "removeAllVCal", "Count is not 1.");
        return;
    }

    c.remove(c.indexOf(x));
}

class KCalendarCore::VCalFormatPrivate : public CalFormatPrivate
{
public:
    Calendar::Ptr mCalendar;
    Event::List mEventsRelate; // Events with relations
    Todo::List mTodosRelate; // To-dos with relations
    QSet<QByteArray> mManuallyWrittenExtensionFields; // X- fields that are manually dumped
};
//@endcond

VCalFormat::VCalFormat()
    : CalFormat(new KCalendarCore::VCalFormatPrivate)
{
}

VCalFormat::~VCalFormat()
{
}

static void mimeErrorHandler(char *e)
{
    qCWarning(KCALCORE_LOG) << "Error parsing vCalendar file:" << e;
}

bool VCalFormat::load(const Calendar::Ptr &calendar, const QString &fileName)
{
    Q_D(VCalFormat);
    d->mCalendar = calendar;

    clearException();

    // this is not necessarily only 1 vcal.  Could be many vcals, or include
    // a vcard...
    registerMimeErrorHandler(&mimeErrorHandler);    // Note: vCalendar error handler provided by libical.
    VObject *vcal = Parse_MIME_FromFileName(const_cast<char *>(QFile::encodeName(fileName).data()));
    registerMimeErrorHandler(nullptr);

    if (!vcal) {
        setException(new Exception(Exception::CalVersionUnknown));
        return false;
    }

    // any other top-level calendar stuff should be added/initialized here

    // put all vobjects into their proper places
    auto savedTimeZoneId = d->mCalendar->timeZoneId();
    populate(vcal);
    d->mCalendar->setTimeZoneId(savedTimeZoneId);

    // clean up from vcal API stuff
    cleanVObjects(vcal);
    cleanStrTbl();

    return true;
}

bool VCalFormat::save(const Calendar::Ptr &calendar, const QString &fileName)
{
    Q_UNUSED(calendar);
    Q_UNUSED(fileName);
    qCWarning(KCALCORE_LOG) << "Saving VCAL is not supported";
    return false;
}

bool VCalFormat::fromRawString(const Calendar::Ptr &calendar, const QByteArray &string)
{
    Q_D(VCalFormat);
    d->mCalendar = calendar;

    if (!string.size()) {
        return false;
    }

    VObject *vcal = Parse_MIME(string.data(), string.size());
    if (!vcal) {
        return false;
    }

    VObjectIterator i;
    initPropIterator(&i, vcal);

    // put all vobjects into their proper places
    auto savedTimeZoneId = d->mCalendar->timeZoneId();
    populate(vcal);
    d->mCalendar->setTimeZoneId(savedTimeZoneId);

    // clean up from vcal API stuff
    cleanVObjects(vcal);
    cleanStrTbl();

    return true;
}

QString VCalFormat::toString(const Calendar::Ptr &calendar)
{
    Q_UNUSED(calendar);

    qCWarning(KCALCORE_LOG) << "Exporting into VCAL is not supported";
    return {};
}

Todo::Ptr VCalFormat::VTodoToEvent(VObject *vtodo)
{
    Q_D(VCalFormat);
    VObject *vo = nullptr;
    VObjectIterator voi;
    char *s = nullptr;

    Todo::Ptr anEvent(new Todo);

    // creation date
    if ((vo = isAPropertyOf(vtodo, VCDCreatedProp)) != nullptr) {
        anEvent->setCreated(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo)))));
        deleteStr(s);
    }

    // unique id
    vo = isAPropertyOf(vtodo, VCUniqueStringProp);
    // while the UID property is preferred, it is not required.  We'll use the
    // default Event UID if none is given.
    if (vo) {
        anEvent->setUid(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo))));
        deleteStr(s);
    }

    // last modification date
    if ((vo = isAPropertyOf(vtodo, VCLastModifiedProp)) != nullptr) {
        anEvent->setLastModified(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo)))));
        deleteStr(s);
    } else {
        anEvent->setLastModified(QDateTime::currentDateTimeUtc());
    }

    // organizer
    // if our extension property for the event's ORGANIZER exists, add it.
    if ((vo = isAPropertyOf(vtodo, ICOrganizerProp)) != nullptr) {
        anEvent->setOrganizer(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo))));
        deleteStr(s);
    } else {
        if (d->mCalendar->owner().name() != QLatin1String("Unknown Name")) {
            anEvent->setOrganizer(d->mCalendar->owner());
        }
    }

    // attendees.
    initPropIterator(&voi, vtodo);
    while (moreIteration(&voi)) {
        vo = nextVObject(&voi);
        if (strcmp(vObjectName(vo), VCAttendeeProp) == 0) {
            Attendee a;
            VObject *vp;
            s = fakeCString(vObjectUStringZValue(vo));
            QString tmpStr = QString::fromUtf8(s);
            deleteStr(s);
            tmpStr = tmpStr.simplified();
            int emailPos1;
            if ((emailPos1 = tmpStr.indexOf(QLatin1Char('<'))) > 0) {
                // both email address and name
                int emailPos2 = tmpStr.lastIndexOf(QLatin1Char('>'));
                a = Attendee(tmpStr.left(emailPos1 - 1), tmpStr.mid(emailPos1 + 1, emailPos2 - (emailPos1 + 1)));
            } else if (tmpStr.indexOf(QLatin1Char('@')) > 0) {
                // just an email address
                a = Attendee(QString(), tmpStr);
            } else {
                // just a name
                // WTF??? Replacing the spaces of a name and using this as email?
                QString email = tmpStr.replace(QLatin1Char(' '), QLatin1Char('.'));
                a = Attendee(tmpStr, email);
            }

            // is there an RSVP property?
            if ((vp = isAPropertyOf(vo, VCRSVPProp)) != nullptr) {
                a.setRSVP(vObjectStringZValue(vp));
            }
            // is there a status property?
            if ((vp = isAPropertyOf(vo, VCStatusProp)) != nullptr) {
                a.setStatus(readStatus(vObjectStringZValue(vp)));
            }
            // add the attendee
            anEvent->addAttendee(a);
        }
    }

    // description for todo
    if ((vo = isAPropertyOf(vtodo, VCDescriptionProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        anEvent->setDescription(QString::fromUtf8(s), Qt::mightBeRichText(QString::fromUtf8(s)));
        deleteStr(s);
    }

    // summary
    if ((vo = isAPropertyOf(vtodo, VCSummaryProp))) {
        s = fakeCString(vObjectUStringZValue(vo));
        anEvent->setSummary(QString::fromUtf8(s), Qt::mightBeRichText(QString::fromUtf8(s)));
        deleteStr(s);
    }

    // location
    if ((vo = isAPropertyOf(vtodo, VCLocationProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        anEvent->setLocation(QString::fromUtf8(s), Qt::mightBeRichText(QString::fromUtf8(s)));
        deleteStr(s);
    }

    // completed
    // was: status
    if ((vo = isAPropertyOf(vtodo, VCStatusProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        if (s && strcmp(s, "COMPLETED") == 0) {
            anEvent->setCompleted(true);
        } else {
            anEvent->setCompleted(false);
        }
        deleteStr(s);
    } else {
        anEvent->setCompleted(false);
    }

    // completion date
    if ((vo = isAPropertyOf(vtodo, VCCompletedProp)) != nullptr) {
        anEvent->setCompleted(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo)))));
        deleteStr(s);
    }

    // priority
    if ((vo = isAPropertyOf(vtodo, VCPriorityProp))) {
        s = fakeCString(vObjectUStringZValue(vo));
        if (s) {
            anEvent->setPriority(atoi(s));
            deleteStr(s);
        }
    }

    anEvent->setAllDay(false);

    // due date
    if ((vo = isAPropertyOf(vtodo, VCDueProp)) != nullptr) {
        anEvent->setDtDue(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo)))));
        deleteStr(s);
        if (anEvent->dtDue().time().hour() == 0 && anEvent->dtDue().time().minute() == 0 && anEvent->dtDue().time().second() == 0) {
            anEvent->setAllDay(true);
        }
    } else {
        anEvent->setDtDue(QDateTime());
    }

    // start time
    if ((vo = isAPropertyOf(vtodo, VCDTstartProp)) != nullptr) {
        anEvent->setDtStart(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo)))));
        deleteStr(s);
        if (anEvent->dtStart().time().hour() == 0 && anEvent->dtStart().time().minute() == 0 && anEvent->dtStart().time().second() == 0) {
            anEvent->setAllDay(true);
        }
    } else {
        anEvent->setDtStart(QDateTime());
    }

    // recurrence stuff
    if ((vo = isAPropertyOf(vtodo, VCRRuleProp)) != nullptr) {
        uint recurrenceType = Recurrence::rNone;
        int recurrenceTypeAbbrLen = 0;

        s = fakeCString(vObjectUStringZValue(vo));
        QString tmpStr = QString::fromUtf8(s);
        deleteStr(s);
        tmpStr = tmpStr.simplified();
        const int tmpStrLen = tmpStr.length();
        if (tmpStrLen > 0) {
            tmpStr = tmpStr.toUpper();
            QStringView prefix = QStringView(tmpStr).left(2);

            // first, read the type of the recurrence
            recurrenceTypeAbbrLen = 1;
            if (tmpStr.at(0) == QLatin1Char('D')) {
                recurrenceType = Recurrence::rDaily;
            } else if (tmpStr.at(0) == QLatin1Char('W')) {
                recurrenceType = Recurrence::rWeekly;
            } else if (tmpStrLen > 1) {
                recurrenceTypeAbbrLen = 2;
                if (prefix == QLatin1String("MP")) {
                    recurrenceType = Recurrence::rMonthlyPos;
                } else if (prefix == QLatin1String("MD")) {
                    recurrenceType = Recurrence::rMonthlyDay;
                } else if (prefix == QLatin1String("YM")) {
                    recurrenceType = Recurrence::rYearlyMonth;
                } else if (prefix == QLatin1String("YD")) {
                    recurrenceType = Recurrence::rYearlyDay;
                }
            }
        }

        if (recurrenceType != Recurrence::rNone) {
            // Immediately after the type is the frequency
            int index = tmpStr.indexOf(QLatin1Char(' '));
            int last = tmpStr.lastIndexOf(QLatin1Char(' ')) + 1; // find last entry
            const int rFreq = QStringView(tmpStr).mid(recurrenceTypeAbbrLen, (index - 1)).toInt();
            ++index; // advance to beginning of stuff after freq

            // Read the type-specific settings
            switch (recurrenceType) {
            case Recurrence::rDaily:
                anEvent->recurrence()->setDaily(rFreq);
                break;

            case Recurrence::rWeekly: {
                QBitArray qba(7);
                QString dayStr;
                if (index == last) {
                    // e.g. W1 #0
                    qba.setBit(anEvent->dtStart().date().dayOfWeek() - 1);
                } else {
                    // e.g. W1 SU #0
                    while (index < last) {
                        dayStr = tmpStr.mid(index, 3);
                        int dayNum = numFromDay(dayStr);
                        if (dayNum >= 0) {
                            qba.setBit(dayNum);
                        }
                        index += 3; // advance to next day, or possibly "#"
                    }
                }
                anEvent->recurrence()->setWeekly(rFreq, qba);
                break;
            }

            case Recurrence::rMonthlyPos: {
                anEvent->recurrence()->setMonthly(rFreq);

                QBitArray qba(7);
                short tmpPos;
                if (index == last) {
                    // e.g. MP1 #0
                    tmpPos = anEvent->dtStart().date().day() / 7 + 1;
                    if (tmpPos == 5) {
                        tmpPos = -1;
                    }
                    qba.setBit(anEvent->dtStart().date().dayOfWeek() - 1);
                    anEvent->recurrence()->addMonthlyPos(tmpPos, qba);
                } else {
                    // e.g. MP1 1+ SU #0
                    while (index < last) {
                        tmpPos = tmpStr.mid(index, 1).toShort();
                        index += 1;
                        if (tmpStr.mid(index, 1) == QLatin1String("-")) {
                            // convert tmpPos to negative
                            tmpPos = 0 - tmpPos;
                        }
                        index += 2; // advance to day(s)
                        while (numFromDay(tmpStr.mid(index, 3)) >= 0) {
                            int dayNum = numFromDay(tmpStr.mid(index, 3));
                            qba.setBit(dayNum);
                            index += 3; // advance to next day, or possibly pos or "#"
                        }
                        anEvent->recurrence()->addMonthlyPos(tmpPos, qba);
                        qba.detach();
                        qba.fill(false); // clear out
                    } // while != "#"
                }
                break;
            }

            case Recurrence::rMonthlyDay:
                anEvent->recurrence()->setMonthly(rFreq);
                if (index == last) {
                    // e.g. MD1 #0
                    short tmpDay = anEvent->dtStart().date().day();
                    anEvent->recurrence()->addMonthlyDate(tmpDay);
                } else {
                    // e.g. MD1 3 #0
                    while (index < last) {
                        int index2 = tmpStr.indexOf(QLatin1Char(' '), index);
                        if ((tmpStr.mid((index2 - 1), 1) == QLatin1String("-")) || (tmpStr.mid((index2 - 1), 1) == QLatin1String("+"))) {
                            index2 = index2 - 1;
                        }
                        short tmpDay = tmpStr.mid(index, (index2 - index)).toShort();
                        index = index2;
                        if (tmpStr.mid(index, 1) == QLatin1String("-")) {
                            tmpDay = 0 - tmpDay;
                        }
                        index += 2; // advance the index;
                        anEvent->recurrence()->addMonthlyDate(tmpDay);
                    } // while != #
                }
                break;

            case Recurrence::rYearlyMonth:
                anEvent->recurrence()->setYearly(rFreq);

                if (index == last) {
                    // e.g. YM1 #0
                    short tmpMonth = anEvent->dtStart().date().month();
                    anEvent->recurrence()->addYearlyMonth(tmpMonth);
                } else {
                    // e.g. YM1 3 #0
                    while (index < last) {
                        int index2 = tmpStr.indexOf(QLatin1Char(' '), index);
                        short tmpMonth = tmpStr.mid(index, (index2 - index)).toShort();
                        index = index2 + 1;
                        anEvent->recurrence()->addYearlyMonth(tmpMonth);
                    } // while != #
                }
                break;

            case Recurrence::rYearlyDay:
                anEvent->recurrence()->setYearly(rFreq);

                if (index == last) {
                    // e.g. YD1 #0
                    short tmpDay = anEvent->dtStart().date().dayOfYear();
                    anEvent->recurrence()->addYearlyDay(tmpDay);
                } else {
                    // e.g. YD1 123 #0
                    while (index < last) {
                        int index2 = tmpStr.indexOf(QLatin1Char(' '), index);
                        short tmpDay = tmpStr.mid(index, (index2 - index)).toShort();
                        index = index2 + 1;
                        anEvent->recurrence()->addYearlyDay(tmpDay);
                    } // while != #
                }
                break;

            default:
                break;
            }

            // find the last field, which is either the duration or the end date
            index = last;
            if (tmpStr.mid(index, 1) == QLatin1String("#")) {
                // Nr of occurrences
                ++index;
                const int rDuration = QStringView(tmpStr).mid(index, tmpStr.length() - index).toInt();
                if (rDuration > 0) {
                    anEvent->recurrence()->setDuration(rDuration);
                }
            } else if (tmpStr.indexOf(QLatin1Char('T'), index) != -1) {
                QDateTime rEndDate = ISOToQDateTime(tmpStr.mid(index, tmpStr.length() - index));
                anEvent->recurrence()->setEndDateTime(rEndDate);
            }
        } else {
            qCDebug(KCALCORE_LOG) << "we don't understand this type of recurrence!";
        } // if known recurrence type
    } // repeats

    // recurrence exceptions
    if ((vo = isAPropertyOf(vtodo, VCExpDateProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        const QStringList exDates = QString::fromUtf8(s).split(QLatin1Char(','));
        for (const auto &date : exDates) {
            const QDateTime exDate = ISOToQDateTime(date);
            if (exDate.time().hour() == 0 && exDate.time().minute() == 0 && exDate.time().second() == 0) {
                anEvent->recurrence()->addExDate(ISOToQDate(date));
            } else {
                anEvent->recurrence()->addExDateTime(exDate);
            }
        }
        deleteStr(s);
    }

    // alarm stuff
    if ((vo = isAPropertyOf(vtodo, VCDAlarmProp))) {
        Alarm::Ptr alarm;
        VObject *a = isAPropertyOf(vo, VCRunTimeProp);
        VObject *b = isAPropertyOf(vo, VCDisplayStringProp);

        if (a || b) {
            alarm = anEvent->newAlarm();
            if (a) {
                alarm->setTime(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(a)))));
                deleteStr(s);
            }
            alarm->setEnabled(true);
            if (b) {
                s = fakeCString(vObjectUStringZValue(b));
                alarm->setDisplayAlarm(QString::fromUtf8(s));
                deleteStr(s);
            } else {
                alarm->setDisplayAlarm(QString());
            }
        }
    }

    if ((vo = isAPropertyOf(vtodo, VCAAlarmProp))) {
        Alarm::Ptr alarm;
        VObject *a;
        VObject *b;
        a = isAPropertyOf(vo, VCRunTimeProp);
        b = isAPropertyOf(vo, VCAudioContentProp);

        if (a || b) {
            alarm = anEvent->newAlarm();
            if (a) {
                alarm->setTime(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(a)))));
                deleteStr(s);
            }
            alarm->setEnabled(true);
            if (b) {
                s = fakeCString(vObjectUStringZValue(b));
                alarm->setAudioAlarm(QFile::decodeName(s));
                deleteStr(s);
            } else {
                alarm->setAudioAlarm(QString());
            }
        }
    }

    if ((vo = isAPropertyOf(vtodo, VCPAlarmProp))) {
        Alarm::Ptr alarm;
        VObject *a = isAPropertyOf(vo, VCRunTimeProp);
        VObject *b = isAPropertyOf(vo, VCProcedureNameProp);

        if (a || b) {
            alarm = anEvent->newAlarm();
            if (a) {
                alarm->setTime(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(a)))));
                deleteStr(s);
            }
            alarm->setEnabled(true);

            if (b) {
                s = fakeCString(vObjectUStringZValue(b));
                alarm->setProcedureAlarm(QFile::decodeName(s));
                deleteStr(s);
            } else {
                alarm->setProcedureAlarm(QString());
            }
        }
    }

    // related todo
    if ((vo = isAPropertyOf(vtodo, VCRelatedToProp)) != nullptr) {
        anEvent->setRelatedTo(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo))));
        deleteStr(s);
        d->mTodosRelate.append(anEvent);
    }

    // secrecy
    Incidence::Secrecy secrecy = Incidence::SecrecyPublic;
    if ((vo = isAPropertyOf(vtodo, VCClassProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        if (s && strcmp(s, "PRIVATE") == 0) {
            secrecy = Incidence::SecrecyPrivate;
        } else if (s && strcmp(s, "CONFIDENTIAL") == 0) {
            secrecy = Incidence::SecrecyConfidential;
        }
        deleteStr(s);
    }
    anEvent->setSecrecy(secrecy);

    // categories
    if ((vo = isAPropertyOf(vtodo, VCCategoriesProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        QString categories = QString::fromUtf8(s);
        deleteStr(s);
        QStringList tmpStrList = categories.split(QLatin1Char(';'));
        anEvent->setCategories(tmpStrList);
    }

    return anEvent;
}

Event::Ptr VCalFormat::VEventToEvent(VObject *vevent)
{
    Q_D(VCalFormat);
    VObject *vo = nullptr;
    VObjectIterator voi;
    char *s = nullptr;

    Event::Ptr anEvent(new Event);

    // creation date
    if ((vo = isAPropertyOf(vevent, VCDCreatedProp)) != nullptr) {
        anEvent->setCreated(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo)))));
        deleteStr(s);
    }

    // unique id
    vo = isAPropertyOf(vevent, VCUniqueStringProp);
    // while the UID property is preferred, it is not required.  We'll use the
    // default Event UID if none is given.
    if (vo) {
        anEvent->setUid(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo))));
        deleteStr(s);
    }

    // revision
    // again NSCAL doesn't give us much to work with, so we improvise...
    anEvent->setRevision(0);
    if ((vo = isAPropertyOf(vevent, VCSequenceProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        if (s) {
            anEvent->setRevision(atoi(s));
            deleteStr(s);
        }
    }

    // last modification date
    if ((vo = isAPropertyOf(vevent, VCLastModifiedProp)) != nullptr) {
        anEvent->setLastModified(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo)))));
        deleteStr(s);
    } else {
        anEvent->setLastModified(QDateTime::currentDateTimeUtc());
    }

    // organizer
    // if our extension property for the event's ORGANIZER exists, add it.
    if ((vo = isAPropertyOf(vevent, ICOrganizerProp)) != nullptr) {
        // FIXME:  Also use the full name, not just the email address
        anEvent->setOrganizer(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo))));
        deleteStr(s);
    } else {
        if (d->mCalendar->owner().name() != QLatin1String("Unknown Name")) {
            anEvent->setOrganizer(d->mCalendar->owner());
        }
    }

    // deal with attendees.
    initPropIterator(&voi, vevent);
    while (moreIteration(&voi)) {
        vo = nextVObject(&voi);
        if (strcmp(vObjectName(vo), VCAttendeeProp) == 0) {
            Attendee a;
            VObject *vp = nullptr;
            s = fakeCString(vObjectUStringZValue(vo));
            QString tmpStr = QString::fromUtf8(s);
            deleteStr(s);
            tmpStr = tmpStr.simplified();
            int emailPos1;
            if ((emailPos1 = tmpStr.indexOf(QLatin1Char('<'))) > 0) {
                // both email address and name
                int emailPos2 = tmpStr.lastIndexOf(QLatin1Char('>'));
                a = Attendee(tmpStr.left(emailPos1 - 1), tmpStr.mid(emailPos1 + 1, emailPos2 - (emailPos1 + 1)));
            } else if (tmpStr.indexOf(QLatin1Char('@')) > 0) {
                // just an email address
                a = Attendee(QString(), tmpStr);
            } else {
                // just a name
                QString email = tmpStr.replace(QLatin1Char(' '), QLatin1Char('.'));
                a = Attendee(tmpStr, email);
            }

            // is there an RSVP property?
            if ((vp = isAPropertyOf(vo, VCRSVPProp)) != nullptr) {
                a.setRSVP(vObjectStringZValue(vp));
            }
            // is there a status property?
            if ((vp = isAPropertyOf(vo, VCStatusProp)) != nullptr) {
                a.setStatus(readStatus(vObjectStringZValue(vp)));
            }
            // add the attendee
            anEvent->addAttendee(a);
        }
    }

    // This isn't strictly true.  An event that doesn't have a start time
    // or an end time isn't all-day, it has an anchor in time but it doesn't
    // "take up" any time.
    /*if ((isAPropertyOf(vevent, VCDTstartProp) == 0) ||
        (isAPropertyOf(vevent, VCDTendProp) == 0)) {
      anEvent->setAllDay(true);
      } else {
      }*/

    anEvent->setAllDay(false);

    // start time
    if ((vo = isAPropertyOf(vevent, VCDTstartProp)) != nullptr) {
        anEvent->setDtStart(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo)))));
        deleteStr(s);

        if (anEvent->dtStart().time().hour() == 0 && anEvent->dtStart().time().minute() == 0 && anEvent->dtStart().time().second() == 0) {
            anEvent->setAllDay(true);
        }
    }

    // stop time
    if ((vo = isAPropertyOf(vevent, VCDTendProp)) != nullptr) {
        anEvent->setDtEnd(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo)))));
        deleteStr(s);

        if (anEvent->dtEnd().time().hour() == 0 && anEvent->dtEnd().time().minute() == 0 && anEvent->dtEnd().time().second() == 0) {
            anEvent->setAllDay(true);
        }
    }

    // at this point, there should be at least a start or end time.
    // fix up for events that take up no time but have a time associated
    if (!isAPropertyOf(vevent, VCDTstartProp)) {
        anEvent->setDtStart(anEvent->dtEnd());
    }
    if (!isAPropertyOf(vevent, VCDTendProp)) {
        anEvent->setDtEnd(anEvent->dtStart());
    }

    ///////////////////////////////////////////////////////////////////////////

    // recurrence stuff
    if ((vo = isAPropertyOf(vevent, VCRRuleProp)) != nullptr) {
        uint recurrenceType = Recurrence::rNone;
        int recurrenceTypeAbbrLen = 0;

        QString tmpStr = (QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo))));
        deleteStr(s);
        tmpStr = tmpStr.simplified();
        const int tmpStrLen = tmpStr.length();
        if (tmpStrLen > 0) {
            tmpStr = tmpStr.toUpper();
            const QStringView prefix(tmpStr.left(2));

            // first, read the type of the recurrence
            recurrenceTypeAbbrLen = 1;
            if (tmpStr.at(0) == QLatin1Char('D')) {
                recurrenceType = Recurrence::rDaily;
            } else if (tmpStr.at(0) == QLatin1Char('W')) {
                recurrenceType = Recurrence::rWeekly;
            } else if (tmpStrLen > 1) {
                recurrenceTypeAbbrLen = 2;
                if (prefix == QLatin1String("MP")) {
                    recurrenceType = Recurrence::rMonthlyPos;
                } else if (prefix == QLatin1String("MD")) {
                    recurrenceType = Recurrence::rMonthlyDay;
                } else if (prefix == QLatin1String("YM")) {
                    recurrenceType = Recurrence::rYearlyMonth;
                } else if (prefix == QLatin1String("YD")) {
                    recurrenceType = Recurrence::rYearlyDay;
                }
            }
        }

        if (recurrenceType != Recurrence::rNone) {
            // Immediately after the type is the frequency
            int index = tmpStr.indexOf(QLatin1Char(' '));
            int last = tmpStr.lastIndexOf(QLatin1Char(' ')) + 1; // find last entry
            const int rFreq = QStringView(tmpStr).mid(recurrenceTypeAbbrLen, (index - 1)).toInt();
            ++index; // advance to beginning of stuff after freq

            // Read the type-specific settings
            switch (recurrenceType) {
            case Recurrence::rDaily:
                anEvent->recurrence()->setDaily(rFreq);
                break;

            case Recurrence::rWeekly: {
                QBitArray qba(7);
                QString dayStr;
                if (index == last) {
                    // e.g. W1 #0
                    qba.setBit(anEvent->dtStart().date().dayOfWeek() - 1);
                } else {
                    // e.g. W1 SU #0
                    while (index < last) {
                        dayStr = tmpStr.mid(index, 3);
                        int dayNum = numFromDay(dayStr);
                        if (dayNum >= 0) {
                            qba.setBit(dayNum);
                        }
                        index += 3; // advance to next day, or possibly "#"
                    }
                }
                anEvent->recurrence()->setWeekly(rFreq, qba);
                break;
            }

            case Recurrence::rMonthlyPos: {
                anEvent->recurrence()->setMonthly(rFreq);

                QBitArray qba(7);
                short tmpPos;
                if (index == last) {
                    // e.g. MP1 #0
                    tmpPos = anEvent->dtStart().date().day() / 7 + 1;
                    if (tmpPos == 5) {
                        tmpPos = -1;
                    }
                    qba.setBit(anEvent->dtStart().date().dayOfWeek() - 1);
                    anEvent->recurrence()->addMonthlyPos(tmpPos, qba);
                } else {
                    // e.g. MP1 1+ SU #0
                    while (index < last) {
                        tmpPos = tmpStr.mid(index, 1).toShort();
                        index += 1;
                        if (tmpStr.mid(index, 1) == QLatin1String("-")) {
                            // convert tmpPos to negative
                            tmpPos = 0 - tmpPos;
                        }
                        index += 2; // advance to day(s)
                        while (numFromDay(tmpStr.mid(index, 3)) >= 0) {
                            int dayNum = numFromDay(tmpStr.mid(index, 3));
                            qba.setBit(dayNum);
                            index += 3; // advance to next day, or possibly pos or "#"
                        }
                        anEvent->recurrence()->addMonthlyPos(tmpPos, qba);
                        qba.detach();
                        qba.fill(false); // clear out
                    } // while != "#"
                }
                break;
            }

            case Recurrence::rMonthlyDay:
                anEvent->recurrence()->setMonthly(rFreq);
                if (index == last) {
                    // e.g. MD1 #0
                    short tmpDay = anEvent->dtStart().date().day();
                    anEvent->recurrence()->addMonthlyDate(tmpDay);
                } else {
                    // e.g. MD1 3 #0
                    while (index < last) {
                        int index2 = tmpStr.indexOf(QLatin1Char(' '), index);
                        if ((tmpStr.mid((index2 - 1), 1) == QLatin1String("-")) || (tmpStr.mid((index2 - 1), 1) == QLatin1String("+"))) {
                            index2 = index2 - 1;
                        }
                        short tmpDay = tmpStr.mid(index, (index2 - index)).toShort();
                        index = index2;
                        if (tmpStr.mid(index, 1) == QLatin1String("-")) {
                            tmpDay = 0 - tmpDay;
                        }
                        index += 2; // advance the index;
                        anEvent->recurrence()->addMonthlyDate(tmpDay);
                    } // while != #
                }
                break;

            case Recurrence::rYearlyMonth:
                anEvent->recurrence()->setYearly(rFreq);

                if (index == last) {
                    // e.g. YM1 #0
                    short tmpMonth = anEvent->dtStart().date().month();
                    anEvent->recurrence()->addYearlyMonth(tmpMonth);
                } else {
                    // e.g. YM1 3 #0
                    while (index < last) {
                        int index2 = tmpStr.indexOf(QLatin1Char(' '), index);
                        short tmpMonth = tmpStr.mid(index, (index2 - index)).toShort();
                        index = index2 + 1;
                        anEvent->recurrence()->addYearlyMonth(tmpMonth);
                    } // while != #
                }
                break;

            case Recurrence::rYearlyDay:
                anEvent->recurrence()->setYearly(rFreq);

                if (index == last) {
                    // e.g. YD1 #0
                    const int tmpDay = anEvent->dtStart().date().dayOfYear();
                    anEvent->recurrence()->addYearlyDay(tmpDay);
                } else {
                    // e.g. YD1 123 #0
                    while (index < last) {
                        int index2 = tmpStr.indexOf(QLatin1Char(' '), index);
                        short tmpDay = tmpStr.mid(index, (index2 - index)).toShort();
                        index = index2 + 1;
                        anEvent->recurrence()->addYearlyDay(tmpDay);
                    } // while != #
                }
                break;

            default:
                break;
            }

            // find the last field, which is either the duration or the end date
            index = last;
            if (tmpStr.mid(index, 1) == QLatin1String("#")) {
                // Nr of occurrences
                ++index;
                const int rDuration = QStringView(tmpStr).mid(index, tmpStr.length() - index).toInt();
                if (rDuration > 0) {
                    anEvent->recurrence()->setDuration(rDuration);
                }
            } else if (tmpStr.indexOf(QLatin1Char('T'), index) != -1) {
                QDateTime rEndDate = ISOToQDateTime(tmpStr.mid(index, tmpStr.length() - index));
                anEvent->recurrence()->setEndDateTime(rEndDate);
            }
            // anEvent->recurrence()->dump();

        } else {
            qCDebug(KCALCORE_LOG) << "we don't understand this type of recurrence!";
        } // if known recurrence type
    } // repeats

    // recurrence exceptions
    if ((vo = isAPropertyOf(vevent, VCExpDateProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        const QStringList exDates = QString::fromUtf8(s).split(QLatin1Char(','));
        for (const auto &date : exDates) {
            const QDateTime exDate = ISOToQDateTime(date);
            if (exDate.time().hour() == 0 && exDate.time().minute() == 0 && exDate.time().second() == 0) {
                anEvent->recurrence()->addExDate(ISOToQDate(date));
            } else {
                anEvent->recurrence()->addExDateTime(exDate);
            }
        }
        deleteStr(s);
    }

    // summary
    if ((vo = isAPropertyOf(vevent, VCSummaryProp))) {
        s = fakeCString(vObjectUStringZValue(vo));
        anEvent->setSummary(QString::fromUtf8(s), Qt::mightBeRichText(QString::fromUtf8(s)));
        deleteStr(s);
    }

    // description
    if ((vo = isAPropertyOf(vevent, VCDescriptionProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        bool isRich = Qt::mightBeRichText(QString::fromUtf8(s));
        if (!anEvent->description().isEmpty()) {
            anEvent->setDescription(anEvent->description() + QLatin1Char('\n') + QString::fromUtf8(s), isRich);
        } else {
            anEvent->setDescription(QString::fromUtf8(s), isRich);
        }
        deleteStr(s);
    }

    // location
    if ((vo = isAPropertyOf(vevent, VCLocationProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        anEvent->setLocation(QString::fromUtf8(s), Qt::mightBeRichText(QString::fromUtf8(s)));
        deleteStr(s);
    }

    // some stupid vCal exporters ignore the standard and use Description
    // instead of Summary for the default field.  Correct for this.
    if (anEvent->summary().isEmpty() && !(anEvent->description().isEmpty())) {
        QString tmpStr = anEvent->description().simplified();
        anEvent->setDescription(QString());
        anEvent->setSummary(tmpStr);
    }

#if 0
    // status
    if ((vo = isAPropertyOf(vevent, VCStatusProp)) != 0) {
        QString tmpStr(s = fakeCString(vObjectUStringZValue(vo)));
        deleteStr(s);
// TODO: Define Event status
//    anEvent->setStatus( tmpStr );
    } else {
//    anEvent->setStatus( "NEEDS ACTION" );
    }
#endif

    // secrecy
    Incidence::Secrecy secrecy = Incidence::SecrecyPublic;
    if ((vo = isAPropertyOf(vevent, VCClassProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        if (s && strcmp(s, "PRIVATE") == 0) {
            secrecy = Incidence::SecrecyPrivate;
        } else if (s && strcmp(s, "CONFIDENTIAL") == 0) {
            secrecy = Incidence::SecrecyConfidential;
        }
        deleteStr(s);
    }
    anEvent->setSecrecy(secrecy);

    // categories
    if ((vo = isAPropertyOf(vevent, VCCategoriesProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        QString categories = QString::fromUtf8(s);
        deleteStr(s);
        QStringList tmpStrList = categories.split(QLatin1Char(','));
        anEvent->setCategories(tmpStrList);
    }

    // attachments
    initPropIterator(&voi, vevent);
    while (moreIteration(&voi)) {
        vo = nextVObject(&voi);
        if (strcmp(vObjectName(vo), VCAttachProp) == 0) {
            s = fakeCString(vObjectUStringZValue(vo));
            anEvent->addAttachment(Attachment(QString::fromUtf8(s)));
            deleteStr(s);
        }
    }

    // resources
    if ((vo = isAPropertyOf(vevent, VCResourcesProp)) != nullptr) {
        QString resources = (QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo))));
        deleteStr(s);
        QStringList tmpStrList = resources.split(QLatin1Char(';'));
        anEvent->setResources(tmpStrList);
    }

    // alarm stuff
    if ((vo = isAPropertyOf(vevent, VCDAlarmProp))) {
        Alarm::Ptr alarm;
        VObject *a = isAPropertyOf(vo, VCRunTimeProp);
        VObject *b = isAPropertyOf(vo, VCDisplayStringProp);

        if (a || b) {
            alarm = anEvent->newAlarm();
            if (a) {
                alarm->setTime(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(a)))));
                deleteStr(s);
            }
            alarm->setEnabled(true);

            if (b) {
                s = fakeCString(vObjectUStringZValue(b));
                alarm->setDisplayAlarm(QString::fromUtf8(s));
                deleteStr(s);
            } else {
                alarm->setDisplayAlarm(QString());
            }
        }
    }

    if ((vo = isAPropertyOf(vevent, VCAAlarmProp))) {
        Alarm::Ptr alarm;
        VObject *a;
        VObject *b;
        a = isAPropertyOf(vo, VCRunTimeProp);
        b = isAPropertyOf(vo, VCAudioContentProp);

        if (a || b) {
            alarm = anEvent->newAlarm();
            if (a) {
                alarm->setTime(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(a)))));
                deleteStr(s);
            }
            alarm->setEnabled(true);

            if (b) {
                s = fakeCString(vObjectUStringZValue(b));
                alarm->setAudioAlarm(QFile::decodeName(s));
                deleteStr(s);
            } else {
                alarm->setAudioAlarm(QString());
            }
        }
    }

    if ((vo = isAPropertyOf(vevent, VCPAlarmProp))) {
        Alarm::Ptr alarm;
        VObject *a;
        VObject *b;
        a = isAPropertyOf(vo, VCRunTimeProp);
        b = isAPropertyOf(vo, VCProcedureNameProp);

        if (a || b) {
            alarm = anEvent->newAlarm();
            if (a) {
                alarm->setTime(ISOToQDateTime(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(a)))));
                deleteStr(s);
            }
            alarm->setEnabled(true);

            if (b) {
                s = fakeCString(vObjectUStringZValue(b));
                alarm->setProcedureAlarm(QFile::decodeName(s));
                deleteStr(s);
            } else {
                alarm->setProcedureAlarm(QString());
            }
        }
    }

    // priority
    if ((vo = isAPropertyOf(vevent, VCPriorityProp))) {
        s = fakeCString(vObjectUStringZValue(vo));
        if (s) {
            anEvent->setPriority(atoi(s));
            deleteStr(s);
        }
    }

    // transparency
    if ((vo = isAPropertyOf(vevent, VCTranspProp)) != nullptr) {
        s = fakeCString(vObjectUStringZValue(vo));
        if (s) {
            int i = atoi(s);
            anEvent->setTransparency(i == 1 ? Event::Transparent : Event::Opaque);
            deleteStr(s);
        }
    }

    // related event
    if ((vo = isAPropertyOf(vevent, VCRelatedToProp)) != nullptr) {
        anEvent->setRelatedTo(QString::fromUtf8(s = fakeCString(vObjectUStringZValue(vo))));
        deleteStr(s);
        d->mEventsRelate.append(anEvent);
    }

    /* Rest of the custom properties */
    readCustomProperties(vevent, anEvent);

    return anEvent;
}

QString VCalFormat::parseTZ(const QByteArray &timezone) const
{
    // qCDebug(KCALCORE_LOG) << timezone;
    QString pZone = QString::fromUtf8(timezone.mid(timezone.indexOf("TZID:VCAL") + 9));
    return pZone.mid(0, pZone.indexOf(QLatin1Char('\n')));
}

QString VCalFormat::parseDst(QByteArray &timezone) const
{
    if (!timezone.contains("BEGIN:DAYLIGHT")) {
        return QString();
    }

    timezone = timezone.mid(timezone.indexOf("BEGIN:DAYLIGHT"));
    timezone = timezone.mid(timezone.indexOf("TZNAME:") + 7);
    QString sStart = QString::fromUtf8(timezone.mid(0, (timezone.indexOf("COMMENT:"))));
    sStart.chop(2);
    timezone = timezone.mid(timezone.indexOf("TZOFFSETTO:") + 11);
    QString sOffset = QString::fromUtf8(timezone.mid(0, (timezone.indexOf("DTSTART:"))));
    sOffset.chop(2);
    sOffset.insert(3, QLatin1Char(':'));
    timezone = timezone.mid(timezone.indexOf("TZNAME:") + 7);
    QString sEnd = QString::fromUtf8(timezone.mid(0, (timezone.indexOf("COMMENT:"))));
    sEnd.chop(2);

    return QStringLiteral("TRUE;") + sOffset + QLatin1Char(';') + sStart + QLatin1Char(';') + sEnd + QLatin1String(";;");
}

QString VCalFormat::qDateToISO(const QDate &qd)
{
    if (!qd.isValid()) {
        return QString();
    }

    return QString::asprintf("%.2d%.2d%.2d", qd.year(), qd.month(), qd.day());
}

QString VCalFormat::qDateTimeToISO(const QDateTime &dt, bool zulu)
{
    Q_D(VCalFormat);
    if (!dt.isValid()) {
        return QString();
    }

    QDateTime tmpDT;
    if (zulu) {
        tmpDT = dt.toUTC();
    } else {
        tmpDT = dt.toTimeZone(d->mCalendar->timeZone());
    }
    QString tmpStr = QString::asprintf("%.2d%.2d%.2dT%.2d%.2d%.2d",
                                       tmpDT.date().year(),
                                       tmpDT.date().month(),
                                       tmpDT.date().day(),
                                       tmpDT.time().hour(),
                                       tmpDT.time().minute(),
                                       tmpDT.time().second());
    if (zulu || dt.timeZone() == QTimeZone::utc()) {
        tmpStr += QLatin1Char('Z');
    }
    return tmpStr;
}

QDateTime VCalFormat::ISOToQDateTime(const QString &dtStr)
{
    Q_D(VCalFormat);
    auto noAllocString = QStringView{dtStr};

    int year = noAllocString.left(4).toInt();
    int month = noAllocString.mid(4, 2).toInt();
    int day = noAllocString.mid(6, 2).toInt();
    int hour = noAllocString.mid(9, 2).toInt();
    int minute = noAllocString.mid(11, 2).toInt();
    int second = noAllocString.mid(13, 2).toInt();

    QDate tmpDate;
    tmpDate.setDate(year, month, day);
    QTime tmpTime;
    tmpTime.setHMS(hour, minute, second);

    if (tmpDate.isValid() && tmpTime.isValid()) {
        // correct for GMT if string is in Zulu format
        if (dtStr.at(dtStr.length() - 1) == QLatin1Char('Z')) {
            return QDateTime(tmpDate, tmpTime, QTimeZone::UTC);
        } else {
            return QDateTime(tmpDate, tmpTime, d->mCalendar->timeZone());
        }
    }

    return {};
}

QDate VCalFormat::ISOToQDate(const QString &dateStr)
{
    auto noAllocString = QStringView{dateStr};

    const int year = noAllocString.left(4).toInt();
    const int month = noAllocString.mid(4, 2).toInt();
    const int day = noAllocString.mid(6, 2).toInt();

    return QDate(year, month, day);
}

bool VCalFormat::parseTZOffsetISO8601(const QString &s, int &result)
{
    // ISO8601 format(s):
    // +- hh : mm
    // +- hh mm
    // +- hh

    // We also accept broken one without +
    int mod = 1;
    int v = 0;
    const QString str = s.trimmed();
    int ofs = 0;
    result = 0;

    // Check for end
    if (str.size() <= ofs) {
        return false;
    }
    if (str[ofs] == QLatin1Char('-')) {
        mod = -1;
        ofs++;
    } else if (str[ofs] == QLatin1Char('+')) {
        ofs++;
    }
    if (str.size() <= ofs) {
        return false;
    }

    // Make sure next two values are numbers
    bool ok;

    if (str.size() < (ofs + 2)) {
        return false;
    }

    v = QStringView(str).mid(ofs, 2).toInt(&ok) * 60;
    if (!ok) {
        return false;
    }
    ofs += 2;

    if (str.size() > ofs) {
        if (str[ofs] == QLatin1Char(':')) {
            ofs++;
        }
        if (str.size() > ofs) {
            if (str.size() < (ofs + 2)) {
                return false;
            }
            v += QStringView(str).mid(ofs, 2).toInt(&ok);
            if (!ok) {
                return false;
            }
        }
    }
    result = v * mod * 60;
    return true;
}

// take a raw vcalendar (i.e. from a file on disk, clipboard, etc. etc.
// and break it down from it's tree-like format into the dictionary format
// that is used internally in the VCalFormat.
void VCalFormat::populate(VObject *vcal)
{
    Q_D(VCalFormat);
    // this function will populate the caldict dictionary and other event
    // lists. It turns vevents into Events and then inserts them.

    VObjectIterator i;
    VObject *curVO;
    Event::Ptr anEvent;
    bool hasTimeZone = false; // The calendar came with a TZ and not UTC
    QTimeZone previousZone; // If we add a new TZ we should leave the spec as it was before

    if ((curVO = isAPropertyOf(vcal, ICMethodProp)) != nullptr) {
        char *methodType = fakeCString(vObjectUStringZValue(curVO));
        // qCDebug(KCALCORE_LOG) << "This calendar is an iTIP transaction of type '" << methodType << "'";
        deleteStr(methodType);
    }

    // warn the user that we might have trouble reading non-known calendar.
    if ((curVO = isAPropertyOf(vcal, VCProdIdProp)) != nullptr) {
        char *s = fakeCString(vObjectUStringZValue(curVO));
        if (!s || strcmp(productId().toUtf8().constData(), s) != 0) {
            qCDebug(KCALCORE_LOG) << "This vCalendar file was not created by KOrganizer or"
                                  << "any other product we support. Loading anyway...";
        }
        setLoadedProductId(QString::fromUtf8(s));
        deleteStr(s);
    }

    // warn the user we might have trouble reading this unknown version.
    if ((curVO = isAPropertyOf(vcal, VCVersionProp)) != nullptr) {
        char *s = fakeCString(vObjectUStringZValue(curVO));
        if (!s || strcmp(_VCAL_VERSION, s) != 0) {
            qCDebug(KCALCORE_LOG) << "This vCalendar file has version" << s << "We only support" << _VCAL_VERSION;
        }
        deleteStr(s);
    }

    // set the time zone (this is a property of the view, so just discard!)
    if ((curVO = isAPropertyOf(vcal, VCTimeZoneProp)) != nullptr) {
        char *s = fakeCString(vObjectUStringZValue(curVO));
        QString ts = QString::fromUtf8(s);
        QString name = QLatin1String("VCAL") + ts;
        deleteStr(s);

        // TODO: While using the timezone-offset + vcal as timezone is is
        // most likely unique, we should REALLY actually create something
        // like vcal-tzoffset-daylightoffsets, or better yet,
        // vcal-hash<the former>

        QStringList tzList;
        QString tz;
        int utcOffset;
        if (parseTZOffsetISO8601(ts, utcOffset)) {
            // qCDebug(KCALCORE_LOG) << "got standard offset" << ts << utcOffset;
            // standard from tz
            // starting date for now 01011900
            QDateTime dt = QDateTime(QDateTime(QDate(1900, 1, 1), QTime(0, 0, 0)));
            tz = QStringLiteral("STD;%1;false;%2").arg(QString::number(utcOffset), dt.toString());
            tzList.append(tz);

            // go through all the daylight tags
            initPropIterator(&i, vcal);
            while (moreIteration(&i)) {
                curVO = nextVObject(&i);
                if (strcmp(vObjectName(curVO), VCDayLightProp) == 0) {
                    char *s = fakeCString(vObjectUStringZValue(curVO));
                    QString dst = QLatin1String(s);
                    QStringList argl = dst.split(QLatin1Char(','));
                    deleteStr(s);

                    // Too short -> not interesting
                    if (argl.size() < 4) {
                        continue;
                    }

                    // We don't care about the non-DST periods
                    if (argl[0] != QLatin1String("TRUE")) {
                        continue;
                    }

                    int utcOffsetDst;
                    if (parseTZOffsetISO8601(argl[1], utcOffsetDst)) {
                        // qCDebug(KCALCORE_LOG) << "got DST offset" << argl[1] << utcOffsetDst;
                        // standard
                        QString strEndDate = argl[3];
                        QDateTime endDate = ISOToQDateTime(strEndDate);
                        // daylight
                        QString strStartDate = argl[2];
                        QDateTime startDate = ISOToQDateTime(strStartDate);

                        QString strRealEndDate = strEndDate;
                        QString strRealStartDate = strStartDate;
                        QDateTime realEndDate = endDate;
                        QDateTime realStartDate = startDate;
                        // if we get dates for some reason in wrong order, earlier is used for dst
                        if (endDate < startDate) {
                            strRealEndDate = strStartDate;
                            strRealStartDate = strEndDate;
                            realEndDate = startDate;
                            realStartDate = endDate;
                        }
                        tz = QStringLiteral("%1;%2;false;%3").arg(strRealEndDate, QString::number(utcOffset), realEndDate.toString());
                        tzList.append(tz);

                        tz = QStringLiteral("%1;%2;true;%3").arg(strRealStartDate, QString::number(utcOffsetDst), realStartDate.toString());
                        tzList.append(tz);
                    } else {
                        qCDebug(KCALCORE_LOG) << "unable to parse dst" << argl[1];
                    }
                }
            }
            if (!QTimeZone::isTimeZoneIdAvailable(name.toLatin1())) {
                qCDebug(KCALCORE_LOG) << "zone is not valid, parsing error" << tzList;
            } else {
                previousZone = d->mCalendar->timeZone();
                d->mCalendar->setTimeZoneId(name.toUtf8());
                hasTimeZone = true;
            }
        } else {
            qCDebug(KCALCORE_LOG) << "unable to parse tzoffset" << ts;
        }
    }

    // Store all events with a relatedTo property in a list for post-processing
    d->mEventsRelate.clear();
    d->mTodosRelate.clear();

    initPropIterator(&i, vcal);

    // go through all the vobjects in the vcal
    while (moreIteration(&i)) {
        curVO = nextVObject(&i);

        /************************************************************************/

        // now, check to see that the object is an event or todo.
        if (strcmp(vObjectName(curVO), VCEventProp) == 0) {
            if (!isAPropertyOf(curVO, VCDTstartProp) && !isAPropertyOf(curVO, VCDTendProp)) {
                qCDebug(KCALCORE_LOG) << "found a VEvent with no DTSTART and no DTEND! Skipping...";
                goto SKIP;
            }

            anEvent = VEventToEvent(curVO);
            if (anEvent) {
                if (hasTimeZone && !anEvent->allDay() && anEvent->dtStart().timeZone() == QTimeZone::utc()) {
                    // This sounds stupid but is how others are doing it, so here
                    // we go. If there is a TZ in the VCALENDAR even if the dtStart
                    // and dtend are in UTC, clients interpret it using also the TZ defined
                    // in the Calendar. I know it sounds braindead but oh well
                    int utcOffSet = anEvent->dtStart().offsetFromUtc();
                    QDateTime dtStart(anEvent->dtStart().addSecs(utcOffSet));
                    dtStart.setTimeZone(d->mCalendar->timeZone());
                    QDateTime dtEnd(anEvent->dtEnd().addSecs(utcOffSet));
                    dtEnd.setTimeZone(d->mCalendar->timeZone());
                    anEvent->setDtStart(dtStart);
                    anEvent->setDtEnd(dtEnd);
                }
                Event::Ptr old =
                    !anEvent->hasRecurrenceId() ? d->mCalendar->event(anEvent->uid()) : d->mCalendar->event(anEvent->uid(), anEvent->recurrenceId());

                if (old) {
                    if (anEvent->revision() > old->revision()) {
                        d->mCalendar->deleteEvent(old); // move old to deleted
                        removeAllVCal(d->mEventsRelate, old);
                        d->mCalendar->addEvent(anEvent); // and replace it with this one
                    }
                } else {
                    d->mCalendar->addEvent(anEvent); // just add this one
                }
            }
        } else if (strcmp(vObjectName(curVO), VCTodoProp) == 0) {
            Todo::Ptr aTodo = VTodoToEvent(curVO);
            if (aTodo) {
                if (hasTimeZone && !aTodo->allDay() && aTodo->dtStart().timeZone() == QTimeZone::utc()) {
                    // This sounds stupid but is how others are doing it, so here
                    // we go. If there is a TZ in the VCALENDAR even if the dtStart
                    // and dtend are in UTC, clients interpret it using also the TZ defined
                    // in the Calendar. I know it sounds braindead but oh well
                    int utcOffSet = aTodo->dtStart().offsetFromUtc();
                    QDateTime dtStart(aTodo->dtStart().addSecs(utcOffSet));
                    dtStart.setTimeZone(d->mCalendar->timeZone());
                    aTodo->setDtStart(dtStart);
                    if (aTodo->hasDueDate()) {
                        QDateTime dtDue(aTodo->dtDue().addSecs(utcOffSet));
                        dtDue.setTimeZone(d->mCalendar->timeZone());
                        aTodo->setDtDue(dtDue);
                    }
                }
                Todo::Ptr old = !aTodo->hasRecurrenceId() ? d->mCalendar->todo(aTodo->uid()) : d->mCalendar->todo(aTodo->uid(), aTodo->recurrenceId());
                if (old) {
                    if (aTodo->revision() > old->revision()) {
                        d->mCalendar->deleteTodo(old); // move old to deleted
                        removeAllVCal(d->mTodosRelate, old);
                        d->mCalendar->addTodo(aTodo); // and replace it with this one
                    }
                } else {
                    d->mCalendar->addTodo(aTodo); // just add this one
                }
            }
        } else if ((strcmp(vObjectName(curVO), VCVersionProp) == 0) || (strcmp(vObjectName(curVO), VCProdIdProp) == 0)
                   || (strcmp(vObjectName(curVO), VCTimeZoneProp) == 0)) {
            // do nothing, we know these properties and we want to skip them.
            // we have either already processed them or are ignoring them.
            ;
        } else if (strcmp(vObjectName(curVO), VCDayLightProp) == 0) {
            // do nothing daylights are already processed
            ;
        } else {
            qCDebug(KCALCORE_LOG) << "Ignoring unknown vObject \"" << vObjectName(curVO) << "\"";
        }
    SKIP:;
    } // while

    // Post-Process list of events with relations, put Event objects in relation
    for (const auto &eventPtr : std::as_const(d->mEventsRelate)) {
        eventPtr->setRelatedTo(eventPtr->relatedTo());
    }

    for (const auto &todoPtr : std::as_const(d->mTodosRelate)) {
        todoPtr->setRelatedTo(todoPtr->relatedTo());
    }

    // Now lets put the TZ back as it was if we have changed it.
    if (hasTimeZone) {
        d->mCalendar->setTimeZone(previousZone);
    }
}

int VCalFormat::numFromDay(const QString &day)
{
    if (day == QLatin1String("MO ")) {
        return 0;
    }
    if (day == QLatin1String("TU ")) {
        return 1;
    }
    if (day == QLatin1String("WE ")) {
        return 2;
    }
    if (day == QLatin1String("TH ")) {
        return 3;
    }
    if (day == QLatin1String("FR ")) {
        return 4;
    }
    if (day == QLatin1String("SA ")) {
        return 5;
    }
    if (day == QLatin1String("SU ")) {
        return 6;
    }

    return -1; // something bad happened. :)
}

Attendee::PartStat VCalFormat::readStatus(const char *s) const
{
    QString statStr = QString::fromUtf8(s);
    statStr = statStr.toUpper();
    Attendee::PartStat status;

    if (statStr == QLatin1String("X-ACTION")) {
        status = Attendee::NeedsAction;
    } else if (statStr == QLatin1String("NEEDS ACTION")) {
        status = Attendee::NeedsAction;
    } else if (statStr == QLatin1String("ACCEPTED")) {
        status = Attendee::Accepted;
    } else if (statStr == QLatin1String("SENT")) {
        status = Attendee::NeedsAction;
    } else if (statStr == QLatin1String("TENTATIVE")) {
        status = Attendee::Tentative;
    } else if (statStr == QLatin1String("CONFIRMED")) {
        status = Attendee::Accepted;
    } else if (statStr == QLatin1String("DECLINED")) {
        status = Attendee::Declined;
    } else if (statStr == QLatin1String("COMPLETED")) {
        status = Attendee::Completed;
    } else if (statStr == QLatin1String("DELEGATED")) {
        status = Attendee::Delegated;
    } else {
        qCDebug(KCALCORE_LOG) << "error setting attendee mStatus, unknown mStatus!";
        status = Attendee::NeedsAction;
    }

    return status;
}

QByteArray VCalFormat::writeStatus(Attendee::PartStat status) const
{
    switch (status) {
    default:
    case Attendee::NeedsAction:
        return "NEEDS ACTION";
    case Attendee::Accepted:
        return "ACCEPTED";
    case Attendee::Declined:
        return "DECLINED";
    case Attendee::Tentative:
        return "TENTATIVE";
    case Attendee::Delegated:
        return "DELEGATED";
    case Attendee::Completed:
        return "COMPLETED";
    case Attendee::InProcess:
        return "NEEDS ACTION";
    }
}

void VCalFormat::readCustomProperties(VObject *o, const Incidence::Ptr &i)
{
    VObjectIterator iter;
    char *s;

    initPropIterator(&iter, o);
    while (moreIteration(&iter)) {
        VObject *cur = nextVObject(&iter);
        const char *curname = vObjectName(cur);
        Q_ASSERT(curname);
        if ((curname[0] == 'X' && curname[1] == '-') && strcmp(curname, ICOrganizerProp) != 0) {
            // TODO - for the time being, we ignore the parameters part
            // and just do the value handling here
            i->setNonKDECustomProperty(curname, QString::fromUtf8(s = fakeCString(vObjectUStringZValue(cur))));
            deleteStr(s);
        }
    }
}

void VCalFormat::writeCustomProperties(VObject *o, const Incidence::Ptr &i)
{
    Q_D(VCalFormat);
    const QMap<QByteArray, QString> custom = i->customProperties();
    for (auto cIt = custom.cbegin(); cIt != custom.cend(); ++cIt) {
        const QByteArray property = cIt.key();
        if (d->mManuallyWrittenExtensionFields.contains(property) || property.startsWith("X-KDE-VOLATILE")) { // krazy:exclude=strings
            continue;
        }

        addPropValue(o, property.constData(), cIt.value().toUtf8().constData());
    }
}
