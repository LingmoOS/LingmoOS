/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the ICalFormat class.
  该文件是用于处理日历数据的API的一部分，并定义了ICalFormat类。

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/
#ifndef KCALCORE_ICALFORMAT_H
#define KCALCORE_ICALFORMAT_H

#include "calformat.h"
#include "freebusy.h"
#include "incidence.h"
#include "schedulemessage.h"

namespace KCalendarCore {
class FreeBusy;
class Incidence;
class IncidenceBase;
class RecurrenceRule;

/**
  @brief
  iCalendar format implementation.

  This class implements the iCalendar format. It provides methods for
  loading/saving/converting iCalendar format data into the internal
  representation as Calendar and Incidences.

  @warning When importing/loading to a Calendar, there is only duplicate
  check if those Incidences are loaded into the Calendar. If they are not
  loaded it will create duplicates.
*/
class Q_CORE_EXPORT ICalFormat : public CalFormat
{
public:
    /**
      Constructor a new iCalendar Format object.
    */
    ICalFormat();

    /**
      Destructor.
    */
    ~ICalFormat() override;

    /**
      @copydoc
      CalFormat::load()
    */
    bool load(const Calendar::Ptr &calendar, const QString &fileName) override;

    /**
      @copydoc
      CalFormat::save()
    */
    bool save(const Calendar::Ptr &calendar, const QString &fileName) override;

    /**
      @copydoc
      CalFormat::fromString()

      @note The notebook is ignored and the default one is used
    */
    bool fromString(const Calendar::Ptr &calendar, const QString &string, bool deleted = false, const QString &notebook = QString()) override;

    /**
      Parses a string, returning the first iCal component as an Incidence.
      解析字符串，将第一个iCal组件作为关联返回。

      @param string is a QString containing the data to be parsed.

      @return non-zero pointer if the parsing was successful; 0 otherwise.
      @see fromString(const Calendar::Ptr &, const QString &), fromRawString()
    */
    Incidence::Ptr fromString(const QString &string);

    /**
      Parses a bytearray, returning the first iCal component as an Incidence, ignoring timezone information.
      解析字节数组，将第一个iCal组件作为关联返回，忽略时区信息。
      This function is significantly faster than fromString by avoiding the overhead of parsing timezone information.
      Timezones are instead solely interpreted by using system-timezones.

      @param string is a utf8 QByteArray containing the data to be parsed.

      @return non-zero pointer if the parsing was successful; 0 otherwise.
      @see fromString(const QString &), fromRawString()
    */
    Incidence::Ptr readIncidence(const QByteArray &string);

    /**
      Parses a string and fills a RecurrenceRule object with the information.
      解析字符串并用信息填充RecurrenceRule对象。

      @param rule is a pointer to a RecurrenceRule object.
      @param string is a QString containing the data to be parsed.
      @return true if successful; false otherwise.
    */
    Q_REQUIRED_RESULT bool fromString(RecurrenceRule *rule, const QString &string);

    /**
      @copydoc
      CalFormat::fromRawString()
    */
    Q_REQUIRED_RESULT bool
    fromRawString(const Calendar::Ptr &calendar, const QByteArray &string, bool deleted = false, const QString &notebook = QString()) override;

    /**
      @copydoc
      CalFormat::toString()
    */
    Q_REQUIRED_RESULT QString toString(const Calendar::Ptr &calendar, const QString &notebook = QString(), bool deleted = false) override;

    /**
      Converts an Incidence to a QString.
      @param incidence is a pointer to an Incidence object to be converted
      into a QString.

      @return the QString will be Null if the conversion was unsuccessful.
    */
    Q_REQUIRED_RESULT QString toString(const Incidence::Ptr &incidence);

    /**
      Converts an Incidence to a QByteArray.
      @param incidence is a pointer to an Incidence object to be converted
      into a QByteArray.

      @return the QString will be Null if the conversion was unsuccessful.
      @since 4.7
    */
    Q_REQUIRED_RESULT QByteArray toRawString(const Incidence::Ptr &incidence);

    /**
      Converts a RecurrenceRule to a QString.
      @param rule is a pointer to a RecurrenceRule object to be converted
      into a QString.

      @return the QString will be Null if the conversion was unsuccessful.
    */
    Q_REQUIRED_RESULT QString toString(RecurrenceRule *rule);

    /**
      Converts an Incidence to iCalendar formatted text.

      @param incidence is a pointer to an Incidence object to be converted
      into iCal formatted text.
      @return the QString will be Null if the conversion was unsuccessful.
    */
    Q_REQUIRED_RESULT QString toICalString(const Incidence::Ptr &incidence);

    /**
      Creates a scheduling message string for an Incidence.

      @param incidence is a pointer to an IncidenceBase object to be scheduled.
      @param method is a Scheduler::Method

      @return a QString containing the message if successful; 0 otherwise.
    */
    Q_REQUIRED_RESULT QString createScheduleMessage(const IncidenceBase::Ptr &incidence, iTIPMethod method);

    /**
      Parses a Calendar scheduling message string into ScheduleMessage object.

      @param calendar is a pointer to a Calendar object associated with the
      scheduling message.
      @param string is a QString containing the data to be parsed.

      @return a pointer to a ScheduleMessage object if successful; 0 otherwise.
      The calling routine may later free the return memory.
    */
    ScheduleMessage::Ptr parseScheduleMessage(const Calendar::Ptr &calendar, const QString &string);

    /**
      Converts a QString into a FreeBusy object.

      @param string is a QString containing the data to be parsed.
      @return a pointer to a FreeBusy object if successful; 0 otherwise.

      @note Do not attempt to free the FreeBusy memory from the calling routine.
    */
    FreeBusy::Ptr parseFreeBusy(const QString &string);

    /**
      Sets the iCalendar time zone.
      @param timeZone is the time zone to set.
      @see timeZone().
    */
    void setTimeZone(const QTimeZone &timeZone);

    /**
      Returns the iCalendar time zone.
      @see setTimeZone().
    */
    Q_REQUIRED_RESULT QTimeZone timeZone() const;

    /**
      Returns the timezone id string used by the iCalendar; an empty string
      if the iCalendar does not have a timezone.
    */
    Q_REQUIRED_RESULT QByteArray timeZoneId() const;

protected:
    /**
      @copydoc
      IncidenceBase::virtual_hook()
    */
    void virtual_hook(int id, void *data) override;

private:
    //@cond PRIVATE
    Q_DISABLE_COPY(ICalFormat)
    class Private;
    Private *const d;
    //@endcond
};

} // namespace KCalendarCore

#endif
