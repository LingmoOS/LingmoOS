/*
    SPDX-FileCopyrightText: 2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendareventsplugin.h"

namespace CalendarEvents
{
CalendarEventsPlugin::CalendarEventsPlugin(QObject *parent)
    : QObject(parent)
{
}

CalendarEventsPlugin::~CalendarEventsPlugin()
{
}

ShowEventInterface::~ShowEventInterface() = default;

}

#include "moc_calendareventsplugin.cpp"
