/*
    SPDX-FileCopyrightText: 2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CALENDAREVENTSPLUGIN_H
#define CALENDAREVENTSPLUGIN_H

#include <QCalendar>
#include <QDateTime>
#include <QMultiHash>
#include <QObject>
#include <QSharedDataPointer>

#include "calendarevents_export.h"

/**
 * The CalendarEvents namespace.
 */
namespace CalendarEvents
{

/**
 * @class EventData calendareventsplugin.h <CalendarEvents/CalendarEventsPlugin>
 *
 * Data about an event.
 */
class CALENDAREVENTS_EXPORT EventData
{
public:
    enum EventType {
        Holiday, // Any holiday
        Event, // General event
        Todo, // A Todo item
    };

    EventData();
    EventData(const EventData &other);
    ~EventData();

    EventData &operator=(const EventData &other);

    /**
     * The start date and time of this event
     */
    QDateTime startDateTime() const;

    /**
     * Set the start date-time of this event
     *
     * @param startDateTime the date-time of when the event is starting
     */
    void setStartDateTime(const QDateTime &startDateTime);

    /**
     * The end date and time of this event
     */
    QDateTime endDateTime() const;

    /**
     * Set the end date-time of this event
     *
     * @param endDateTime the date-time of when the event is ending
     */
    void setEndDateTime(const QDateTime &endDateTime);

    /**
     * If true, this event goes on the whole day (eg. a holiday)
     */
    bool isAllDay() const;

    /**
     * If set to true, it will be displayed in the Calendar agenda
     * without any time besides it, marked as "going on all day"
     *
     * This is useful for single-day events only, for multiple-day
     * events, leave to false (default)
     *
     * @param isAllDay set to true if the event takes all day, false otherwise
     *                 (defaults to false)
     */
    void setIsAllDay(bool isAllDay);

    /**
     * If true, this event won't mark the day in the calendar grid
     * The main purpose for this flag is to support
     * namedays, where in some countries the calendars have
     * different name in them every day. This is just a minor holiday
     * and as such should not mark the calendar grid, otherwise
     * the whole grid would be in a different color.
     */
    bool isMinor() const;

    /**
     * If set to true, it won't be marked in the calendar grid
     *
     * @param isMinor true if it's a minor event (like a nameday holiday),
     *                false otherwise (defaults to false)
     */
    void setIsMinor(bool isMinor);

    /**
     * Event title
     */
    QString title() const;

    /**
     * Sets the title of the event
     *
     * @param title The event title
     */
    void setTitle(const QString &title);

    /**
     * Event description, can provide more details about the event
     */
    QString description() const;

    /**
     * Sets the event description, which allows to add more details
     * about this event
     *
     * @param description The description
     */
    void setDescription(const QString &description);

    /**
     * Type of the current event, eg. a holiday, an event or a todo item
     */
    EventType type() const;

    /**
     * Sets the event type, eg. a holiday, an event or a todo item
     *
     * @param type The event type,
     */
    void setEventType(EventType type);

    /**
     * The color that should be used to mark this event with
     * It comes in the HTML hex format, eg. #AARRGGBB or #RRGGBB
     */
    QString eventColor() const;

    /**
     * This is to support various calendar colors the user might
     * have configured elsewhere
     *
     * @param color The color for this event in the HTML hex format
     *              eg. #AARRGGBB or #RRGGBB (this is passed directly
     *              to QML)
     */
    void setEventColor(const QString &color);

    /**
     * Unique ID of the event
     */
    QString uid() const;

    /**
     * Sets the uid of the event
     *
     * This is a mandatory field only if you want to use
     * the eventModified/eventRemoved signals, otherwise
     * setting it is optional
     *
     * @param uid A unique id, recommended is to use the plugin name as prefix (to keep it unique)
     */
    void setUid(const QString &uid);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

/**
 * @class CalendarEventsPlugin calendareventsplugin.h <CalendarEvents/CalendarEventsPlugin>
 *
 * Plugin for feeding events to a calendar instance.
 */
class CALENDAREVENTS_EXPORT CalendarEventsPlugin : public QObject
{
    Q_OBJECT

public:
    enum class SubLabelPriority {
        Low, /**< Usually used in alternate calendars */
        Default, /**< For holidays or normal events */
        High, /**< For flagged or marked events */
        Urgent,
    };
    Q_ENUM(SubLabelPriority)

    explicit CalendarEventsPlugin(QObject *parent = nullptr);
    ~CalendarEventsPlugin() override;

    /**
     * When this is called, the plugin should load all events data
     * between those two date ranges. Once the data are ready, it should
     * just emit the dataReady() signal. The range is usually one month
     *
     * @param startDate the start of the range
     * @param endDate the end of the range
     */
    virtual void loadEventsForDateRange(const QDate &startDate, const QDate &endDate) = 0;

    struct SubLabel {
        QString label; /**< The label will be displayed in the tooltip or beside the full date */
        QString yearLabel; /**< The label will be displayed under the year number */
        QString monthLabel; /**< The label will be displayed under the month number */
        QString dayLabel; /**< The label will be displayed under the day number */
        SubLabelPriority priority = SubLabelPriority::Default; /**< The display priority of the sublabel */
    };

Q_SIGNALS:
    /**
     * Emitted when the plugin has loaded the events data
     *
     * @param data A hash containing a QDate key for the event
     *             in the value, CalendarEvents::EventData, which holds all
     *             the details for the given event
     *             It's a multihash as there can be multiple events
     *             in the same day
     *             For multi-day events, insert just one with the key
     *             being the startdate of the event
     */
    void dataReady(const QMultiHash<QDate, CalendarEvents::EventData> &data);

    /**
     * Should be emitted when there is a modification of an event
     * that was previously returned via the dataReady() signal
     *
     * @param event The modified event data
     */
    void eventModified(const CalendarEvents::EventData &modifiedEvent);

    /**
     * Should be emitted when the plugin removes some event
     * from its collection
     *
     * @param uid The uid of the event that was removed
     */
    void eventRemoved(const QString &uid);

    /**
     * Emitted when the plugin has loaded the alternate dates
     *
     * @param data A hash containing a QDate key from Gregorian calendar
     *             for the alternate date in the value, QDate.
     * @since 6.0
     */
    void alternateCalendarDateReady(const QHash<QDate, QCalendar::YearMonthDay> &data);

    /**
     * Emitted when the plugin has loaded the sublabels
     *
     * @param data A hash containing a QDate key for the sublabels
     *             in the value, SubLabel.
     * @since 5.95
     */
    void subLabelReady(const QHash<QDate, SubLabel> &data);
};

/**
 * @class ShowEventInterface calendareventsplugin.h <CalendarEvents/CalendarEventsPlugin>
 *
 * Interface for displaying event details
 *
 * ShowEventInterface is an additional interface the CalendarEventsPlugin
 * implementations can implement if they support displaying details about
 * events (e.g. opening the event in KOrganizer).
 *
 * @since 5.61
 */
class CALENDAREVENTS_EXPORT ShowEventInterface
{
public:
    virtual ~ShowEventInterface();

    /**
     * When this is called, the plugin should show a window displaying the
     * full preview of the event.
     *
     * The plugin should return true if the event details can be displayed, false
     * otherwise.
     */
    virtual bool showEvent(const QString &uid) = 0;
};

}

Q_DECLARE_INTERFACE(CalendarEvents::CalendarEventsPlugin, "org.kde.CalendarEventsPlugin")
Q_DECLARE_INTERFACE(CalendarEvents::ShowEventInterface, "org.kde.CalendarEventsPlugin.ShowEventInterface")

#endif
