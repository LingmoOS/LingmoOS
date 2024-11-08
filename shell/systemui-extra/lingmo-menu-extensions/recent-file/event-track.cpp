/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "event-track.h"

#include <lingmosdk/diagnosetest/libkydatacollect.h>

using namespace LingmoUIMenu;

EventTrack::EventTrack(QObject *parent) : QObject(parent)
{

}

EventTrack *EventTrack::instance()
{
    static EventTrack eventTrack;
    return &eventTrack;
}

void EventTrack::sendClickEvent(const QString &event, const QString &page)
{
    KTrackData* data = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(data, event.toUtf8().data(), page.toUtf8().data());
    kdk_dia_data_free(data);
}

void EventTrack::sendDefaultEvent(const QString& event, const QString& page)
{
    KTrackData* data = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_upload_default(data, event.toUtf8().data(), page.toUtf8().data());
    kdk_dia_data_free(data);
}

void EventTrack::sendSearchEvent(const QString &event, const QString &page, const QString &content)
{
    KTrackData* data = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_SEARCH);
    kdk_dia_upload_search_content(data, event.toUtf8().data(), page.toUtf8().data(), content.toUtf8().data());
    kdk_dia_data_free(data);
}
