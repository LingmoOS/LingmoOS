/*
    SPDX-FileCopyrightText: 2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendareventsplugin.h"

#include <QSharedData>

class CalendarEvents::EventData::Private : public QSharedData
{
public:
    Private()
        : isAllDay(false)
        , isMinor(false)
    {
    }
    Private(const Private &other)
        : QSharedData(other)
    {
        startDateTime = other.startDateTime;
        endDateTime = other.endDateTime;
        title = other.title;
        description = other.description;
        uid = other.uid;
        eventColor = other.eventColor;
        type = other.type;
        isAllDay = other.isAllDay;
        isMinor = other.isMinor;
    }
    ~Private()
    {
    }
    QDateTime startDateTime; // Start of the event
    QDateTime endDateTime; // End of the event
    QString title; // Title of the event
    QString description; // Additional info of the event
    QString uid; // An internal event id, useful mostly just for the eventModified/Removed signals
    QString eventColor; // Optional color of the event in the HTML hex format, eg. #AARRGGBB or #RRGGBB
    EventType type; // Type of the event
    bool isAllDay; // True if the event takes all day, then it won't be displayed with any time
    bool isMinor; // A minor holiday that will not create a colored entry in the calendar
};

//---------------------------------------------------

namespace CalendarEvents
{
EventData::EventData()
    : d(new Private())
{
}

EventData::EventData(const EventData &other)
    : d(other.d)
{
}

EventData::~EventData()
{
}

EventData &EventData::operator=(const EventData &other)
{
    if (this == &other) {
        return *this;
    }

    d = other.d;
    return *this;
}

QDateTime EventData::startDateTime() const
{
    return d->startDateTime;
}

void EventData::setStartDateTime(const QDateTime &startDateTime)
{
    d->startDateTime = startDateTime;
}

QDateTime EventData::endDateTime() const
{
    return d->endDateTime;
}

void EventData::setEndDateTime(const QDateTime &endDateTime)
{
    d->endDateTime = endDateTime;
}

bool EventData::isAllDay() const
{
    return d->isAllDay;
}

void EventData::setIsAllDay(bool isAllDay)
{
    d->isAllDay = isAllDay;
}

bool EventData::isMinor() const
{
    return d->isMinor;
}

void EventData::setIsMinor(bool isMinor)
{
    d->isMinor = isMinor;
}

QString EventData::title() const
{
    return d->title;
}

void EventData::setTitle(const QString &title)
{
    d->title = title;
}

QString EventData::description() const
{
    return d->description;
}

void EventData::setDescription(const QString &description)
{
    d->description = description;
}

QString EventData::uid() const
{
    return d->uid;
}

void EventData::setUid(const QString &uid)
{
    d->uid = uid;
}

EventData::EventType EventData::type() const
{
    return d->type;
}

void EventData::setEventType(EventData::EventType type)
{
    d->type = type;
}

QString EventData::eventColor() const
{
    return d->eventColor;
}

void EventData::setEventColor(const QString &color)
{
    d->eventColor = color;
}

}
